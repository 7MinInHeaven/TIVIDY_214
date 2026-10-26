#ifndef WORKFLOWOBSERVER_H
#define WORKFLOWOBSERVER_H

class WorkflowObserver {
public:
    virtual ~WorkflowObserver() {}
    virtual void update() = 0;
};

#endif