//=====================================================================
// TIVIDY interactive TUI (the interface for the demo, spec section 5)
// A marker can: start a request, inspect it, take actions, see denials
// and state changes, view history, and use BOTH industries.
//=====================================================================
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Command.h"
#include "Common.h"
#include "Composite.h"
#include "DatabaseObserver.h"
#include "Decorator.h"
#include "ExternalDatabase.h"
#include "NotificationBridge.h"
#include "NotificationObserver.h"
#include "Proxy.h"
#include "ScenarioAccessPolicies.h"
#include "WorkflowEngineFacade.h"

using namespace std;

namespace {

//-------demo people (name, role, industry)
struct Person {
  string industry;
  Participant who;
};

//-------everything the menu needs in one place
struct App {
  AuditLog log;                              // shared trace: denials, audited transitions
  WorkflowEngineFacade facade;               // the engine entry point (Facade)
  AccessPolicy policy;                       // who may do what (Proxy)
  vector<Person> people;
  map<int, unique_ptr<AccessControlProxy> > proxies;  // one proxy per signed-in person
  string industry;                           // "insurance" or "service"
  int currentId;                             // who is signed in
  map<int, string> categoryOf;               // display only, so checklist names can be shown

  App() : industry("insurance"), currentId(1) {
    configureInsuranceAccess(policy);        // same AccessPolicy class, two industries
    configureServiceRequestAccess(policy);
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
  }
};

//-------a tiny in-memory database so the Observer demo can show saved records
// (stand-in until the SQLite/Postgres adapter is wired in)
class MemoryDatabase : public ExternalDatabase {
public:
  bool saveRecord(int taskID, string taskName, string state, string) override {
    records.push_back(taskName + " -> " + state);
    cout << "  [DB] saved record #" << taskID << ": " << records.back() << "\n";
    return true;
  }
  string getRecordState(string) override { return records.empty() ? "" : records.back(); }
  vector<string> fetchAllRecords() override { return records; }
  bool isConnected() override { return true; }

private:
  vector<string> records;
};

//-------input helpers (bad input never crashes the program)
bool readLine(const string &prompt, string &out) {
  cout << prompt;
  return static_cast<bool>(getline(cin, out));
}

bool readInt(const string &prompt, int &value) {
  string text;
  if (!readLine(prompt, text)) {
    return false;
  }
  try {
    value = stoi(text);
    return true;
  } catch (const exception &) {
    cout << "Please type a number.\n";
    return false;
  }
}

string chooseFrom(const string &title, const vector<string> &options) {
  cout << title << "\n";
  for (size_t i = 0; i < options.size(); ++i) {
    cout << "  " << i + 1 << ") " << options[i] << "\n";
  }
  int pick = 0;
  while (readInt("Pick: ", pick)) {
    if (pick >= 1 && static_cast<size_t>(pick) <= options.size()) {
      return options[pick - 1];
    }
    cout << "Pick a number from the list.\n";
  }
  return "";
}

//-------turns every kind of failure into a readable line (spec: sensible handling of invalid actions)
void attempt(const function<void()> &action) {
  try {
    action();
  } catch (const AccessDeniedError &e) {
    cout << "\n[PROXY] " << e.what() << "\n";
  } catch (const ValidationError &e) {
    cout << "\n[DECORATOR VALIDATION] " << e.what() << "\n";
  } catch (const invalid_argument &e) {
    cout << "\n[INPUT REJECTED] " << e.what() << "\n";
  } catch (const logic_error &e) {
    cout << "\n[ENGINE REFUSED] " << e.what() << "\n";
  } catch (const exception &e) {
    cout << "\n[ERROR] " << e.what() << "\n";
  }
}

//-------checklist names are display only until the Facade owns real tasks
vector<string> checklistFor(const string &industry, const string &category) {
  static const map<string, vector<string> > lists = {
      {"insurance/DeathBenefit", {"Verify policy number and status", "Review chronology of events", "Check beneficiary reference details", "Confirm payout calculation", "Draft response"}},
      {"insurance/AccidentalClaim", {"Verify policy number and status", "Review chronology of events", "Verify incident or police reference", "Check exclusion clauses", "Draft response"}},
      {"insurance/CriticalIllness", {"Verify policy number and status", "Review chronology of events", "Medical diagnosis code check", "Hospital benefit calculation", "Draft response"}},
      {"service/WaterLeak", {"Verify location", "Assign crew", "Isolate and repair", "Verify repair"}},
      {"service/Pothole", {"Verify location", "Assign crew", "Repair surface", "Verify repair"}},
      {"service/Streetlight", {"Verify location", "Assign electrician", "Replace fitting", "Verify repair"}}};
  map<string, vector<string> >::const_iterator found = lists.find(industry + "/" + category);
  return found == lists.end() ? vector<string>() : found->second;
}

//-------who is signed in, and their proxy (the Proxy checks every action below)
const Person &currentPerson(App &app) {
  for (size_t i = 0; i < app.people.size(); ++i) {
    if (app.people[i].who.getId() == app.currentId) {
      return app.people[i];
    }
  }
  return app.people[0];
}

AccessControlProxy &proxy(App &app) {
  const Person &person = currentPerson(app);
  unique_ptr<AccessControlProxy> &slot = app.proxies[person.who.getId()];
  if (!slot) {
    slot.reset(new AccessControlProxy(&app.facade, &app.policy, person.who, &app.log));
  }
  return *slot;
}

int actor(App &app) { return currentPerson(app).who.getId(); }

bool askRequest(int &id) { return readInt("Request id: ", id); }

//=====================================================================
// MAIN MENU ACTIONS
//=====================================================================

//-------button to toggle scenario (insurance <-> service requests, same engine)
void toggleIndustry(App &app) {
  app.industry = (app.industry == "insurance") ? "service" : "insurance";
  for (size_t i = 0; i < app.people.size(); ++i) {
    if (app.people[i].industry == app.industry) {
      app.currentId = app.people[i].who.getId();
      break;
    }
  }
  cout << "Scenario is now: " << app.industry << ". Signed in as " << currentPerson(app).who.getName() << ".\n";
}

//-------button to switch the signed-in person (shows role-based access)
void switchPerson(App &app) {
  vector<size_t> choices;
  for (size_t i = 0; i < app.people.size(); ++i) {
    if (app.people[i].industry == app.industry) {
      choices.push_back(i);
      cout << "  " << choices.size() << ") " << app.people[i].who.getName() << " (" << app.people[i].who.getRoleName() << ")\n";
    }
  }
  int pick = 0;
  if (readInt("Pick: ", pick) && pick >= 1 && static_cast<size_t>(pick) <= choices.size()) {
    app.currentId = app.people[choices[pick - 1]].who.getId();
    cout << "Signed in as " << currentPerson(app).who.getName() << ".\n";
  }
}

//-------button to submit a request (intake check = Decorator rules, then Proxy -> Facade)
void submit(App &app) {
  map<string, string> payload;
  payload["industry"] = app.industry;
  string text;
  if (app.industry == "insurance") {
    readLine("Policy number: ", text);   payload["policy_number"] = text;
    readLine("Email: ", text);           payload["customer_email"] = text;
    payload["category"] = chooseFrom("Claim type:", {"DeathBenefit", "AccidentalClaim", "CriticalIllness"});
    readLine("Claimed amount (try 'abc' for the exception path): ", text); payload["amount"] = text;
    readLine("Description: ", text);     payload["description"] = text;
  } else {
    readLine("Location: ", text);        payload["location"] = text;
    payload["category"] = chooseFrom("Request type:", {"WaterLeak", "Pothole", "Streetlight"});
    readLine("Description: ", text);     payload["description"] = text;
  }

  //-------intake check with ValidatedTask + rules (stand-in for the chain's IntakeHandler)
  ValidatedTask intake(new WorkUnit("Intake"), "Pending");
  for (map<string, string>::const_iterator it = payload.begin(); it != payload.end(); ++it) {
    intake.setData(it->first, it->second);
    intake.addRule(shared_ptr<const ValidationRule>(new RequiredKeyRule(it->first)));
  }
  if (app.industry == "insurance") {
    intake.addRule(shared_ptr<const ValidationRule>(new EmailFormatRule("customer_email")));
    intake.addRule(shared_ptr<const ValidationRule>(new NumericRangeRule("amount", 1, 100000000)));
  }
  vector<string> failures;
  if (!intake.validate(failures)) {
    throw ValidationError("Intake", failures);
  }

  int id = proxy(app).submitRequest(actor(app), payload);
  app.categoryOf[id] = payload["category"];
  cout << "Request " << id << " submitted (Proxy -> Facade).\n";
}

//-------button to list what this person is allowed to see (Proxy filters the list)
void listRequests(App &app) {
  vector<int> ids = proxy(app).listInstanceIds(actor(app));
  if (ids.empty()) {
    cout << "No requests visible to " << currentPerson(app).who.getName() << ".\n";
  }
  for (size_t i = 0; i < ids.size(); ++i) {
    cout << "Request " << ids[i] << ": " << proxy(app).getStatus(actor(app), ids[i]) << "\n";
  }
}

//-------button to view one request: status, checklist, history
void viewRequest(App &app) {
  int id = 0;
  if (!askRequest(id)) return;
  cout << "Status: " << proxy(app).getStatus(actor(app), id) << "\n";
  vector<string> steps = checklistFor(app.industry, app.categoryOf[id]);
  for (size_t i = 0; i < steps.size(); ++i) {
    cout << "  task " << i + 1 << ": " << steps[i] << "\n";
  }
  vector<string> history = proxy(app).getHistory(actor(app), id);
  cout << "History:\n";
  for (size_t i = 0; i < history.size(); ++i) {
    cout << "  - " << history[i] << "\n";
  }
}

void takeOwnership(App &app) {
  int id = 0;
  if (askRequest(id)) { proxy(app).takeOwnership(actor(app), id); cout << "Taken.\n"; }
}

void completeTask(App &app) {
  int id = 0, task = 0;
  if (askRequest(id) && readInt("Task number: ", task)) { proxy(app).completeTask(actor(app), id, task); cout << "Task " << task << " completed.\n"; }
}

void approve(App &app) {
  int id = 0;
  if (askRequest(id)) { proxy(app).approveRequest(actor(app), id); cout << "Approved.\n"; }
}

void reject(App &app) {
  int id = 0;
  string reason;
  if (askRequest(id) && readLine("Reason: ", reason)) { proxy(app).rejectRequest(actor(app), id, reason); cout << "Rejected.\n"; }
}

void cancel(App &app) {
  int id = 0;
  if (askRequest(id)) { proxy(app).cancelRequest(actor(app), id); cout << "Cancelled.\n"; }
}

void undo(App &app) {
  int id = 0;
  if (askRequest(id)) { proxy(app).undoLastAction(actor(app), id); cout << "Last action undone.\n"; }
}

void saveProgress(App &app) {
  int id = 0;
  if (askRequest(id)) { proxy(app).saveProgress(actor(app), id); cout << "Progress saved.\n"; }
}

//-------button to show the audit trace (Proxy denials and audited transitions)
void showTrace(App &app) {
  cout << "----- audit trace -----\n";
  for (size_t i = 0; i < app.log.getEntries().size(); ++i) {
    cout << AuditLog::formatEntry(app.log.getEntries()[i]) << "\n";
  }
}

//=====================================================================
// PATTERN SHOWCASE (each item drives the REAL classes directly)
//=====================================================================

//-------showcase 1: Composite + Prototype + State
void showComposite() {
  WorkGroup claim("Claim");
  Stage *stage = new Stage("Investigation");
  SubProcess *checks = new SubProcess("Checks");
  WorkUnit *verify = new WorkUnit("Verify policy");
  WorkUnit *review = new WorkUnit("Review events");
  checks->add(verify);
  checks->add(review);
  stage->add(checks);
  claim.add(stage);

  cout << "[Composite] " << claim.getName() << " > " << stage->getName() << " > " << checks->getName() << "\n";
  vector<WorkComponent *> units = checks->getChildren();
  for (size_t i = 0; i < units.size(); ++i) {
    cout << "    task '" << units[i]->getName() << "' is " << units[i]->getState()->getStatus() << "\n";
  }

  WorkUnit *copy = verify->clone();   // Prototype: an independent copy
  verify->nextState();                // State: Pending moves forward
  cout << "[State] original '" << verify->getName() << "' is " << verify->getState()->getStatus() << "\n";
  cout << "[Prototype] its clone is still " << copy->getState()->getStatus() << "\n";
  delete copy;

  WorkGroup *treeCopy = claim.clone();
  cout << "[Prototype] whole tree cloned: " << (treeCopy != &claim ? "separate object" : "same object")
       << ", " << treeCopy->getChildren().size() << " stage(s)\n";
  delete treeCopy;
}

//-------showcase 2: Decorator (validation, audit, priority + SLA), you type the response
void showDecorator(App &app) {
  WorkUnit probe("probe");            // learn the real text of the "working" status
  probe.nextState();
  string working = probe.getState()->getStatus();

  ManualClock clock(1000000);         // a clock we control, to show an overdue SLA
  ValidatedTask *validated = new ValidatedTask(new WorkUnit("Draft response"), working);
  validated->addRule(shared_ptr<const ValidationRule>(new RequiredKeyRule("response_email")));
  AuditedTask *audited = new AuditedTask(validated, &app.log);
  PrioritisedTask task(audited, PriorityLevel::Urgent, 7, &clock);

  cout << "[Decorator] stack: PrioritisedTask > AuditedTask > ValidatedTask > WorkUnit, priority "
       << priorityLevelToString(task.getPriority()) << "\n";
  task.nextState();
  cout << "Started: " << task.getState()->getStatus() << ", SLA deadline set: " << (task.hasDeadline() ? "yes" : "no") << "\n";
  attempt([&]() { task.nextState(); });   // blocked: response_email is empty
  clock.advanceDays(8);
  cout << "[SLA] 8 days later overdue? " << (task.isOverdue() ? "YES" : "no") << "\n";

  string response;
  readLine("Type the response (leave empty to stay blocked): ", response);
  task.setData("response_email", response);
  attempt([&]() { task.nextState(); cout << "Now: " << task.getState()->getStatus() << ", overdue? " << (task.isOverdue() ? "YES" : "no") << "\n"; });
  cout << "(see the audit trace for the [Audited] lines)\n";
}

//-------showcase 3: Command + Memento (execute, then undo)
void showCommand() {
  WorkUnit unit("Review claim");
  ActionExecutor executor;
  bool ok = executor.executeCommand(new StartTaskCommand(&unit));
  cout << "[Command] start executed: " << (ok ? "yes" : "no") << ", state " << unit.getState()->getStatus() << "\n";
  ok = executor.executeCommand(new CompleteTaskCommand(&unit));
  cout << "[Command] complete executed: " << (ok ? "yes" : "no") << ", state " << unit.getState()->getStatus() << "\n";
  ok = executor.undoLastAction();
  cout << "[Memento] undo worked: " << (ok ? "yes" : "no") << ", state " << unit.getState()->getStatus() << "\n";
}

//-------showcase 4: Observer + Bridge (+ database stand-in): one state change, many reactions
void showObserver(App &app) {
  AuditedTask task(new WorkUnit("Notify demo"), &app.log);   // observers attach to the OUTERMOST task
  ConsoleEmailAdapter console(cout);                          // Bridge: same notice, different channels
  EmailChannel email(&console);
  AppChannel appChannel;
  NotificationObserver notifier(&task, &email, &appChannel);
  MemoryDatabase database;
  DatabaseObserver recorder(&task, &database);
  task.attach(&notifier);
  task.attach(&recorder);
  cout << "[Observer] advancing the task; every observer is told:\n";
  task.nextState();
  task.nextState();
  task.detach(&notifier);
  task.detach(&recorder);
}

//-------showcase 5: Builder + Chain of Responsibility + Bridge, via the Facade's blueprint run
void showBuilder(App &app) {
  string email;
  readLine("Customer email for the notice (Enter = default): ", email);
  WorkItem item;
  if (!email.empty()) item.setData("email", email);
  cout << "[Builder] reading blueprint.json, building the workflow, running the chain...\n";
  app.facade.submitRequest(item);
}

void showcaseMenu(App &app) {
  string choice;
  while (true) {
    cout << "\n--- Pattern showcase ---\n"
            " 1) Composite + Prototype + State\n 2) Decorator (type a response, watch the SLA)\n"
            " 3) Command + Memento (execute / undo)\n 4) Observer + Bridge\n"
            " 5) Builder + Chain of Responsibility (blueprint)\n 0) Back\n";
    if (!readLine("Choose: ", choice) || choice == "0") return;
    if (choice == "1") attempt([&]() { showComposite(); });
    else if (choice == "2") attempt([&]() { showDecorator(app); });
    else if (choice == "3") attempt([&]() { showCommand(); });
    else if (choice == "4") attempt([&]() { showObserver(app); });
    else if (choice == "5") attempt([&]() { showBuilder(app); });
    else cout << "Pick a number from the menu.\n";
  }
}

void showcase(App &app) { showcaseMenu(app); }

} // namespace

