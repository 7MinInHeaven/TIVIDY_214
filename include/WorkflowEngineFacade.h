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
#include <map>
#include <vector>
using namespace std;

class WorkflowService {
  public:
      virtual ~WorkflowService() {}

      virtual int submitRequest(int actorId, const std::map<std::string, std::string>& payload) = 0;
      virtual void takeOwnership(int actorId, int instanceId) = 0;
      virtual void completeTask(int actorId, int instanceId, int taskId) = 0;
      virtual void approveRequest(int actorId, int instanceId) = 0;
      virtual void rejectRequest(int actorId, int instanceId, const std::string& reason) = 0;
      virtual void cancelRequest(int actorId, int instanceId) = 0;
      virtual void undoLastAction(int actorId, int instanceId) = 0;
      virtual void saveProgress(int actorId, int instanceId) = 0;

      virtual std::string getStatus(int actorId, int instanceId) const = 0;
      virtual std::vector<std::string> getHistory(int actorId, int instanceId) const = 0;
      virtual std::vector<int> listInstanceIds(int actorId) const = 0;
  };

  class WorkflowEngineFacade: public WorkflowService {
  private:
    // Handler* masterTemplate
  public:
    WorkflowEngineFacade();
    ~WorkflowEngineFacade();
    int submitRequest(int actorId, const std::map<std::string, std::string>& payload) override;
    void takeOwnership(int actorId, int instanceId) override;
    void completeTask(int actorId, int instanceId, int taskId) override;
    void approveRequest(int actorId, int instanceId) override;
    void rejectRequest(int actorId, int instanceId, const std::string& reason) override;
    void cancelRequest(int actorId, int instanceId) override;
    void undoLastAction(int actorId, int instanceId) override;
    void saveProgress(int actorId, int instanceId) override;

    std::string getStatus(int actorId, int instanceId) const override;
    std::vector<std::string> getHistory(int actorId, int instanceId) const override;
    std::vector<int> listInstanceIds(int actorId) const override;
};

#endif