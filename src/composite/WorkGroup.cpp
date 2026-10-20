#include "Composite.h"

WorkGroup::WorkGroup(const WorkGroup& other) : WorkComponent(other){
    for(size_t i = 0; i < children.size(); i++){
        children.push_back(other.children[i]->clone());
    }
}

void WorkGroup::add(WorkComponent* child) {
    children.push_back(child);
}

void WorkGroup::remove(WorkComponent* child){
    for(size_t i =0 ; i < children.size() ; i++){
        if(children[i] == child){
            children.erase(vector.begin() + i);
        }
    }
}

vector<WorkComponent*> WorkGroup::getChildren(){
    return children;
}

void WorkGroup::start(){
    WorkComponent::start();
    for(size_t i = 0; i < children.size(); i++){
        children[i]->start();
    }
}

void WorkGroup::review(){
    WorkComponent::review();
    for(size_t i = 0; i < children.size(); i++){
        children[i]->review();
    }
}

void WorkGroup::completeReview(){
    WorkComponent::completeReview();
    for(size_t i = 0; i < children.size(); i++){
        children[i]->completeReview();
    }
}

void WorkGroup::complete() {
    WorkComponent::complete();
    for(size_t i = 0; i < children.size(); i++) {
        children[i]->complete();
    }
}

Handler* WorkGroup::clone(){
    WorkGroup(this*);
}

