#include "Composite.h"

DepthFirstIterator::DepthFirstIterator(WorkComponent* root) : root(root){
    recursive(root);
}
void DepthFirstIterator::first(){index = 0;}
void DepthFirstIterator::next(){
    if(!isDone()){
        ++index;
    }
}
bool DepthFirstIterator::isDone(){
    return index >= elements.size();
}
void DepthFirstIterator::recursive(WorkComponent* node){
    elements.push_back(node);

    for(WorkComponent* c : node->getChildren()){
        recursive(children[i]);
    }
}
WorkComponent* DepthFirstIterator::getCurrent(){
    return isDone() ? nullptr : elements[index];
} 