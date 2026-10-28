#include "Decorator.h"

#include <exception>

using namespace std;

namespace {

string statusOf(WorkComponent& task) {
    WorkState* state = task.getState();
    return state != nullptr ? state->getStatus() : string("Unknown");
}

} // basically namespace

//-------- Construction ------------------------------

AuditedTask::AuditedTask(WorkComponent* taskToWrap, AuditLog* log)
    : TaskDecorator(taskToWrap), auditLog(log) {
    if (log == nullptr) {
        throw invalid_argument("AuditedTask needs an AuditLog");
    }
}

//------------------extrabehaviour ----------------------------
void AuditedTask::nextState() {
    string before = statusOf(inner());
    try {
        TaskDecorator::nextState();
    } catch (const exception& error) {
        auditLog->record("[" + getName() + "] " + before + " -> FAILED: " + error.what());
        throw;
    } catch (...) {
        auditLog->record("[" + getName() + "] " + before + " -> FAILED: unknown error");
        throw;
    }
    auditLog->record("[" + getName() + "] " + before + " -> " + statusOf(inner()));
}

//-------- prototype --------

WorkComponent* AuditedTask::clone() const {
    return new AuditedTask(getWrappedTask()->clone(), auditLog);
}
