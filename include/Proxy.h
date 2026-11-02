#ifndef PROXY_H
#define PROXY_H

// Proxy pattern (protection proxy): check WHO is asking before forwarding to the engine.
//
// GoF roles:  Subject      = WorkflowService
//             RealSubject  = WorkflowEngineFacade   (src/facade)
//             Proxy        = AccessControlProxy
// Helpers:    AccessPolicy, AccessAction, AccessScope, AccessDeniedError

#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "Common.h"
#include "WorkflowEngineFacade.h"

//==================== What can be done, and on which requests ====================

/**
 * @brief Things a participant can try to do through the engine.
 *
 * Named AccessAction (not Action) because Action is already the Command pattern's base class.
 */
enum class AccessAction {
    Submit,         ///< Start a new request
    View,           ///< See a request's status
    ViewHistory,    ///< See a request's history
    TakeOwnership,  ///< Claim a request to work on it (locks it)
    CompleteTask,   ///< Complete a task inside a request
    Approve,        ///< Sign off a request
    Reject,         ///< Reject a request
    Cancel,         ///< Cancel a request
    Undo,           ///< Undo the last action
    SaveProgress    ///< Save work in progress
};

/** @brief Which requests a granted action applies to. */
enum class AccessScope {
    Any,             ///< Every request
    OwnRequest,      ///< Only requests this participant submitted
    AssignedRequest  ///< Only requests this participant has taken ownership of
};

std::string accessActionToString(AccessAction action);
std::string accessScopeToString(AccessScope scope);

//==================== AccessPolicy ====================

class AccessPolicy {
public:
    static const int NoInstance = -1;

    void grantAction(const std::string& roleName, AccessAction action, AccessScope scope);

    bool isAllowed(const Participant& participant, AccessAction action, int instanceId) const;

    void registerRequester(int instanceId, int participantId);
    int getRequesterId(int instanceId) const;

    bool tryAssign(int instanceId, int participantId);
    void releaseAssignment(int instanceId, int participantId);
    int getAssigneeId(int instanceId) const;

private:
    bool satisfiesScope(AccessScope scope, int participantId, int instanceId) const;

    std::map<std::string, std::map<AccessAction, AccessScope> > allowedActionsByRole;
    std::map<int, int> requesterByInstance;
    std::map<int, int> assigneeByInstance;
};

// //==================== WorkflowService (Subject) ====================

// // TEMPORARY HOME. WorkflowService belongs in the Facade's header (src/facade/Facade.h), because
// // the Proxy depends on the Facade's interface and never the other way round. Once Facade.h
// // declares it (inside the same TIVIDY_WORKFLOWSERVICE_DEFINED guard), delete this block.
// #ifndef TIVIDY_WORKFLOWSERVICE_DEFINED
// #define TIVIDY_WORKFLOWSERVICE_DEFINED

// /**
//  * @brief The operations the UI may perform on the engine.
//  *
//  * Implemented by WorkflowEngineFacade (the real engine) and by AccessControlProxy. Every call
//  * carries the id of the participant performing it (actorId) so the engine can record who did
//  * what; the proxy also checks it matches the signed-in participant.
//  */
// class WorkflowService {
// public:
//     virtual ~WorkflowService() {}

//     /** @return the id of the newly created workflow instance. */
//     virtual int submitRequest(int actorId, const std::map<std::string, std::string>& payload) = 0;
//     virtual void takeOwnership(int actorId, int instanceId) = 0;
//     virtual void completeTask(int actorId, int instanceId, int taskId) = 0;
//     virtual void approveRequest(int actorId, int instanceId) = 0;
//     virtual void rejectRequest(int actorId, int instanceId, const std::string& reason) = 0;
//     virtual void cancelRequest(int actorId, int instanceId) = 0;
//     virtual void undoLastAction(int actorId, int instanceId) = 0;
//     virtual void saveProgress(int actorId, int instanceId) = 0;

//     virtual std::string getStatus(int actorId, int instanceId) const = 0;
//     virtual std::vector<std::string> getHistory(int actorId, int instanceId) const = 0;
//     /** @return ids of existing instances; a proxy filters this to what the actor may see. */
//     virtual std::vector<int> listInstanceIds(int actorId) const = 0;
// };

// #endif // TIVIDY_WORKFLOWSERVICE_DEFINED

class AccessDeniedError : public std::runtime_error {
public:
    explicit AccessDeniedError(const std::string& message);
};

class AccessControlProxy : public WorkflowService {
public:
   
    AccessControlProxy(WorkflowService* realService, AccessPolicy* accessPolicy,
                       const Participant& currentParticipant, AuditLog* auditLog = nullptr);

    int submitRequest(int actorId, const std::map<std::string, std::string>& payload) override;
    void takeOwnership(int actorId, int instanceId) override;
    void completeTask(int actorId, int instanceId, int taskId) override;
    void approveRequest(int actorId, int instanceId) override;
    void rejectRequest(int actorId, int instanceId, const std::string& reason) override;
    void cancelRequest(int actorId, int instanceId) override;
    void undoLastAction(int actorId, int instanceId) override;
    void saveProgress(int actorId, int instanceId) override;

    std::string getStatus(int actorId, int instanceId) const override;
    std::vector<std::string> getHistory(int actorId, int instanceId) const override;
    std::vector<int> listInstanceIds(int actorId) const override;

    const Participant& getCurrentParticipant() const;

private:
    void checkAccess(int actorId, AccessAction action, int instanceId) const;
    [[noreturn]] void deny(AccessAction action, int instanceId, const std::string& reason) const;

    WorkflowService* realService;
    AccessPolicy* accessPolicy;
    Participant currentParticipant;
    AuditLog* auditLog;
};

#endif // PROXY_H
