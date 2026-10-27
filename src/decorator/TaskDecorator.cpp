#include "Decorator.h"
#include <stdexcept>

using namespace std;

//-------- Construction --------

// The WorkComponent base needs a name, so it is taken from the wrapped task. nameOf() also rejects a null task before anything is dereferenced.
string TaskDecorator::nameOf(const WorkComponent* task) {
    if (task == nullptr) {
        throw invalid_argument("TaskDecorator needs a task to wrap");
    }
    return task->getName();
}

TaskDecorator::TaskDecorator(WorkComponent* taskToWrap)
    : WorkComponent(nameOf(taskToWrap)), wrappedTask(taskToWrap) {}

TaskDecorator::~TaskDecorator() {}

//-------- Forwarded operations --------

WorkState* TaskDecorator::getState() const { return wrappedTask->getState(); }

string TaskDecorator::getName() const { return wrappedTask->getName(); }

void TaskDecorator::setData(const string& key, const string& value) { wrappedTask->setData(key, value); }

string TaskDecorator::getData(const string& key) const { return wrappedTask->getData(key); }

// lowkey after the wrapped task changes state, tell THIS decorator's observers too for outer
void TaskDecorator::nextState() {
    if (wrappedTask->isComplete()) {
        throw std::logic_error("Cannot advance a completed task");
    }
    
    wrappedTask->nextState();
    notify();
}

bool TaskDecorator::isComplete() const { return wrappedTask->isComplete(); }

PriorityLevel TaskDecorator::getPriority() const { return wrappedTask->getPriority(); }

void TaskDecorator::accept(WorkVisitor& visitor) { wrappedTask->accept(visitor); }

//acces for subclasses
WorkComponent* TaskDecorator::getWrappedTask() const { return wrappedTask.get(); }
WorkComponent& TaskDecorator::inner() const { return *wrappedTask; }
