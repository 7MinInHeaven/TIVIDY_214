#include "WorkflowEngineFacade.h"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <nlohmann/json.hpp>
#include "Composite.h"
#include "ChainBuilder.h"
#include "WorkflowBuilder.h"
#include "Director.h"

#include "ResendEmailAdapter.h"
#include "BrevoEmailAdapter.h"

WorkflowEngineFacade::WorkflowEngineFacade() : nextId(1) {
  curl_global_init(CURL_GLOBAL_DEFAULT);
  dbObserver = new DatabaseObserver(nullptr, new PostgresAdapter(new PostgresAPI(), "workflow_audit_log", true),new SQLiteAdapter(new SQLiteAPI("tividy_local.db"), "workflow_audit_log", true));

}
WorkflowEngineFacade::~WorkflowEngineFacade() {
  curl_global_cleanup(); 
  delete dbObserver;
}


//---------Robert added------------------
void WorkflowEngineFacade::attachObserverToTree(WorkComponent* component) {
  if (component == nullptr || dbObserver == nullptr) return;

  component->attach(dbObserver); // attach to WorkGroup/Stage/SubProcess/WorkUnit

  WorkGroup* group = dynamic_cast<WorkGroup*>(component);
  if (group != nullptr) {
    for (WorkComponent* child : group->getChildren()) {
      attachObserverToTree(child); //attach to all children if exist
    }
  }
}

//------------------------------------------

void WorkflowEngineFacade::submitRequest(WorkItem item) {
  std::string path = "src/service_blueprint.json";
  std::ifstream file(path);
  if (!file) 
  {
    file.open("blueprint.json");
  }

  if (!file) {
    std::cerr << "couldn't find json file: " << path << "\n";
    return;
  }

  json data;
  file >> data;

  WorkflowBuilder builder;
  Director director(&builder, &item, false);
  director.construct(data);
  Handler* chainHead = builder.getResult();
  std::vector<WorkGroup*> workGroups = builder.getWorkGroups();   // we own these
  
  for (size_t i = 0; i < workGroups.size(); i++) {
    attachObserverToTree(workGroups[i]);
  }
  
  std::cout << "\n=== Chain of Responsibility Execution (handleRequest across chained WorkGroups) ===\n";
  if (chainHead != nullptr) {
    chainHead->handleRequest(item);
  }

  for (size_t i = 0; i < workGroups.size(); i++) {
    delete workGroups[i];
  }
  workGroups.clear();
  chainHead = nullptr;

  std::cout << "\n=== Work Item Payload ===\n";
  for (const auto& pair : item.getPayloadData()) {
    std::cout << pair.first << " -> " << pair.second << "\n";
  }

  // =========================================================
  // 1. STRATEGY PATTERN: Setup Transport based on .env variables
  // =========================================================
  unique_ptr<EmailTransport> transport;
  
  const char* resendKey = getenv("RESEND_API_KEY");
  const char* brevoKey  = getenv("BREVO_API_KEY");
  const char* mjKey     = getenv("MJ_APIKEY_PUBLIC");

  if (resendKey) {
      cout << "[Init] Strategy: Using Resend Adapter\n";
      transport.reset(new ResendEmailAdapter(resendKey, getenv("RESEND_FROM_EMAIL")));
  } 
  else if (brevoKey) {
      cout << "[Init] Strategy: Using Brevo Adapter\n";
      transport.reset(new BrevoEmailAdapter(brevoKey, getenv("BREVO_FROM_EMAIL")));
  } 
  else if (mjKey) {
      bool sandbox = (getenv("MJ_SANDBOX") != nullptr);
      cout << "[Init] Strategy: Using Mailjet Adapter " << (sandbox ? "(SANDBOX)" : "(LIVE)") << "\n";
      transport.reset(new MailjetEmailAdapter(mjKey, getenv("MJ_APIKEY_PRIVATE"), getenv("MJ_FROM_EMAIL"), "TIVIDY Support", sandbox));
  } 
  else {
      cout << "[Init] Missing API credentials. Using Console Adapter.\n";
      transport.reset(new ConsoleEmailAdapter(cout));
  }

  // 2. Setup Channels
  EmailChannel emailChannel(transport.get());
  AppChannel appChannel;

  // 3. Extract data from the WorkItem
  string customerEmail = item.hasData("customer_email") ? item.getData("customer_email") : 
                         (item.hasData("email") ? item.getData("email") : "your-personal-email@gmail.com");
                         
  string supportAgentId = item.hasData("agent_id") ? item.getData("agent_id") : "AGENT-442";
  string currentState = item.hasData("state") ? item.getData("state") : "Pending";

  // 4. Create Notifications passing the WorkItem context
  ResultNotice emailResult(&emailChannel, &item, customerEmail, currentState);
  ResultNotice appResult(&appChannel, &item, supportAgentId, currentState);
  DeadlineNotice deadlineAlert(&appChannel, &item, supportAgentId, currentState);

  // 5. Send!
  emailResult.send();
  appResult.send();
  deadlineAlert.send();

  // 6. Check for Errors from ANY of the adapters
  if (auto *mj = dynamic_cast<MailjetEmailAdapter *>(transport.get())) {
    if (!mj->lastError().empty()) cerr << "\n[MAILJET ERROR] " << mj->lastError() << "\n";
  }
  else if (auto *rs = dynamic_cast<ResendEmailAdapter *>(transport.get())) {
    if (!rs->lastError().empty()) cerr << "\n[RESEND ERROR] " << rs->lastError() << "\n";
  }
  else if (auto *br = dynamic_cast<BrevoEmailAdapter *>(transport.get())) {
    if (!br->lastError().empty()) cerr << "\n[BREVO ERROR] " << br->lastError() << "\n";
  }

  cout << "\n========================================\n";
  cout << "   All Notifications Processed          \n";
  cout << "========================================\n";
}
//----------------mo added-------------------------------------------------------------

