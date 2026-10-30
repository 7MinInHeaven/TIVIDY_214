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
    std::cout << "stage: " << getName() << "\n";
    printTasks(this);                         
    Handler::handleRequest(item);
}
