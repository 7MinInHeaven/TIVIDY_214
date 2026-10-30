#include "Composite.h"
#include "Command.h"

WorkGroup::WorkGroup(std::string name) : WorkComponent(name) {}
WorkGroup::WorkGroup(const WorkGroup& other) : WorkComponent(other){
    for(size_t i = 0; i < other.children.size(); i++){
        children.push_back(other.children[i]->clone());
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
    return true;
}

void WorkGroup::remove(WorkComponent* child){
    for(size_t i =0 ; i < children.size() ; i++){
        if(children[i] == child){
            delete children[i];
            children.erase(children.begin() + i);
        }
    }
}

void WorkGroup::remove(std::string name){
    for(size_t i =0 ; i < children.size() ; i++){
        if(children[i]->getName() == name){
            delete children[i];
            children.erase(children.begin() + i);
        }
    }
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

void WorkGroup::handleRequest(WorkItem item)
{
    std::cout << "Work Group: " + getName() + " is starting\n";
    ActionExecutor ae;


    ae.executeCommand(new StartTaskCommand(this));

    std::cout << "Have you completed " + getName() +"?\n";
    std::string input;
    std::cin >> input;
    if (input == "Y")
    {
        ae.executeCommand(new CompleteTaskCommand(this));
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
  if (state) {
    state->handleUpdate(this);
  }
  return true;
}

bool WorkGroup::completeTask() {
  if (state) {
    state->handleUpdate(this);
  }
  return true;
}

bool WorkGroup::assignTask(const std::string &task) {
  name = task;
  return true;
}


