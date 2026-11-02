#include "WorkVisitor.h"
#include "Composite.h"
#include "Decorator.h"

bool WorkComponentValidator::validate(WorkComponent* parent, WorkComponent* child){
    Kind parentKind = KindFinder(parent).getKind();
    Kind childKind = KindFinder(child).getKind();

    //enum formatted such that lower int can have higher int as child
    //i.e 1 can have 3 as child
    return static_cast<int>(parentKind) < static_cast<int>(childKind);
}

WorkComponentValidator::KindFinder::KindFinder(WorkComponent* item){item->accept(*this);}

void WorkComponentValidator::KindFinder::visit(const WorkGroup&) {kind = Kind::WorkGroup;}
void WorkComponentValidator::KindFinder::visit(const Stage&) {kind = Kind::Stage;}
void WorkComponentValidator::KindFinder::visit(const SubProcess&) {kind = Kind::SubProcess;}
void WorkComponentValidator::KindFinder::visit(const WorkUnit&) {kind = Kind::WorkUnit;}

void WorkComponentValidator::KindFinder::visit(const PrioritisedTask& item) {unwrap(item.getWrappedTask());}
void WorkComponentValidator::KindFinder::visit(const AuditedTask& item) {unwrap(item.getWrappedTask());}
void WorkComponentValidator::KindFinder::visit(const ValidatedTask& item) {unwrap(item.getWrappedTask());}
WorkComponentValidator::Kind WorkComponentValidator::KindFinder::getKind() {return kind;}
void WorkComponentValidator::KindFinder::unwrap(WorkComponent* wrapped){wrapped->accept(*this);}