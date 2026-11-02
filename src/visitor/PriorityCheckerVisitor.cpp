#include "WorkVisitor.h"
#include "Composite.h"
#include "Decorator.h"

void PriorityCheckerVisitor::visit(const WorkUnit&){}
void PriorityCheckerVisitor::visit(const WorkGroup&){}
void PriorityCheckerVisitor::visit(const Stage&){}
void PriorityCheckerVisitor::visit(const SubProcess&){}
void PriorityCheckerVisitor::visit(const AuditedTask& item){item.getWrappedTask()->accept(*this);}
void PriorityCheckerVisitor::visit(const ValidatedTask& item){item.getWrappedTask()->accept(*this);}
void PriorityCheckerVisitor::visit(const PrioritisedTask& item){
    found = true;
    level = computeRank(item);
}

void PriorityCheckerVisitor::reset(){
    found = false;
    level = 0;
}

int PriorityCheckerVisitor::computeRank(const PrioritisedTask& item){
    int level = static_cast<int>(item.getPriority());
    return item.isOverdue() ? level + overduePenalty : level;
}