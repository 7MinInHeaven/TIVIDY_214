#include "Iterator.h"

PriorityIterator::PriorityIterator(WorkComponent* root){
    for(DepthFirstIterator it(root); !it.isDone(); it.next()){
        WorkComponent* node = it.getCurrent();

        checker.reset();
        node->accept(checker);
        if(checker.found){
            elements.emplace_back(checker.level, node);
        }
    }

    std::stable_sort(elements.begin(), elements.end(), 
    [](const std::pair<int, WorkComponent*>&a, const std::pair<int, WorkComponent*>& b){
        return a.first > b.first;
    });
}

void PriorityIterator::first(){
    index = 0;
}

void PriorityIterator::next(){
    if(!isDone()){
        ++index;
    }
}

bool PriorityIterator::isDone(){
    return index >= elements.size();

}

WorkComponent* PriorityIterator::getCurrent(){
    return isDone() ? nullptr : elements[index].second;
}