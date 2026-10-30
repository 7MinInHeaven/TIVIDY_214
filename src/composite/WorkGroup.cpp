#include "Composite.h"
#include "Command.h"
#include "PendingState.h"
#include "InProgressState.h"
#include "CompletedState.h"
#include <iostream>

WorkGroup::WorkGroup(std::string name) : WorkComponent(name), interactive(true) {}
WorkGroup::WorkGroup(const WorkGroup& other) : WorkComponent(other), interactive(other.interactive){
    for(size_t i = 0; i < other.children.size(); i++){
        children.push_back(other.children[i]->clone());
        children.back()->setParent(this);
    }
}

WorkGroup::~WorkGroup(){
    for(size_t i = 0; i < children.size(); i++){
        delete children[i];
    } 

    children.clear();
}

bool WorkGroup::add(WorkComponent* child) {
    if(!WorkComponentValidator::validate(this, child)) {
        delete child;
        return false;
    }
    for(size_t i = 0; i < children.size(); i++){
        if(children[i] == child) {
            delete child;
            return false;
        }
        if(children[i]->getName() == child->getName()) {
            delete child;
            return false;
        }
    }

    children.push_back(child);
    child->setParent(this);
    refreshState();
    return true;
}

void WorkGroup::remove(WorkComponent* child){
    for(size_t i =0 ; i < children.size() ; i++){
        if(children[i] == child){
            delete children[i];
            children.erase(children.begin() + i);
        }
    }
    refreshState();
}

void WorkGroup::remove(std::string name){
    for(size_t i =0 ; i < children.size() ; i++){
        if(children[i]->getName() == name){
            delete children[i];
            children.erase(children.begin() + i);
        }
    }
    refreshState();
}

void WorkGroup::accept(WorkVisitor& visitor) const
{
    visitor.visit(*this);
}

WorkGroup* WorkGroup::clone() const {
    return new WorkGroup(*this);
}

const std::vector<WorkComponent*> WorkGroup::getChildren() const{
    return children;
}


void WorkGroup::printTasks(WorkComponent* component)
{
    WorkGroup* group = dynamic_cast<WorkGroup*>(component);
    if (group == nullptr) return;               

    for (WorkComponent* child : group->getChildren())
    {
        std::cout << "sub task: " << child->getName() << "\n";
        printTasks(child);           
    }
}

void WorkGroup::setInteractive(bool on)
{
    interactive = on;
}

void WorkGroup::refreshState()
{
    if (children.empty()) {
        return;
    }

    size_t completed = 0;
    bool started = false;
    for (size_t i = 0; i < children.size(); i++)
    {
        std::string status = children[i]->getStatus();
        if (status == "Completed")
        {
            completed++;
            started = true;
        }
        else if (status == "In Progress")
        {
            started = true;
        }
    }

    std::string wanted = "Pending";
    if (completed == children.size())
    {
        wanted = "Completed";
    }
    else if (started)
    {
        wanted = "In Progress";
    }

    if (getStatus() == wanted) {
        return;                         
    }

    if (wanted == "Completed")
    {
        setState(new CompletedState());
    }
    else if (wanted == "In Progress")
    {
        setState(new InProgressState());
    }
    else
    {
        setState(new PendingState());
    }
}

static void completeLeaves(WorkComponent* component, ActionExecutor& ae)
{
    WorkGroup* group = dynamic_cast<WorkGroup*>(component);
    if (group != nullptr)
    {
        std::vector<WorkComponent*> kids = group->getChildren();
        for (size_t i = 0; i < kids.size(); i++)
        {
            completeLeaves(kids[i], ae);
        }
        return;
    }

    if (component->getStatus() == "Pending")
    {
        ae.executeCommand(new AssignTaskCommand(component, "console-user"));   
        ae.executeCommand(new StartTaskCommand(component));
    }
    if (component->getStatus() == "In Progress")
    {
        ae.executeCommand(new CompleteTaskCommand(component));
    }
}

void WorkGroup::handleRequest(WorkItem item)
{
    std::cout << "Work Group: " + getName() + " is " + getStatus() + "\n";

    if (!isComplete() && !children.empty() && interactive)
    {
        std::cout << "Complete all tasks in " + getName() + " now? (Y/N)\n";
        std::string input;
        std::cin >> input;
        if (input == "Y")
        {
            ActionExecutor ae;
            completeLeaves(this, ae);       // the group's state follows its children
        }
    }

    if (!isComplete() && !children.empty())
    {
        std::cout << "Chain paused at " + getName() + "\n";
        return;                          
    }

    if (interactive)
    {
        printTasks(this);
    }
    Handler::handleRequest(item);
}


// WorkMemento* WorkGroup::createMemento() {
//   return new WorkMemento(0, state, 0);
// }

// void WorkGroup::setMemento(WorkMemento* memento) {
//   if (!memento)
//     return;
//   taskId = memento->taskId;
//   currentState = memento->currentState;
//   assignedWorkerId = memento->assignedWorkerId;
// }


bool WorkGroup::startTask() {
  std::cout << getName() + " cannot be started directly: its state follows its tasks\n";
  return false;
}

bool WorkGroup::completeTask() {
  std::cout << getName() + " cannot be completed directly: its state follows its tasks\n";
  return false;
}

bool WorkGroup::assignTask(const std::string &task) {
  name = task;
  return true;
}