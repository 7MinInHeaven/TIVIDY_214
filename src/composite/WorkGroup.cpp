#include "Composite.h"
#include <iostream>

WorkGroup::WorkGroup(std::string name) : WorkComponent(name) {}

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
            children.erase(children.begin() + i);
        }
    }
}

std::vector<WorkComponent*> WorkGroup::getChildren(){
    return children;
}

void WorkGroup::handleRequest(WorkItem item) {
    std::cout << "stage: " << getName() << "\n";

    for (WorkComponent* child : children) {
        std::cout << "  sub task: " << child->getName() << "\n";
    }
    Handler::handleRequest(item);
}

