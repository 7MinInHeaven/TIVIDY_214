#include "Composite.h"

WorkUnit::WorkUnit(std::string name) : WorkComponent(name) {}

WorkMemento* WorkUnit::createMemento() {
    return new WorkMemento();
}

void WorkUnit::setMemento(WorkMemento*) {

}

void WorkUnit::accept(WorkVisitor& visitor) {
    visitor.visit(*this);
}

WorkUnit* WorkUnit::clone() const {
    return new WorkUnit(*this);
}