#ifndef WORKFLOWSUBJECT_H
#define WORKFLOWSUBJECT_H

#include "WorkflowObserver.h"
#include <vector>

class WorkflowSubject {
protected:
    std::vector<WorkflowObserver*> observerList;

public:
    WorkflowSubject();
    virtual ~WorkflowSubject();

    void attach(WorkflowObserver* observer);
    void detach(WorkflowObserver* observer);
    void notify();
};

#endif