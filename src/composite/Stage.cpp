#include "Composite.h"

void Stage::accept(WorkVisitor& visitor){
    visitor.visit(*this);
}

void Stage::add(SubProcess* child){
    WorkGroup::add(child);
}

void Stage::add(WorkUnit* child){
    WorkGroup::add(child);
}