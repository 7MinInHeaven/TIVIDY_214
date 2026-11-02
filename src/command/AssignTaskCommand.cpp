#include "Command.h"

AssignTaskCommand::AssignTaskCommand(WorkComponent* receiver, const std::string& workerId) : receiver(receiver), workerId(workerId) {}

bool AssignTaskCommand::execute() {
    return receiver->assignTask(workerId);
}

WorkComponent* AssignTaskCommand::getReceiver() const {
    return receiver;
}