//finds a request or reports that it does not exist
WorkflowEngineFacade::Request &WorkflowEngineFacade::find(int instanceId) {
  map<int, Request>::iterator found = requests.find(instanceId);
  if (found == requests.end()) {
    throw invalid_argument("no such request");
  }
  return found->second;
}

const WorkflowEngineFacade::Request &WorkflowEngineFacade::find(int instanceId) const {
  map<int, Request>::const_iterator found = requests.find(instanceId);
  if (found == requests.end()) {
    throw invalid_argument("no such request");
  }
  return found->second;
}

//stores a new request and returns its id; an empty payload is rejected
int WorkflowEngineFacade::submitRequest(int actorId, const map<string, string> &payload) {
  if (payload.empty()) {
    throw invalid_argument("Intake rejected: the request is empty");
  }
  int id = nextId++;
  Request &request = requests[id];
  request.requester = actorId;
  request.status = "Open";
  request.payload = payload;
  request.history.push_back("submitted by participant " + to_string(actorId));
  return id;
}

// an Open request can be taken by one participant at a time
void WorkflowEngineFacade::takeOwnership(int actorId, int instanceId) {
  Request &request = find(instanceId);
  if (request.status != "Open") {
    throw logic_error("Request is " + request.status + " and cannot be taken");
  }
  if (request.owner != 0 && request.owner != actorId) {
    throw logic_error("Request is already owned by participant " + to_string(request.owner));
  }
  request.owner = actorId;
  request.history.push_back("taken by participant " + to_string(actorId));
}

//only the owner can complete tasks, and each task only once
void WorkflowEngineFacade::completeTask(int actorId, int instanceId, int taskId) {
  Request &request = find(instanceId);
  if (request.status != "Open") {
    throw logic_error("Request is " + request.status + ", tasks can no longer change");
  }
  if (request.owner != actorId) {
    throw logic_error("Take ownership of the request first");
  }
  if (taskId < 1) {
    throw invalid_argument("no such task");
  }
  for (size_t i = 0; i < request.completedTasks.size(); ++i) {
    if (request.completedTasks[i] == taskId) {
      throw logic_error("Task " + to_string(taskId) + " is already completed");
    }
  }
  request.completedTasks.push_back(taskId);
  request.history.push_back("task " + to_string(taskId) + " completed by participant " + to_string(actorId));
}

// approving needs a request that someone has taken (no amount-based authority yet)
void WorkflowEngineFacade::approveRequest(int actorId, int instanceId) {
  Request &request = find(instanceId);
  if (request.status != "Open" || request.owner == 0) {
    throw logic_error("Only a request that has been taken can be approved");
  }
  request.status = "Approved";
  request.history.push_back("approved by participant " + to_string(actorId));
}

//same rule as approving, and the reason is kept in the history
void WorkflowEngineFacade::rejectRequest(int actorId, int instanceId, const string &reason) {
  Request &request = find(instanceId);
  if (request.status != "Open" || request.owner == 0) {
    throw logic_error("Only a request that has been taken can be rejected");
  }
  request.status = "Rejected";
  request.history.push_back("rejected by participant " + to_string(actorId) + ": " + reason);
}

//any request that is not already finished can be cancelled
void WorkflowEngineFacade::cancelRequest(int actorId, int instanceId) {
  Request &request = find(instanceId);
  if (request.status != "Open") {
    throw logic_error("Request is already " + request.status);
  }
  request.status = "Cancelled";
  request.history.push_back("cancelled by participant " + to_string(actorId));
}

//reverses the most recently completed task
void WorkflowEngineFacade::undoLastAction(int, int instanceId) {
  Request &request = find(instanceId);
  if (request.status != "Open" || request.completedTasks.empty()) {
    throw logic_error("Nothing to undo");
  }
  int taskId = request.completedTasks.back();
  request.completedTasks.pop_back();
  request.history.push_back("undo: task " + to_string(taskId) + " reopened");
}

//memento idk
void WorkflowEngineFacade::saveProgress(int, int instanceId) {
  find(instanceId).history.push_back("progress saved");
}

string WorkflowEngineFacade::getStatus(int, int instanceId) const {
  const Request &request = find(instanceId);
  return request.status + (request.owner != 0 ? " (owner " + to_string(request.owner) + ")" : "");
}

//equest's history, oldest first
vector<string> WorkflowEngineFacade::getHistory(int, int instanceId) const {
  return find(instanceId).history;
}

vector<int> WorkflowEngineFacade::listInstanceIds(int) const {
  vector<int> ids;
  for (map<int, Request>::const_iterator it = requests.begin(); it != requests.end(); ++it) {
    ids.push_back(it->first);
  }
  return ids;
}  
