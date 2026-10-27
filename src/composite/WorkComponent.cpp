#include "Composite.h"

WorkComponent::WorkComponent(std::string name) : name(name){
    item = new WorkItem();
    state = new PendingState();
}

WorkComponent::WorkComponent(const WorkComponent& other){
    state = other.state->clone();
    item = new WorkItem(*other.item);
    name = other.name;
}

WorkComponent::~WorkComponent() {
    delete state;
    delete item;
}

WorkState* WorkComponent::getState() const {
    return state;
}   

std::string WorkComponent::getName() const {
    return name;
}

WorkItem* WorkComponent::getItem() const{
    return item;
}

void WorkComponent::setData(const std::string& key, const std::string& value){ //mo
    item->setData(key, value);
}


//mo default implemenations, for simplikity
std::string WorkComponent::getData(const std::string& key) const {
    return item->getData(key);
}

void WorkComponent::nextState() {
}

bool WorkComponent::isComplete() const {
    return false; 
}

PriorityLevel WorkComponent::getPriority() const {
    return PriorityLevel::Low; //fallback
}





void WorkComponent::setState(WorkState* state){
    delete this->state;
    this->state = state;
}


