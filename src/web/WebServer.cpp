#include "WebServer.h"

#include <dirent.h>
#include <sys/stat.h>

#include <cctype>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <utility>

#include <nlohmann/json.hpp>
#include "httplib.h"

#include "Decorator.h"
#include "IntakeCheck.h"
#include "PostgresAPI.h"
#include "PostgresAdapter.h"
#include "Proxy.h"
#include "SQLiteAPI.h"
#include "SQLiteAdapter.h"
#include "ScenarioAccessPolicies.h"
#include "WorkflowBlueprint.h"
#include "WorkflowEngineFacade.h"

using namespace std;
using Json = nlohmann::json;

namespace {

struct Person { string group; Participant who; };   // group = which scenario family the person belongs to
struct Event { string pattern; string text; };
struct Task { int id; string name; string state; };

typedef function<void(const Person &, AccessControlProxy &, httplib::Response &)> RouteFn;

//-------small helpers
string readFile(const string &path) {
  ifstream in(path.c_str());
  stringstream text;
  text << in.rdbuf();
  return text.str();
}

string slugOf(const string &text) {
  string slug;
  for (size_t i = 0; i < text.size(); ++i) {
    unsigned char c = static_cast<unsigned char>(text[i]);
    if (isalnum(c)) slug += static_cast<char>(tolower(c));
    else if (!slug.empty() && slug[slug.size() - 1] != '_') slug += '_';
  }
  if (slug.size() > 40) slug.resize(40);
  return slug.empty() ? "custom" : slug;
}

//-------which people belong to which scenario (a choice, see the notes)
string groupOf(const WorkflowScenario &scenario) {
  string text = scenario.id + scenario.path;
  if (text.find("life") != string::npos) return "insurance";
  if (text.find("service") != string::npos) return "service";
  return "generic";
}

//-------the form fields come from the blueprint's required_inputs
Json inputsOf(const string &path) {
  Json list = Json::array();
  try {
    Json blueprint = Json::parse(readFile(path));
    if (blueprint.contains("required_inputs")) {
      for (size_t i = 0; i < blueprint["required_inputs"].size(); ++i) {
        const Json &input = blueprint["required_inputs"][i];
        Json entry;
        entry["key"] = input.value("key", "");
        entry["prompt"] = input.value("prompt", "");
        list.push_back(entry);
      }
    }
  } catch (const exception &) {}
  return list;
}

//-------the task tree shown on the page: structure from the blueprint, states from the live request
Json treeOf(const Json &node, const vector<Task> &rows, int &next) {
  Json out;
  out["type"] = node.value("type", "");
  out["name"] = node.value("name", "");
  if (node.value("type", "") == "WorkUnit") {
    int id = next++;
    out["id"] = id;
    out["state"] = (id >= 1 && static_cast<size_t>(id) <= rows.size()) ? rows[id - 1].state : "";
  } else {
    out["children"] = Json::array();
    if (node.contains("children")) {
      for (size_t i = 0; i < node["children"].size(); ++i) {
        out["children"].push_back(treeOf(node["children"][i], rows, next));
      }
    }
  }
  return out;
}

Json treeFor(const string &path, const vector<Task> &rows) {
  Json roots = Json::array();
  int next = 1;
  try {
    Json blueprint = Json::parse(readFile(path));
    for (size_t i = 0; i < blueprint["chain"].size(); ++i) {
      roots.push_back(treeOf(blueprint["chain"][i], rows, next));
    }
  } catch (const exception &) {}
  return roots;
}

//-------finds the HTML file the report Visitor just wrote (newest .html in the working folder)
string newestHtml(time_t since) {
  DIR *dir = opendir(".");
  if (dir == nullptr) return "";
  string bestName;
  long bestSec = 0, bestNsec = 0;
  while (dirent *entry = readdir(dir)) {
    string name = entry->d_name;
    if (name.size() < 5 || name.substr(name.size() - 5) != ".html") continue;
    struct stat info;
    if (stat(name.c_str(), &info) != 0 || info.st_mtime < since) continue;
    long sec = static_cast<long>(info.st_mtim.tv_sec), nsec = static_cast<long>(info.st_mtim.tv_nsec);
    if (bestName.empty() || sec > bestSec || (sec == bestSec && nsec >= bestNsec)) {
      bestName = name; bestSec = sec; bestNsec = nsec;
    }
  }
  closedir(dir);
  return bestName.empty() ? "" : readFile(bestName);
}

//-------which subsystems an action touches (shown in the "What just fired" panel)
const vector<pair<string, string> > &firedFor(const string &action) {
  static const map<string, vector<pair<string, string> > > table = {
      {"submit", {{"Facade", "one entry point: submitRequest"}, {"Proxy", "Submit permission checked, requester registered"},
                  {"Decorator", "intake rules (ValidatedTask) passed"}, {"Builder", "blueprint JSON built into the workflow"},
                  {"Prototype", "workflow cloned from its template"}, {"Composite", "groups, stages and tasks created"},
                  {"Observer", "database observer attached to every node"}}},
      {"take", {{"Proxy", "ownership lock taken, other agents are now refused"}, {"Facade", "owner recorded"}}},
      {"complete", {{"Proxy", "assigned-request check passed"}, {"Command", "assign, start and complete commands executed"},
                    {"State", "task moved to Completed"}, {"Memento", "snapshot saved so the step can be undone"},
                    {"Observer", "database observer notified"}, {"Adapter", "record written through the database adapter"},
                    {"Chain of Responsibility", "next work group resumed"}}},
      {"undo", {{"Command", "last commands undone"}, {"Memento", "earlier task state restored"}}},
      {"save", {{"Facade", "progress note saved"}}},
      {"approve", {{"Proxy", "Approve permission checked"}, {"Facade", "decision recorded"}}},
      {"reject", {{"Proxy", "Reject permission checked"}, {"Facade", "decision and reason recorded"}}},
      {"cancel", {{"Proxy", "Cancel permission checked"}, {"Facade", "request cancelled"}}},
      {"report", {{"Visitor", "ProgressReportVisitor walked the tree and wrote the HTML report"}}},
      {"blueprint", {{"Builder", "new blueprint validated and added to the catalog"}}}};
  static const vector<pair<string, string> > none;
  map<string, vector<pair<string, string> > >::const_iterator found = table.find(action);
  return found == table.end() ? none : found->second;
}

//-------the actions that need no extra data
typedef function<void(AccessControlProxy &, int, int, const string &)> RequestAction;
const map<string, RequestAction> &requestActions() {
  static const map<string, RequestAction> actions = {
      {"take", [](AccessControlProxy &p, int me, int id, const string &) { p.takeOwnership(me, id); }},
      {"approve", [](AccessControlProxy &p, int me, int id, const string &) { p.approveRequest(me, id); }},
      {"reject", [](AccessControlProxy &p, int me, int id, const string &why) { p.rejectRequest(me, id, why); }},
      {"cancel", [](AccessControlProxy &p, int me, int id, const string &) { p.cancelRequest(me, id); }},
      {"save", [](AccessControlProxy &p, int me, int id, const string &) { p.saveProgress(me, id); }}};
  return actions;
}

} // namespace

