#include "Composite.h"

Stage::Stage(std::string name) : WorkGroup(name){}

void Stage::accept(WorkVisitor& visitor) const{
    visitor.visit(*this);
}

Stage* Stage::clone() const {
    return new Stage(*this);
}