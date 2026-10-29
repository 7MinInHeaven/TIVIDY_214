#include "Iterator.h"

FilterIterator::FilterIterator(WorkComponent* root, std::function<bool(WorkComponent*)> filter) : DepthFirstIterator(root), filter(filter){
    skipFilter();
}

void FilterIterator::first() {
    DepthFirstIterator::first();
    skipFilter();
}

void FilterIterator::next() {
    DepthFirstIterator::next(); 
    skipFilter();
}

void FilterIterator::skipFilter() {
    while(!isDone() && !filter(getCurrent())){
        DepthFirstIterator::next();
    }
}
