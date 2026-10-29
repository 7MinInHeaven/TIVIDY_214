#include "Iterator.h"

StateIterator::StateIterator(WorkComponent* root, std::string state) : FilterIterator(root, [state](WorkComponent* c){
    return state == c->getState()->getStatus();
}){}