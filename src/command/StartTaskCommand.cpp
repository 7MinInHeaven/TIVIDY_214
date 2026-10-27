#include "Command.h"

StartTaskCommand::StartTaskCommand(WorkUnit* receiver) : receiver(receiver) {}

bool StartTaskCommand::execute() {
    return receiver->startTask();
}

WorkUnit* StartTaskCommand::getReceiver() const{
    return receiver;
}