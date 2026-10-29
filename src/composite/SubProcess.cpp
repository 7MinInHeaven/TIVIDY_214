#include "Composite.h"

SubProcess::SubProcess(std::string name) : WorkGroup(name) {}

void SubProcess::accept(WorkVisitor& visitor) const {
    visitor.visit(*this);
}

SubProcess* SubProcess::clone() const{
    return new SubProcess(*this);
}