#include "Command.h"

CompleteTaskCommand::CompleteTaskCommand(WorkComponent* receiver) : receiver(receiver){}

bool CompleteTaskCommand::execute() {
    return receiver->completeTask();
}

WorkComponent* CompleteTaskCommand::getReceiver() const{
    return receiver;
}