#ifndef WORKFLOWBUILDER_H
#define WORKFLOWBUILDER_H

#include "Handler.h"


class WorkflowBuilder {
private: 
    Handler* head;
    Handler* tail;

public:
    void reset();
    void buildNestedProcess(const std::string& config);
    Handler* getResult();
};

#endif 