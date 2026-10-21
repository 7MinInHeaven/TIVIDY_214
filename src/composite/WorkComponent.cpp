#include "Composite.h"
#include "PendingState.h"
#include "CompletedState.h"
#include "InProgressState.h"

WorkComponent::WorkComponent(std::string name) : name(name){
    item = new WorkItem();
    state = new PendingState();
}

WorkComponent::WorkComponent(const WorkComponent& other){
    state = state->clone();
    item = new WorkItem(*other.item);
    name = other.name;
}

WorkState* WorkComponent::getState() {
    return state;
}

std::string WorkComponent::getName() {
    return name;
}

std::string WorkComponent::getData(std::string key){
    return item->getData(key);
}

void WorkComponent::setData(std::string key, std::string value){
    item->setData(key, value);
}

void WorkComponent::setState(WorkState* state)
{
    if (state != nullptr)
    {
        delete this->state;
    }
    this->state = state;
}