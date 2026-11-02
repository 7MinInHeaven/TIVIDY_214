#include "Command.h"

StartTaskCommand::StartTaskCommand(WorkComponent* receiver) : receiver(receiver) {}

bool StartTaskCommand::execute() {
    return receiver->startTask();
}

WorkComponent* StartTaskCommand::getReceiver() const{
    return receiver;
}