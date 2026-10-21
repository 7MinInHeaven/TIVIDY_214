#include "../include/Command.h"

ActionExecutor::~ActionExecutor() {
    for (size_t i=0; i<actions.size(); ++i) {
        delete actions[i];
    }
}


bool ActionExecutor::executeCommand(Action* action) {
    WorkMemento* before = action->getReceiver()->createMemento();
    
    if (!action->execute()) {
        delete before;
        delete action;
        return false;
    }

    history.addMemento(before);
    actions.push_back(action);
    return true;
}

bool ActionExecutor::undoLastAction(){
    if(actions.empty()) {
        return false;
    }

    Action* lastAction = actions.back();
    if (!lastAction->isUndoable())  return false;
    lastAction->undo();
    lastAction->getReceiver()->setMemento(history.getLatestMemento());//restore the saved state
    
    
    history.popLatest();  //deleted memento
    delete lastAction;  //delete the action
    actions.pop_back();  
    return true;
}