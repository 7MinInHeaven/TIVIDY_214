#ifndef WORKFLOWOBSERVER_H
#define WORKFLOWOBSERVER_H

class WorkComponent;

class WorkflowObserver {
public:
    virtual ~WorkflowObserver() {}
     virtual void setSubject(WorkComponent*) {}
};

#endif