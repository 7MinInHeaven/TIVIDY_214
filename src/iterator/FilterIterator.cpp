#include "Iterator.h"

FilterIterator::(WorkComponent* root, std::function<bool(WorkComponent*)> filter) : DepthFirstIterator(root), filter(filter){}

void FilterIterator::first() override {
    DepthFirstIterator::first();
    skipFilter();
}

void FilterIterator::next() override {
    DepthFirstIterator::next(); 
    skipFilter;
}

void FilterIterator::skipFilter() {
    while(!isDone() && !filter(getCurrent())){
        DepthFirstIterator::next();
    }
}
