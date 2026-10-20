#include "Composite.h"

WorkMemento* WorkUnit::createMemento() {
    return new WorkMemento();
}

void WorkUnit::setMemento(WorkMemento*) {

}

void WorkUnit::accept(WorkVisitor& visitor) {
    visitor.visit(*this);
}