//=====================================================================
// Site: everything the routes share
//=====================================================================
struct WebServer::Site {
  WorkflowEngineFacade *facade;
  string webRoot;
  AuditLog log;                                 // Proxy denials and decorator lines end up here
  AccessPolicy policy;                          // ONE policy for every industry
  vector<Person> people;
  map<int, unique_ptr<AccessControlProxy> > proxies;
  mutex engineLock;                             // one request at a time into the engine
  vector<Event> events;
  size_t auditSeen;
  map<int, WorkflowScenario> scenarioOf;        // which blueprint each request came from
  map<int, map<string, string> > submitted;
  vector<string> warnings;
  unique_ptr<PostgresAdapter> pg;
  unique_ptr<SQLiteAdapter> lite;
  mutex dbLock;

  Site(WorkflowEngineFacade *f, const string &root) : facade(f), webRoot(root), auditSeen(0) {
    configureInsuranceAccess(policy);
    configureServiceRequestAccess(policy);
    //-------roles for the generic industries (job application, custom blueprints)
    policy.grantAction("Requester", AccessAction::Submit, AccessScope::Any);
    policy.grantAction("Requester", AccessAction::View, AccessScope::OwnRequest);
    policy.grantAction("Requester", AccessAction::ViewHistory, AccessScope::OwnRequest);
    policy.grantAction("Requester", AccessAction::Cancel, AccessScope::OwnRequest);
    policy.grantAction("Handler", AccessAction::View, AccessScope::Any);
    policy.grantAction("Handler", AccessAction::ViewHistory, AccessScope::Any);
    policy.grantAction("Handler", AccessAction::TakeOwnership, AccessScope::Any);
    policy.grantAction("Handler", AccessAction::CompleteTask, AccessScope::AssignedRequest);
    policy.grantAction("Handler", AccessAction::SaveProgress, AccessScope::AssignedRequest);
    policy.grantAction("Handler", AccessAction::Undo, AccessScope::AssignedRequest);
    policy.grantAction("Handler", AccessAction::Cancel, AccessScope::AssignedRequest);
    policy.grantAction("Approver", AccessAction::View, AccessScope::Any);
    policy.grantAction("Approver", AccessAction::ViewHistory, AccessScope::Any);
    policy.grantAction("Approver", AccessAction::Approve, AccessScope::Any);
    policy.grantAction("Approver", AccessAction::Reject, AccessScope::Any);

    people.push_back({"insurance", Participant(1, "Mo?", "Customer")});
    people.push_back({"insurance", Participant(2, "Gabriella", "Customer")});
    people.push_back({"insurance", Participant(10, "Tsohliella", "ClaimsSpecialist")});
    people.push_back({"insurance", Participant(11, "Thando", "ClaimsSpecialist")});
    people.push_back({"insurance", Participant(20, "Ashleyella", "TeamLead")});
    people.push_back({"insurance", Participant(21, "Roberta", "SeniorManager")});
    people.push_back({"service", Participant(101, "Jords", "Citizen")});
    people.push_back({"service", Participant(105, "Kemi", "Dispatcher")});
    people.push_back({"service", Participant(110, "Noni", "Technician")});
    people.push_back({"service", Participant(120, "MoButCooler", "Supervisor")});
    people.push_back({"generic", Participant(201, "Guest Requester", "Requester")});
    people.push_back({"generic", Participant(202, "Guest Handler", "Handler")});
    people.push_back({"generic", Participant(203, "Guest Approver", "Approver")});
    warnings = facade->refreshScenarios();
  }

