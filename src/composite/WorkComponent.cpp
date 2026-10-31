#include "Composite.h"
#include "Iterator.h"
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>

WorkComponent::WorkComponent(std::string name) : name(name){
    item = new WorkItem();
    state = new PendingState();

    std::string generatedID = generateRandomID();
    for(DepthFirstIterator it(this); !it.isDone(); it.next()){
        WorkComponent* current = it.getCurrent();
        if(current->getID() == generatedID){
            generatedID = generateRandomID();
            it.first(); 
        }
    }
    taskID = generatedID;   
    parent = nullptr;
}
WorkComponent::WorkComponent(const WorkComponent& other){
    state = other.state->clone();
    item = new WorkItem(*other.item);
    name = other.name;
    taskID = other.taskID;
    assignedWorkerID = other.assignedWorkerID;
    parent = nullptr;         
}
WorkComponent::~WorkComponent() {
    delete state;
    delete item;
}

const WorkState* WorkComponent::getState() const {return state;}   
std::string WorkComponent::getStatus() const {return state->getStatus();}
std::string WorkComponent::getName() const {return name;}
WorkItem* WorkComponent::getItem() const {return item;}
std::string WorkComponent::getID() const {return taskID;}
std::string WorkComponent::getAssignedWorkerId() const {return assignedWorkerID;}
std::string WorkComponent::getData(const std::string& key) const {return item->getData(key);}
PriorityLevel WorkComponent::getPriority() const {return PriorityLevel::Normal;} //default prio to Normal

void WorkComponent::setData(const std::string& key, const std::string& value){ //mo
    item->setData(key, value);
    this->notify();
}
void WorkComponent::setState(WorkState* state){
    delete this->state;
    this->state = state;
    this->notify();

    // tell the group we sit in, so its state can follow ours
    if (parent != nullptr) {
        parent->refreshState();
    }
}
void WorkComponent::setParent(WorkGroup* newParent){
    parent = newParent;
}
bool WorkComponent::assignTask(const std::string& workerID){
    assignedWorkerID = workerID; 
    return true;
}
// Pending -> In Progress. Needs an assignee, and any other state is refused.
bool WorkComponent::startTask(){
    if (assignedWorkerID.empty()) {
        std::cout << "Cannot start '" + name + "': nobody is assigned\n";
        return false;
    }
    if (state == nullptr || state->getStatus() != "Pending") {
        std::cout << "Cannot start '" + name + "': it is not Pending\n";
        return false;
    }
    state->handleUpdate(this);
    return true;
}
// In Progress -> Completed. A Pending task has to be started first.
bool WorkComponent::completeTask(){
    if (state == nullptr || state->getStatus() != "In Progress") {
        std::cout << "Cannot complete '" + name + "': it is not In Progress\n";
        return false;
    }
    state->handleUpdate(this);
    return true;
}

// defaults: only WorkUnit saves and restores a memento
WorkMemento* WorkComponent::createMemento(){ return nullptr; }
void WorkComponent::setMemento(WorkMemento*){}

void WorkComponent::nextState() {
    if (state != nullptr) {
        state->handleUpdate(this);
    }//mo
}
bool WorkComponent::isComplete() const {
    return (state != nullptr && state->getStatus() == "Completed");//mo 
}

bool WorkComponent::add(WorkComponent* child){
    delete child;
    return false;
}
void WorkComponent::remove(WorkComponent*){}
void WorkComponent::remove(std::string){}
const std::vector<WorkComponent*> WorkComponent::getChildren() const {return {};}

std::string WorkComponent::generateRandomID() const {
    std::random_device device; //this does nondeterministic seeding, not really necessary but cool (sort of)
    std::mt19937 gen(device()); //Mersenne Twister engine (was recommended, no specific reason)

    //8 digits, unsigned cuz hex
    std::uniform_int_distribution<u_int32_t> dis(0x0, 0xFFFFFFFF);
    u_int32_t id = dis(gen);

    std::ostringstream output;
    //fills it with 0, will always be 8 digits 
    output << std::hex << std::setfill('0') << std::setw(8) << id;

    return output.str();
}