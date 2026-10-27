#include "WorkflowBuilder.h"
#include "Composite.h"

WorkflowBuilder::WorkflowBuilder() {}
WorkflowBuilder::~WorkflowBuilder() {}

WorkGroup* WorkflowBuilder::currentGroup() const 
{
    return stack.back();
}

void WorkflowBuilder::attach(WorkGroup* group)
{
    if (stack.empty()) 
    {
        workGroups.push_back(group);
        if (head == nullptr) {
            head = group;
            tail = group;
        } else {
            tail->setNext(group);
            tail = group;
        }
    } 
    else 
    {
        //attach nested processes to current Workgroup
        WorkGroup* parent = currentGroup();

        // if (Stage* stageChild = dynamic_cast<Stage*>(group)) 
        // {
        //     parent->add(stageChild);
        // } 
        // else if (SubProcess* subChild = dynamic_cast<SubProcess*>(group)) 
        // {
        //     parent->add(subChild);
        // } else 
        // {
        //     parent->add(reinterpret_cast<Stage*>(group));
        // }
        parent->add(group);
    }

    //place workgroup on stack so other processes can be added to it 
    stack.push_back(group);
}

void WorkflowBuilder::reset()
{
    head = nullptr;
    tail = nullptr;
    workGroups.clear();
    stack.clear();
}

void WorkflowBuilder::buildWorkGroup(const std::string& name)
{
    WorkGroup* group = new WorkGroup(name);
    attach(group);
}
    

void WorkflowBuilder::buildStage(const std::string& name)
{
    Stage* stage = new Stage(name);
    attach(stage);
}
    

void WorkflowBuilder::buildSubProcess(const std::string& name) 
{
    SubProcess* sub = new SubProcess(name);
    attach(sub);
}
    

void WorkflowBuilder::buildWorkUnit(const std::string& name)
{
    WorkGroup* parent = currentGroup();
    if (parent == nullptr) {
        return;
    }

    WorkUnit* unit = new WorkUnit(name);
    parent->add(unit);
}
    
// void WorkflowBuilder::addDecorator(const std::string& type,const std::vector<std::string>& requiredKeys = {})
// {

// }
    
// void WorkflowBuilder::addRule(const std::string& key, const std::string& op, const std::string& value)
// {
//     WorkGroup* target = currentGroup();
//     if (target == nullptr) {
//         return;
//     }

//     //add rule to only workgroups

// }
    
    
void WorkflowBuilder::endGroup() 
{
    if (!stack.empty()) 
    {
        stack.pop_back();
    }
}
    
    
Handler* WorkflowBuilder::getResult()
{
    return head;
}