  //-------the "What just fired" feed
  void note(const string &pattern, const string &text) { events.push_back({pattern, text}); }
  void fire(const string &action) {
    const vector<pair<string, string> > &list = firedFor(action);
    for (size_t i = 0; i < list.size(); ++i) note(list[i].first, list[i].second);
  }
  void flushAudit() {
    const vector<AuditEntry> &entries = log.getEntries();
    for (; auditSeen < entries.size(); ++auditSeen) {
      const string &m = entries[auditSeen].message;
      note(m.find("Access denied") == 0 ? "Proxy" : "Audit", m);
    }
  }
  void addEngineLine(const string &raw) {
    size_t first = raw.find_first_not_of(" \t\r\n");
    if (first == string::npos) return;
    string line = raw.substr(first);
    if (line.find_first_not_of("=- ") == string::npos) return;   // skip separator lines
    note("Engine", line.substr(0, 200));
  }

  //-------lookups
  const Person *personById(int id) const {
    for (size_t i = 0; i < people.size(); ++i) if (people[i].who.getId() == id) return &people[i];
    return nullptr;
  }
  AccessControlProxy &proxyFor(const Person &p) {
    unique_ptr<AccessControlProxy> &slot = proxies[p.who.getId()];
    if (!slot) slot.reset(new AccessControlProxy(facade, &policy, p.who, &log));
    return *slot;
  }
  const WorkflowScenario *scenarioById(const string &id) const {
    const vector<WorkflowScenario> &all = facade->listScenarios();
    for (size_t i = 0; i < all.size(); ++i) if (all[i].id == id) return &all[i];
    return nullptr;
  }
  bool allowed(const Person &p, AccessAction a, int id) { return policy.isAllowed(p.who, a, id); }

