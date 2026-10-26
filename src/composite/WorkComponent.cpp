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

void WorkComponent::setData(std::string key, std::string value){
    item->setData(key, value);
}

void WorkComponent::setState(WorkState* state){
    delete this->state;
    this->state = state;
}


