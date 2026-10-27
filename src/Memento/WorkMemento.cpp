#include "../include/Memento.h"
#include "../include/WorkState.h"

WorkMemento::WorkMemento(const std::string& taskId, WorkState* currentState, const std::string& assignedWorkerId) : taskId(taskId), currentState(currentState), assignedWorkerId(assignedWorkerId) {}

WorkMemento::~WorkMemento() {
    delete currentState;
}