  //-------captures what the engine prints (chain output, alerts, emails) so the page can show it
  class Capture {
  public:
    explicit Capture(Site &s) : site(s), oldOut(cout.rdbuf(buffer.rdbuf())), oldErr(cerr.rdbuf(buffer.rdbuf())) {}
    ~Capture() {
      cout.rdbuf(oldOut);
      cerr.rdbuf(oldErr);
      istringstream lines(buffer.str());
      string line;
      while (getline(lines, line)) site.addEngineLine(line);
    }
  private:
    Site &site;
    ostringstream buffer;
    streambuf *oldOut;
    streambuf *oldErr;
  };

  void reply(httplib::Response &res, const Json &body) { res.set_content(body.dump(), "application/json"); }
  void fail(httplib::Response &res, int code, const string &message, const string &pattern) {
    Json body;
    body["error"] = message;
    res.status = code;
    res.set_content(body.dump(), "application/json");
    if (!pattern.empty()) note(pattern, message);
  }

  //-------runs one route for the signed-in person and turns every failure into a readable error
  void guard(const httplib::Request &req, httplib::Response &res, const RouteFn &fn) {
    lock_guard<mutex> hold(engineLock);
    Capture capture(*this);
    try {
      string who = req.get_header_value("X-Participant");
      if (who.empty()) who = req.get_param_value("as");
      const Person *me = who.empty() ? nullptr : personById(atoi(who.c_str()));
      if (me == nullptr) throw invalid_argument("Pick who you are signed in as");
      fn(*me, proxyFor(*me), res);
    } catch (const AccessDeniedError &e) {
      fail(res, 403, e.what(), "");                       // the Proxy already wrote this to the audit log
    } catch (const ValidationError &e) {
      string text;
      for (size_t i = 0; i < e.getFailures().size(); ++i) text += (i ? "; " : "") + e.getFailures()[i];
      fail(res, 422, "Validation Error: " + text, "Decorator");
    } catch (const BlueprintValidationError &e) {
      fail(res, 422, string("Blueprint rejected: ") + e.what(), "Builder");
    } catch (const invalid_argument &e) {
      fail(res, 400, e.what(), "Facade");
    } catch (const logic_error &e) {
      fail(res, 409, e.what(), "Facade");
    } catch (const exception &e) {
      fail(res, 500, e.what(), "Engine");
    }
    flushAudit();
  }

  //-------tasks of one request, parsed from "3. name [State] in group"
  vector<Task> tasks(int actor, int id) {
    vector<Task> rows;
    vector<string> lines = facade->getTaskStates(actor, id);
    for (size_t i = 0; i < lines.size(); ++i) {
      size_t dot = lines[i].find(". "), inPos = lines[i].rfind("] in ");
      if (dot == string::npos || inPos == string::npos) continue;
      size_t open = lines[i].rfind(" [", inPos);
      if (open == string::npos || open < dot + 2) continue;
      Task t;
      t.id = atoi(lines[i].substr(0, dot).c_str());
      t.name = lines[i].substr(dot + 2, open - (dot + 2));
      t.state = lines[i].substr(open + 2, inPos - (open + 2));
      rows.push_back(t);
    }
    return rows;
  }
  static int doneCount(const vector<Task> &rows) {
    int done = 0;
    for (size_t i = 0; i < rows.size(); ++i) if (rows[i].state == "Completed") ++done;
    return done;
  }
  string nameOf(int id) const { const Person *p = personById(id); return p ? p->who.getName() : "someone"; }

