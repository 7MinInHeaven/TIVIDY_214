#ifndef WORKFLOWENGINEFACADE_H
#define WORKFLOWENGINEFACADE_H

#include "MailjetEmailAdapter.h"
#include "NotificationBridge.h"
#include "WorkItem.h"
#include <cstdlib>
#include <curl/curl.h>
#include <iostream>
#include <memory>
#include <string>
using namespace std;

class WorkflowEngineFacade {
private:
  // Handler* masterTemplate
public:
  WorkflowEngineFacade();
  ~WorkflowEngineFacade();
  void submitRequest(WorkItem item);
};

#endif