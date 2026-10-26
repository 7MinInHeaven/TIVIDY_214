#include "Composite.h"

Stage::Stage(std::string name) : WorkGroup(name){}

void Stage::accept(WorkVisitor& visitor){
    visitor.visit(*this);
}

void Stage::add(SubProcess* child){
    WorkGroup::add(child);
}

void Stage::add(WorkUnit* child){
    WorkGroup::add(child);
}

Stage* Stage::clone() {
    return new Stage(*this);
}