#include "WorkflowEngineFacade.h"

WorkflowEngineFacade::WorkflowEngineFacade() {
  curl_global_init(CURL_GLOBAL_DEFAULT);
}
WorkflowEngineFacade::~WorkflowEngineFacade() { curl_global_cleanup(); }

void WorkflowEngineFacade::submitRequest(WorkItem item) {
  //(Future Chain of Responsibility && Builder integration)
  // if (masterTemplate) {
  //     Handler* chain = masterTemplate->clone();
  //     chain->handleRequest(item);
  //     delete chain;
  // }

  // 1. Setup Transport (Mailjet if credentials set, else Console)
  unique_ptr<EmailTransport> transport;
  const char *key = getenv("MJ_APIKEY_PUBLIC");
  const char *secret = getenv("MJ_APIKEY_PRIVATE");
  const char *from = getenv("MJ_FROM_EMAIL");
  if (key && secret && from) {
    bool sandbox = (getenv("MJ_SANDBOX") != nullptr);
    cout << "[Init] Using Mailjet Adapter "
         << (sandbox ? "(SANDBOX)" : "(LIVE)") << "\n";
    transport.reset(
        new MailjetEmailAdapter(key, secret, from, "TIVIDY Support", sandbox));
  } else {
    cout << "[Init] Missing Mailjet credentials. Using Console Adapter.\n";
    transport.reset(new ConsoleEmailAdapter(cout));
  }
  // 2. Setup Channels
  EmailChannel emailChannel(transport.get());
  AppChannel appChannel;
  // 3. Fallback defaults matching teammate's original main
  string customerEmail = item.hasData("email")
                             ? item.getData("email")
                             : "your-personal-email@gmail.com";
  string supportAgentId =
      item.hasData("agent_id") ? item.getData("agent_id") : "AGENT-442";
  // 4. Create Notifications passing the WorkItem context
  ResultNotice emailResult(&emailChannel, &item, customerEmail);
  ResultNotice appResult(&appChannel, &item, supportAgentId);
  DeadlineNotice deadlineAlert(&appChannel, &item, supportAgentId);
  // 5. Send!
  emailResult.send();
  appResult.send();
  deadlineAlert.send();
  // 6. Check for Mailjet Errors
  if (auto *mj = dynamic_cast<MailjetEmailAdapter *>(transport.get())) {
    if (!mj->lastError().empty()) {
      cerr << "\n[MAILJET ERROR] " << mj->lastError() << "\n";
    }
  }
  cout << "\n========================================\n";
  cout << "   All Notifications Processed          \n";
  cout << "========================================\n";
}