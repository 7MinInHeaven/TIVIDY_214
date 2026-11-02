#include "Proxy.h"

#include <sstream>

using namespace std;


AccessDeniedError::AccessDeniedError(const string& message) : runtime_error(message) {}

//-----------------------Construction------------------

AccessControlProxy::AccessControlProxy(WorkflowService* service, AccessPolicy* policy,
                                       const Participant& participant, AuditLog* log)
    : realService(service), accessPolicy(policy), currentParticipant(participant), auditLog(log) {
    if (service == nullptr) {
        throw invalid_argument("AccessControlProxy needs a real service to forward to");
    }
    if (policy == nullptr) {
        throw invalid_argument("AccessControlProxy needs an AccessPolicy");
    }
}

const Participant& AccessControlProxy::getCurrentParticipant() const { return currentParticipant; }

//---------------------Checking--------------

void AccessControlProxy::deny(AccessAction action, int instanceId, const string& reason) const {
    ostringstream message;
    message << "Access denied: " << currentParticipant.getName() << " (" << currentParticipant.getRoleName()
            << ") cannot " << accessActionToString(action);
    if (instanceId != AccessPolicy::NoInstance) {
        message << " on request " << instanceId;
    }
    message << " - " << reason;
    if (auditLog != nullptr) {
        auditLog->record(message.str());
    }
    throw AccessDeniedError(message.str());
}

void AccessControlProxy::checkAccess(int actorId, AccessAction action, int instanceId) const {
    if (actorId != currentParticipant.getId()) {
        deny(action, instanceId, "the actor is not the signed-in participant");
    }
    if (!accessPolicy->isAllowed(currentParticipant, action, instanceId)) {
        deny(action, instanceId, "not permitted for this role or request");
    }
}

//-------------------------Forwarded operations --------

int AccessControlProxy::submitRequest(int actorId, const map<string, string>& payload) {
    checkAccess(actorId, AccessAction::Submit, AccessPolicy::NoInstance);
    int instanceId = realService->submitRequest(actorId, payload);
    accessPolicy->registerRequester(instanceId, actorId); // so the submitter can later see their own request
    return instanceId;
}

void AccessControlProxy::takeOwnership(int actorId, int instanceId) {
    checkAccess(actorId, AccessAction::TakeOwnership, instanceId);

    bool alreadyHeldByActor = (accessPolicy->getAssigneeId(instanceId) == actorId);
    if (!accessPolicy->tryAssign(instanceId, actorId)) {
        deny(AccessAction::TakeOwnership, instanceId, "it is already being handled by another agent");
    }
    try {
        realService->takeOwnership(actorId, instanceId);
    } catch (...) {
        if (!alreadyHeldByActor) {
            accessPolicy->releaseAssignment(instanceId, actorId); // never keep a lock the engine did not grant
        }
        throw;
    }
}

void AccessControlProxy::completeTask(int actorId, int instanceId, int taskId) {
    checkAccess(actorId, AccessAction::CompleteTask, instanceId);
    realService->completeTask(actorId, instanceId, taskId);
}

void AccessControlProxy::approveRequest(int actorId, int instanceId) {
    checkAccess(actorId, AccessAction::Approve, instanceId);
    realService->approveRequest(actorId, instanceId);
}

void AccessControlProxy::rejectRequest(int actorId, int instanceId, const string& reason) {
    checkAccess(actorId, AccessAction::Reject, instanceId);
    realService->rejectRequest(actorId, instanceId, reason);
}

void AccessControlProxy::cancelRequest(int actorId, int instanceId) {
    checkAccess(actorId, AccessAction::Cancel, instanceId);
    realService->cancelRequest(actorId, instanceId);
}

void AccessControlProxy::undoLastAction(int actorId, int instanceId) {
    checkAccess(actorId, AccessAction::Undo, instanceId);
    realService->undoLastAction(actorId, instanceId);
}

void AccessControlProxy::saveProgress(int actorId, int instanceId) {
    checkAccess(actorId, AccessAction::SaveProgress, instanceId);
    realService->saveProgress(actorId, instanceId);
}

string AccessControlProxy::getStatus(int actorId, int instanceId) const {
    checkAccess(actorId, AccessAction::View, instanceId);
    return realService->getStatus(actorId, instanceId);
}

vector<string> AccessControlProxy::getHistory(int actorId, int instanceId) const {
    checkAccess(actorId, AccessAction::ViewHistory, instanceId);
    return realService->getHistory(actorId, instanceId);
}

vector<int> AccessControlProxy::listInstanceIds(int actorId) const {
    if (actorId != currentParticipant.getId()) {
        deny(AccessAction::View, AccessPolicy::NoInstance, "the actor is not the signed-in participant");
    }
    vector<int> everything = realService->listInstanceIds(actorId);
    vector<int> visible;
    for (size_t i = 0; i < everything.size(); ++i) {
        if (accessPolicy->isAllowed(currentParticipant, AccessAction::View, everything[i])) {
            visible.push_back(everything[i]);
        }
    }
    return visible;
}
