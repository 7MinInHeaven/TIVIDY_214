#include "Memento.h"
#include "WorkState.h"

WorkMemento::WorkMemento(const std::string &taskId, WorkState *currentState,
                         const std::string &assignedWorkerId)
    : taskId(taskId), currentState(currentState->clone()),
      assignedWorkerId(assignedWorkerId) {}

WorkMemento::~WorkMemento() { delete currentState; }
