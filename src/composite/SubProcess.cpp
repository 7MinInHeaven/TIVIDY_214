#include "Composite.h"

SubProcess::SubProcess(std::string name) : WorkGroup(name) {}

void SubProcess::accept(WorkVisitor& visitor){
    visitor.visit(*this);
}

void SubProcess::add(WorkUnit* child){
    WorkGroup::add(child);
}

SubProcess* SubProcess::clone(){
    return new SubProcess(*this);
}