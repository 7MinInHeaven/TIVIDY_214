#include "Iterator.h"

PriorityIterator::PriorityIterator(WorkComponent* root) : DepthFirstIterator(root){
    PriorityCollectorVisitor collector;
    for(DepthFirstIterator it(root); !it.isDone(); it.next()){
        it.getCurrent()->accept(collector);
    }

    elements = std::move(collector.found);
    std::stable_sort(elements.begin(), elements.end(), 
        [](PrioritisedTask* a, PrioritisedTask* b){
            return a->getPriority() > b->getPriority();
        });
}