#include "WorkVisitor.h"
#include "Composite.h"
#include "Decorator.h"

PriorityCheckerVisitor::~PriorityCheckerVisitor(){
    found.clear();
}

void PriorityCheckerVisitor::visit(const WorkUnit&){}
void PriorityCheckerVisitor::visit(const WorkGroup& item){
    for(WorkComponent* c : item.getChildren()){
        c->accept(*this);
    }
}
void PriorityCheckerVisitor::visit(const Stage& item){
    for(WorkComponent* c : item.getChildren()){
        c->accept(*this);
    }
}
void PriorityCheckerVisitor::visit(const SubProcess& item){
    for(WorkComponent* c : item.getChildren()){
        c->accept(*this);
    }
}
void PriorityCheckerVisitor::visit(const AuditedTask&){}
void PriorityCheckerVisitor::visit(const ValidatedTask&){}

void PriorityCheckerVisitor::visit(const PrioritisedTask& item){
    found.push_back(&item);
}