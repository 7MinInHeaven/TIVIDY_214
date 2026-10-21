#include "Command.h"

AssignTaskCommand::AssignTaskCommand(WorkUnit* receiver, const std::string& workerId) : receiver(receiver), workerId(workerId) {}

bool AssignTaskCommand::execute() {
    return receiver->assignTask(workerId);
}

WorkUnit* AssignTaskCommand::getReceiver() const {
    return receiver;
}

