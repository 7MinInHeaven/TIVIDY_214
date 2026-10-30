#include "Composite.h"
#include "Command.h"
#include "Memento.h"

WorkUnit::WorkUnit(const std::string& name) : WorkComponent(name){}

WorkMemento* WorkUnit::createMemento() 
{
  return new WorkMemento(taskID, state, assignedWorkerID);
}

void WorkUnit::setMemento(WorkMemento* memento) 
{
  if (!memento) return;
  taskID = memento->taskId;
  assignedWorkerID = memento->assignedWorkerId;

  if (memento->currentState != nullptr) 
  {
    setState(memento->currentState->clone());  
  }
}

void WorkUnit::accept(WorkVisitor& visitor) const{
    visitor.visit(*this);
}

WorkUnit* WorkUnit::clone() const {
    return new WorkUnit(*this);
}
