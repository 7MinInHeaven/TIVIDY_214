#include "Proxy.h"

using namespace std;


void AccessPolicy::grantAction(const string& roleName, AccessAction action, AccessScope scope) {
    allowedActionsByRole[roleName][action] = scope;
}

//-------- Decisions --------

bool AccessPolicy::isAllowed(const Participant& participant, AccessAction action, int instanceId) const {
    map<string, map<AccessAction, AccessScope> >::const_iterator role =
        allowedActionsByRole.find(participant.getRoleName());
    if (role == allowedActionsByRole.end()) {
        return false; // unknown role: nothing is allowed
    }
    map<AccessAction, AccessScope>::const_iterator grant = role->second.find(action);
    if (grant == role->second.end()) {
        return false; // role exists but was never granted this action
    }
    return satisfiesScope(grant->second, participant.getId(), instanceId);
}

bool AccessPolicy::satisfiesScope(AccessScope scope, int participantId, int instanceId) const {
    if (scope == AccessScope::Any) {
        return true;
    }
    if (scope == AccessScope::OwnRequest) {
        return instanceId != NoInstance && getRequesterId(instanceId) == participantId;
    }
    return instanceId != NoInstance && getAssigneeId(instanceId) == participantId;
}

//----------------Who submitted-------------------

void AccessPolicy::registerRequester(int instanceId, int participantId) {
    requesterByInstance[instanceId] = participantId;
}

int AccessPolicy::getRequesterId(int instanceId) const {
    map<int, int>::const_iterator found = requesterByInstance.find(instanceId);
    return found == requesterByInstance.end() ? -1 : found->second;
}

//-------------Ownership lock------------------------

bool AccessPolicy::tryAssign(int instanceId, int participantId) {
    map<int, int>::const_iterator found = assigneeByInstance.find(instanceId);
    if (found != assigneeByInstance.end() && found->second != participantId) {
        return false;
    }
    assigneeByInstance[instanceId] = participantId;
    return true;
}

void AccessPolicy::releaseAssignment(int instanceId, int participantId) {
    map<int, int>::iterator found = assigneeByInstance.find(instanceId);
    if (found != assigneeByInstance.end() && found->second == participantId) {
        assigneeByInstance.erase(found);
    }
}

int AccessPolicy::getAssigneeId(int instanceId) const {
    map<int, int>::const_iterator found = assigneeByInstance.find(instanceId);
    return found == assigneeByInstance.end() ? -1 : found->second;
}
