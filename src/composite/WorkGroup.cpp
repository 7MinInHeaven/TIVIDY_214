#include "Composite.h"

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

//not sure if this is correct
void WorkGroup::add(Stage* child){
    add(static_cast<WorkComponent*>(child));
}

void WorkGroup::add(SubProcess* child){
    add(static_cast<WorkComponent*>(child));
}

void WorkGroup::add(WorkUnit* child){
    add(static_cast<WorkComponent*>(child));
}

void WorkGroup::add(WorkComponent* child) {
    for(size_t i = 0; i < children.size(); i++){
        if(children[i] == child){
            std::cout << "Cannot add the same child twice" << std::endl;
            return;
        }

        if(children[i]->getName() == child->getName()){
            std::cout << "There already exists a child with name: " << child->getName() << std::endl;
            delete child;
            return;
        }
    }

    children.push_back(child);
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

void WorkGroup::accept(WorkVisitor& visitor){
    visitor.visit(*this);
}

WorkGroup* WorkGroup::clone() const {
    return new WorkGroup(*this);
}

std::vector<WorkComponent*> WorkGroup::getChildren() const{
    return children;
}