  //-------one request as JSON (the Proxy checks that this person may see it)
  Json describe(const Person &me, AccessControlProxy &px, int id, bool detail) {
    int actor = me.who.getId();
    string statusText = px.getStatus(actor, id);
    vector<Task> rows = tasks(actor, id);
    int done = doneCount(rows);
    bool allDone = !rows.empty() && static_cast<size_t>(done) == rows.size();
    string base = statusText.substr(0, statusText.find(' '));
    bool open = (base == "Open");
    int lockId = policy.getAssigneeId(id);
    bool known = scenarioOf.count(id) > 0;

    Json out, can;
    out["id"] = id;
    out["scenario"] = known ? scenarioOf[id].id : "";
    out["status"] = base;
    out["lock"] = lockId > 0 ? "Locked by " + nameOf(lockId) : "";
    out["progress"] = to_string(done) + "/" + to_string(rows.size());
    out["awaitingApproval"] = open && allDone;
    can["take"] = open && lockId < 0 && allowed(me, AccessAction::TakeOwnership, id);
    can["complete"] = open && lockId == actor && allowed(me, AccessAction::CompleteTask, id);
    can["approve"] = open && allDone && allowed(me, AccessAction::Approve, id);
    can["reject"] = open && allDone && allowed(me, AccessAction::Reject, id);
    can["cancel"] = open && allowed(me, AccessAction::Cancel, id);
    can["undo"] = open && done > 0 && allowed(me, AccessAction::Undo, id);
    can["save"] = open && allowed(me, AccessAction::SaveProgress, id);
    out["can"] = can;
    if (!detail) return out;

    out["tree"] = known ? treeFor(scenarioOf[id].path, rows) : Json::array();
    Json history = Json::array();
    try {
      vector<string> lines = px.getHistory(actor, id);
      for (size_t i = 0; i < lines.size(); ++i) history.push_back(lines[i]);
    } catch (const AccessDeniedError &) {}                 // this role may not read history
    out["history"] = history;
    Json data = Json::object();
    if (submitted.count(id)) {
      for (map<string, string>::const_iterator it = submitted[id].begin(); it != submitted[id].end(); ++it) {
        if (it->first != "blueprint_path") data[it->first] = it->second;
      }
    }
    out["data"] = data;
    return out;
  }

  Json bootJson() {
    Json out;
    out["scenarios"] = Json::array();
    const vector<WorkflowScenario> &all = facade->listScenarios();
    static const char *colours[] = {"#1f4e79", "#1b6b3a", "#6a3d9a", "#9a5b00", "#8a1c3c"};
    for (size_t i = 0; i < all.size(); ++i) {
      Json s;
      s["id"] = all[i].id;
      s["label"] = all[i].label;
      s["group"] = groupOf(all[i]);
      s["color"] = colours[i % 5];
      s["inputs"] = inputsOf(all[i].path);
      out["scenarios"].push_back(s);
    }
    out["people"] = Json::array();
    for (size_t i = 0; i < people.size(); ++i) {
      Json p;
      p["id"] = people[i].who.getId();
      p["name"] = people[i].who.getName();
      p["role"] = people[i].who.getRoleName();
      p["group"] = people[i].group;
      p["submit"] = policy.isAllowed(people[i].who, AccessAction::Submit, AccessPolicy::NoInstance);
      out["people"].push_back(p);
    }
    out["warnings"] = warnings;
    return out;
  }

  //-------the report: the Visitor writes one HTML file per work group, we join them
  string report(int actor, int id) {
    time_t since = time(nullptr) - 1;
    int groups = facade->writeProgressReport(actor, id, 0);
    string all;
    for (int g = 0; g < groups; ++g) {
      if (g > 0) facade->writeProgressReport(actor, id, g);
      all += newestHtml(since) + "\n";
    }
    if (all.find_first_not_of(" \n") == string::npos) throw runtime_error("The report file was not found in the working folder");
    return all;
  }

  //-------database panel: reads through the Adapter (Postgres first, SQLite fallback)
  Json dbJson() {
    lock_guard<mutex> hold(dbLock);
    if (!pg) {
      pg.reset(new PostgresAdapter(new PostgresAPI(), "workflow_audit_log", true));
      lite.reset(new SQLiteAdapter(new SQLiteAPI("tividy_local.db"), "workflow_audit_log", true));
    }
    vector<string> rows = pg->fetchAllRecords();
    string which = "PostgresAdapter (Neon cloud)";
    if (rows.empty() && !pg->isConnected()) {
      rows = lite->fetchAllRecords();
      which = "SQLiteAdapter (local fallback)";
    }
    Json out;
    out["adapter"] = which;
    out["rows"] = Json::array();
    for (size_t i = rows.size() > 15 ? rows.size() - 15 : 0; i < rows.size(); ++i) out["rows"].push_back(rows[i]);
    return out;
  }

