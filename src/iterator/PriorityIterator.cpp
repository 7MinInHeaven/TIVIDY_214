#include "Iterator.h"

PriorityIterator::PriorityIterator(WorkComponent* root) : DepthFirstIterator(root){
    std::stable_sort(elements.begin(), elements.end(), 
        [](WorkComponent* a, WorkComponent* b){
            return a->getPriority() > b->getPriority();
        });
}