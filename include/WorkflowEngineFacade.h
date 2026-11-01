#ifndef WORKFLOWENGINEFACADE_H
#define WORKFLOWENGINEFACADE_H

#include "Composite.h"
#include "RequestChain.h"
#include "DatabaseObserver.h"
#include "PostgresAdapter.h"
#include "PostgresAPI.h"
#include "SQLiteAdapter.h"
#include "SQLiteAPI.h"
#include "MailjetEmailAdapter.h"
#include "NotificationBridge.h"
#include "WorkflowScenarioCatalog.h"
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
      map<string, string> payload;
      unique_ptr<RequestChain> chain; // this request's WorkGroups, task states and commands
    };
    Request &find(int instanceId);
    const Request &find(int instanceId) const;
    int nextId; // mo, next request id to hand out
    map<int, Request> requests; //mo , all requests by id

    //Robert's stuff
    DatabaseObserver* dbObserver;
    WorkflowScenarioCatalog scenarioCatalog;
    void attachObserverToTree(WorkComponent* component);

  public:
    WorkflowEngineFacade();
    ~WorkflowEngineFacade();

    /**
     * @brief Refreshes the available JSON workflow scenarios.
     * @return Errors encountered while skipping invalid scenario files.
     */
    std::vector<std::string> refreshScenarios();

    /**
     * @brief Returns the scenarios discovered by the last refresh.
     */
    const std::vector<WorkflowScenario>& listScenarios() const;

    /**
     * @brief Executes the default workflow blueprint.
     */
    void submitRequest(WorkItem item);

    /**
     * @brief Executes a selected workflow blueprint without changing the engine patterns.
     * @param item Work item passed through the workflow.
     * @param blueprintPath Validated scenario file to execute.
     */
    void submitRequest(WorkItem item, const std::string& blueprintPath);

    int submitRequest(int actorId, const std::map<std::string, std::string>& payload) override;

    /**
     * @brief Same as above, but the request's workflow is built from the given blueprint file.
     */
    int submitRequest(int actorId, const std::map<std::string, std::string>& payload, const std::string& blueprintPath);
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

    /**
     * @brief One line per task: number, name, state and the group it belongs to.
     */
    std::vector<std::string> getTaskStates(int actorId, int instanceId) const;
    /**
     * @brief Generates an HTML progress report for a specific workflow group.
     * @return The total number of groups in the request's chain.
     */
    int writeProgressReport(int actorId, int instanceId, int groupIndex) const; // mo added
};

#endif

// #ifndef WORKFLOWENGINEFACADE_H
// #define WORKFLOWENGINEFACADE_H

// #include "Composite.h"
// #include "DatabaseObserver.h"
// #include "PostgresAdapter.h"
// #include "PostgresAPI.h"
// #include "SQLiteAdapter.h"
// #include "SQLiteAPI.h"
// #include "MailjetEmailAdapter.h"
// #include "NotificationBridge.h"
// #include "WorkflowScenarioCatalog.h"
// #include "WorkItem.h"
// #include <cstdlib>
// #include <curl/curl.h>
// #include <iostream>
// #include <memory>
// #include <string>
// #include <map>
// #include <vector>
// using namespace std;

// class WorkflowService {
//   public:
//       virtual ~WorkflowService() {}

//       virtual int submitRequest(int actorId, const std::map<std::string, std::string>& payload) = 0;
//       virtual void takeOwnership(int actorId, int instanceId) = 0;
//       virtual void completeTask(int actorId, int instanceId, int taskId) = 0;
//       virtual void approveRequest(int actorId, int instanceId) = 0;
//       virtual void rejectRequest(int actorId, int instanceId, const std::string& reason) = 0;
//       virtual void cancelRequest(int actorId, int instanceId) = 0;
//       virtual void undoLastAction(int actorId, int instanceId) = 0;
//       virtual void saveProgress(int actorId, int instanceId) = 0;

//       virtual std::string getStatus(int actorId, int instanceId) const = 0;
//       virtual std::vector<std::string> getHistory(int actorId, int instanceId) const = 0;
//       virtual std::vector<int> listInstanceIds(int actorId) const = 0;
//   };

//   class WorkflowEngineFacade: public WorkflowService {
//   private:
//     // Handler* masterTemplate
//     // mo
//     struct Request {
//       Request() : requester(0), owner(0) {}
//       int requester;  // who submitted it
//       int owner; // who took it (0 = nobody yet)
//       string status; // Open, Approved, Rejected or Cancelled
//       vector<string> history;
//       vector<int> completedTasks; // task ids, in the order they were completed
//       map<string, string> payload;
//     };
//     Request &find(int instanceId);
//     const Request &find(int instanceId) const;
//     int nextId; // mo, next request id to hand out
//     map<int, Request> requests; //mo , all requests by id

//     //Robert's stuff
//     DatabaseObserver* dbObserver;
//     WorkflowScenarioCatalog scenarioCatalog;
//     void attachObserverToTree(WorkComponent* component);

//   public:
//     WorkflowEngineFacade();
//     ~WorkflowEngineFacade();

//     /**
//      * @brief Refreshes the available JSON workflow scenarios.
//      * @return Errors encountered while skipping invalid scenario files.
//      */
//     std::vector<std::string> refreshScenarios();

//     /**
//      * @brief Returns the scenarios discovered by the last refresh.
//      */
//     const std::vector<WorkflowScenario>& listScenarios() const;

//     /**
//      * @brief Executes the default workflow blueprint.
//      */
//     void submitRequest(WorkItem item);

//     /**
//      * @brief Executes a selected workflow blueprint without changing the engine patterns.
//      * @param item Work item passed through the workflow.
//      * @param blueprintPath Validated scenario file to execute.
//      */
//     void submitRequest(WorkItem item, const std::string& blueprintPath);

//     int submitRequest(int actorId, const std::map<std::string, std::string>& payload) override;
//     void takeOwnership(int actorId, int instanceId) override;
//     void completeTask(int actorId, int instanceId, int taskId) override;
//     void approveRequest(int actorId, int instanceId) override;
//     void rejectRequest(int actorId, int instanceId, const std::string& reason) override;
//     void cancelRequest(int actorId, int instanceId) override;
//     void undoLastAction(int actorId, int instanceId) override;
//     void saveProgress(int actorId, int instanceId) override;

//     std::string getStatus(int actorId, int instanceId) const override;
//     std::vector<std::string> getHistory(int actorId, int instanceId) const override;
//     std::vector<int> listInstanceIds(int actorId) const override;
// };

// #endif