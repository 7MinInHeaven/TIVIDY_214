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
    // mo
    struct Request {
      Request() : requester(0), owner(0) {}
      int requester;  // who submitted it
      int owner; // who took it (0 = nobody yet)
      string status; // Open, Approved, Rejected or Cancelled
      vector<string> history;
      vector<int> completedTasks; // task ids, in the order they were completed
      map<string, string> payload;
    };
    Request &find(int instanceId);
    const Request &find(int instanceId) const;
    int nextId; // mo, next request id to hand out
    map<int, Request> requests; //mo , all requests by id
  public:
    WorkflowEngineFacade();
    ~WorkflowEngineFacade();
    void submitRequest(WorkItem item);   // mo - old notification entry point kept so test_facade.cpp still works
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