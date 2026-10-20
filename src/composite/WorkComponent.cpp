#include "Composite.h"

WorkComponent::WorkComponent(std::string name) : name(name){
    item = new WorkItem();
    state = new PendingState();
}

WorkComponent::WorkComponent(const WorkComponent& other){
    state = state->clone();
    item = new item(other.item*);
    name = other.name;
}

WorkState* getState() {
    return state;
}

std::string getName() {
    return name;
}

std::string WorkComponent::getData(std::string key){
    return item->getData(key);
}

void WorkComponent::setData(std::string key, std::string value){
    item->setData(key, value);
}

//this will change depending on state function names
void WorkComponent::start(){
    state->start();
}

void WorkComponent::review(){
    state->review();
}

void WorkComponent::completeReview() {
    state->completeReview();
}

void WorkComponent::complete() {
    state->complete();
}

void accept(WorkVisitor& visitor){
    visitor->visit(this*);
}

Handler* clone(){
    return WorkComponent(this*);
}
