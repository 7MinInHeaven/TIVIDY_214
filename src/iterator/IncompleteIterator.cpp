#include "Iterator.h"

IncompleteIterator::IncompleteIterator(WorkComponent* root) : FilterIterator(root, [](WorkComponent* c){
    return c->getState()->getStatus() != "Completed";
}) {}