//=====================================================================
// MAIN LOOP
//=====================================================================
int main() {
  cout << "========================================\n"
          "   TIVIDY: Generic Workflow Engine      \n"
          "========================================\n";
  App app;

  //-------menu table: one line per button
  static const map<string, function<void(App &)> > buttons = {
      {"1", toggleIndustry}, {"2", switchPerson}, {"3", submit},     {"4", listRequests},
      {"5", viewRequest},    {"6", takeOwnership}, {"7", completeTask}, {"8", approve},
      {"9", reject},         {"10", cancel},      {"11", undo},      {"12", saveProgress},
      {"13", showTrace},     {"14", showcase}};

  string choice;
  while (true) {
    cout << "\n[" << app.industry << "] signed in: " << currentPerson(app).who.getName() << " ("
         << currentPerson(app).who.getRoleName() << ")\n"
            " 1) Toggle scenario    2) Switch person   3) Submit request   4) List requests\n"
            " 5) View request       6) Take ownership  7) Complete task    8) Approve\n"
            " 9) Reject            10) Cancel         11) Undo           12) Save progress\n"
            "13) Audit trace       14) Pattern showcase   0) Quit\n";
    if (!readLine("Choose: ", choice) || choice == "0") break;
    map<string, function<void(App &)> >::const_iterator found = buttons.find(choice);
    if (found == buttons.end()) {
      cout << "Pick a number from the menu.\n";
      continue;
    }
    attempt([&]() { found->second(app); });
  }
  cout << "Goodbye.\n";
  return 0;
}