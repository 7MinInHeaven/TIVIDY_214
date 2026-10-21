#include "WorkflowBuilder.h"
#include "Composite.h"

void WorkflowBuilder::reset()
{
    head = nullptr;
    tail = nullptr;
}

void WorkflowBuilder::buildNestedProcess(const std::string& config)
{

    if (config == "Init")
    {
        if (!head) {head = new WorkGroup("Init");}
    }
    else if (config == "S1")
    {
        if (head)
        {
            head->setNext(new WorkGroup("STEP 1"));
        }
        else 
        {
            head = new WorkGroup("Init");
        }
    }
    else if (config == "Final") 
    {
        if (head)
        {
            head->setNext(new WorkGroup("Final"));
        }
    }

}
    
Handler* WorkflowBuilder::getResult()
{
    return head;
}
