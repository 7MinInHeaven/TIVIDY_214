#ifndef COMPOSITE_H
#define COMPOSITE_H

#include <string>
#include "WorkFlowSubject.h"
#include "WorkState.h"


#include "WorkItem.h"

class WorkMemento;

class WorkComponent 
{
public:
    WorkComponent(std::string name);
    WorkComponent(const WorkComponent& other);
    virtual ~WorkComponent();

private:
    WorkState* state;
   WorkItem* item;
    std::string name;
};



class WorkUnit : public WorkComponent {
    private:
        std::string taskId;
        WorkState* currentState;
        std::string assignedWorkerId;

    public:
    WorkUnit(const std::string& taskId);
    WorkMemento* createMemento();
    void setMemento(WorkMemento* memento);
    bool startTask();
    bool completeTask();
    bool assignTask(const std::string& workerId);
    WorkState* getState();
    std::string getTaskId();
    std::string getAssignedWorkerId();
    
};


#endif