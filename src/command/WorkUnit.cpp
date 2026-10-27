#include "Command.h"
#include "Memento.h"

WorkUnit::WorkUnit(const std::string &taskId)
    : WorkComponent(taskId), taskId(taskId), currentState(nullptr), assignedWorkerId("") {}

bool WorkUnit::startTask() {
  if (assignedWorkerId.empty())
    return false;
  if (currentState) {
    currentState->handleUpdate(this);
  }
  return true;
}

bool WorkUnit::completeTask() {
  if (currentState) {
    currentState->handleUpdate(this);
  }
  return true;
}

bool WorkUnit::assignTask(const std::string &workerId) {
  assignedWorkerId = workerId;
  return true;
}

WorkMemento* WorkUnit::createMemento() {
  return new WorkMemento(taskId, currentState, assignedWorkerId);
}

void WorkUnit::setMemento(WorkMemento* memento) {
  if (!memento)
    return;
  taskId = memento->taskId;
  currentState = memento->currentState;
  assignedWorkerId = memento->assignedWorkerId;
}

WorkState* WorkUnit::getState() { 
  return currentState; 
}

std::string WorkUnit::getAssignedWorkerId() { 
  return assignedWorkerId; 
}

std::string WorkUnit::getTaskId() { 
  return taskId; 
}
