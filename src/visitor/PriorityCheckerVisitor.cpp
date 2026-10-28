#include "WorkVisitor.h"

PriorityCheckerVisitor::~PriorityCheckerVisitor(){
    found.clear();
}

void PriorityCheckerVisitor::visit(WorkUnit& item){}
void PriorityCheckerVisitor::visit(WorkGroup& item){}

void PriorityCheckerVisitor::visit(PrioritisedTask& item){
    found.push_back(&item);
}