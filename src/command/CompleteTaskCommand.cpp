#include "Command.h"

CompleteTaskCommand::CompleteTaskCommand(WorkUnit* receiver) : receiver(receiver){}

bool CompleteTaskCommand::execute() {
    return receiver->completeTask();
}

WorkUnit* CompleteTaskCommand::getReceiver() const{
    return receiver;
}