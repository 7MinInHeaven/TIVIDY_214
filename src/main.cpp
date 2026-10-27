#include "MailjetEmailAdapter.h"
#include "NotificationBridge.h"
#include "WorkItem.h"
#include "WorkflowEngineFacade.h"
#include <cstdlib>
#include <curl/curl.h>
#include <iostream>
#include <memory>

using namespace std;

int main() {
  cout << "========================================\n";
  cout << "   TIVIDY: Generic Workflow Engine      \n";
  cout << "========================================\n\n";

  WorkflowEngineFacade facade;
   WorkItem item;
  item.setData("id", "REQ-001");
  item.setData("category", "Plumbing");
  item.setData("email", "u25@tuks.co.za"); 
  item.setData("state", "Pending");
  item.setData("agent_id", "AGENT-007");

  // 3. Submit the request to the system
  facade.submitRequest(item);
  

  return 0;
}