  // -------------------------------------------routes--------------------------------------------------------------------
  bool serve(const string &host, int port) {
    httplib::Server server;
    if (!server.set_mount_point("/", webRoot)) {
      cerr << "Cannot find the web folder '" << webRoot << "'. Run from the repository root.\n";
      return false;
    }

    server.Get("/api/bootstrap", [this](const httplib::Request &, httplib::Response &res) {
      lock_guard<mutex> hold(engineLock);
      reply(res, bootJson());
    });

    server.Get("/api/events", [this](const httplib::Request &req, httplib::Response &res) {
      lock_guard<mutex> hold(engineLock);
      size_t since = req.has_param("since") ? static_cast<size_t>(atol(req.get_param_value("since").c_str())) : 0;
      Json out;
      out["next"] = events.size();
      out["events"] = Json::array();
      for (size_t i = since; i < events.size(); ++i) {
        Json e;
        e["p"] = events[i].pattern;
        e["t"] = events[i].text;
        out["events"].push_back(e);
      }
      reply(res, out);
    });

    server.Get("/api/db", [this](const httplib::Request &, httplib::Response &res) {
      try { reply(res, dbJson()); } catch (const exception &e) { fail(res, 500, e.what(), ""); }
    });

    //-------submit (validation toast comes from the Decorator rules)
    server.Post("/api/submit", [this](const httplib::Request &req, httplib::Response &res) {
      guard(req, res, [&](const Person &me, AccessControlProxy &px, httplib::Response &out) {
        const WorkflowScenario *scenario = scenarioById(req.get_param_value("scenario"));
        if (scenario == nullptr) throw invalid_argument("Unknown industry");
        Json inputs = inputsOf(scenario->path);
        vector<string> keys;
        map<string, string> payload;
        for (size_t i = 0; i < inputs.size(); ++i) {
          string key = inputs[i]["key"];
          keys.push_back(key);
          payload[key] = req.get_param_value(key);
        }
        vector<string> failures = checkIntake(payload, keys);
        if (!failures.empty()) throw ValidationError("Intake", failures);
        payload["blueprint_path"] = scenario->path;            // the Facade builds this blueprint
        int id = px.submitRequest(me.who.getId(), payload);
        scenarioOf[id] = *scenario;
        submitted[id] = payload;
        note("UI", me.who.getName() + " submitted request " + to_string(id));
        fire("submit");
        Json body;
        body["id"] = id;
        reply(out, body);
      });
    });

    server.Get("/api/requests", [this](const httplib::Request &req, httplib::Response &res) {
      guard(req, res, [&](const Person &me, AccessControlProxy &px, httplib::Response &out) {
        vector<int> ids = px.listInstanceIds(me.who.getId());   // the Proxy filters this list
        Json list = Json::array();
        for (size_t i = 0; i < ids.size(); ++i) list.push_back(describe(me, px, ids[i], false));
        reply(out, list);
      });
    });

    server.Get(R"(/api/requests/(\d+))", [this](const httplib::Request &req, httplib::Response &res) {
      int id = atoi(req.matches[1].str().c_str());
      guard(req, res, [&](const Person &me, AccessControlProxy &px, httplib::Response &out) {
        reply(out, describe(me, px, id, true));
      });
    });

    server.Get(R"(/api/requests/(\d+)/blueprint)", [this](const httplib::Request &req, httplib::Response &res) {
      int id = atoi(req.matches[1].str().c_str());
      guard(req, res, [&](const Person &me, AccessControlProxy &px, httplib::Response &out) {
        px.getStatus(me.who.getId(), id);                       // Proxy: may this person see it?
        if (scenarioOf.count(id) == 0) throw invalid_argument("No blueprint recorded for this request");
        out.set_content(readFile(scenarioOf[id].path), "text/plain");
      });
    });

    server.Get(R"(/api/requests/(\d+)/report)", [this](const httplib::Request &req, httplib::Response &res) {
      int id = atoi(req.matches[1].str().c_str());
      guard(req, res, [&](const Person &me, AccessControlProxy &px, httplib::Response &out) {
        px.getStatus(me.who.getId(), id);
        string html = report(me.who.getId(), id);
        fire("report");
        out.set_header("Content-Disposition", "attachment; filename=progress_report_" + to_string(id) + ".html");
        out.set_content(html, "text/html");
      });
    });

    //-------take / approve / reject / cancel / save
    server.Post(R"(/api/requests/(\d+)/(take|approve|reject|cancel|save))", [this](const httplib::Request &req, httplib::Response &res) {
      int id = atoi(req.matches[1].str().c_str());
      string action = req.matches[2].str();
      guard(req, res, [&](const Person &me, AccessControlProxy &px, httplib::Response &out) {
        int actor = me.who.getId();
        if ((action == "approve" || action == "reject") && allowed(me, AccessAction::Approve, id)) {
          vector<Task> rows = tasks(actor, id);
          if (rows.empty() || static_cast<size_t>(doneCount(rows)) != rows.size()) {
            throw logic_error("Finish every checklist task before approval");
          }
        }
        requestActions().at(action)(px, actor, id, req.get_param_value("reason"));
        note("UI", me.who.getName() + " " + action + " request " + to_string(id));
        fire(action);
        Json body;
        body["ok"] = true;
        reply(out, body);
      });
    });

    //-------complete one task (assign + start + complete commands run behind this one call)
    server.Post(R"(/api/requests/(\d+)/tasks/(\d+)/complete)", [this](const httplib::Request &req, httplib::Response &res) {
      int id = atoi(req.matches[1].str().c_str());
      int taskId = atoi(req.matches[2].str().c_str());
      guard(req, res, [&](const Person &me, AccessControlProxy &px, httplib::Response &out) {
        int actor = me.who.getId();
        vector<Task> before = tasks(actor, id);
        px.completeTask(actor, id, taskId);
        vector<Task> after = tasks(actor, id);
        size_t i = static_cast<size_t>(taskId) - 1;
        if (taskId < 1 || i >= after.size() || before[i].state == after[i].state) {
          throw logic_error("That task cannot be completed yet: finish the earlier work groups first, or it is already done");
        }
        note("UI", me.who.getName() + " completed task " + to_string(taskId) + " of request " + to_string(id));
        fire("complete");
        Json body;
        body["ok"] = true;
        reply(out, body);
      });
    });

    server.Post(R"(/api/requests/(\d+)/undo)", [this](const httplib::Request &req, httplib::Response &res) {
      int id = atoi(req.matches[1].str().c_str());
      guard(req, res, [&](const Person &me, AccessControlProxy &px, httplib::Response &out) {
        int actor = me.who.getId();
        int before = doneCount(tasks(actor, id));
        px.undoLastAction(actor, id);
        if (doneCount(tasks(actor, id)) >= before) throw logic_error("Nothing to undo");
        note("UI", me.who.getName() + " undid the last step of request " + to_string(id));
        fire("undo");
        Json body;
        body["ok"] = true;
        reply(out, body);
      });
    });

    //-------generic industry: paste a blueprint, it is validated, saved to src/ and becomes a new tab
    server.Post("/api/blueprints", [this](const httplib::Request &req, httplib::Response &res) {
      guard(req, res, [&](const Person &, AccessControlProxy &, httplib::Response &out) {
        if (req.body.size() > 200000) throw invalid_argument("Blueprint is too large");
        Json blueprint;
        try { blueprint = Json::parse(req.body); }
        catch (const exception &e) { throw invalid_argument(string("That is not valid JSON: ") + e.what()); }
        WorkflowBlueprint::validate(blueprint, "<website>");      // throws BlueprintValidationError
        string path = "src/custom_" + slugOf(blueprint.value("workflow_name", "custom")) + ".json";
        ofstream file(path.c_str());
        file << blueprint.dump(2);
        file.close();
        if (!file) throw runtime_error("Could not save " + path);
        warnings = facade->refreshScenarios();
        note("Builder", "saved " + path);
        fire("blueprint");
        reply(out, bootJson());
      });
    });

    cout << "TIVIDY website: http://" << host << ":" << port << "\n";
    return server.listen(host, port);
  }
};

//=====================================================================
WebServer::WebServer(WorkflowEngineFacade *facade, const string &webRoot) : site(new Site(facade, webRoot)) {}
WebServer::~WebServer() {}
bool WebServer::run(const string &host, int port) { return site->serve(host, port); }