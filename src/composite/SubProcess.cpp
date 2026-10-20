#include "Composite.h"

void SubProcess::accept(WorkVisitor& visitor){
    visitor.visit(*this);
}

void SubProcess::add(WorkUnit* child){
    WorkGroup::add(child);
}