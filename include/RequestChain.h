#ifndef REQUESTCHAIN_H
#define REQUESTCHAIN_H

#include <map>
#include <string>
#include <vector>
#include "Command.h"
#include "Composite.h"
#include "Director.h"
#include "WorkItem.h"

class RequestChain {
public:
    RequestChain(const std::map<std::string, std::string>& payload);
    ~RequestChain();
    bool build(const json& blueprint);
    void indexTasks();
    const std::vector<WorkGroup*>& groups() const;
    int taskCount() const;
    std::vector<std::string> describeTasks() const;
    bool isFinished() const;
    bool completeTask(const std::string& workerId, int taskId);
    int undoLastCompletion();
    void resume();

private:
    struct Step 
    {
        int taskId;
        int commands;       
    };
    RequestChain(const RequestChain&) = delete;
    RequestChain& operator=(const RequestChain&) = delete;
    int unfinishedGroup() const;
    void collectTasks(WorkComponent* component, size_t groupIndex);
    bool run(Action* action, int& executed);
    void rollback(int count);
    WorkItem item;
    std::vector<WorkGroup*> chain;          
    std::vector<WorkComponent*> tasks;     
    std::vector<size_t> taskGroup;         
    ActionExecutor executor;
    std::vector<Step> steps;
};

#endif
