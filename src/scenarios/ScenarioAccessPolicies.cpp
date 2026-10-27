#include "ScenarioAccessPolicies.h"

//-------- Insurance --------

void configureInsuranceAccess(AccessPolicy& policy) {
    // A customer submits claims and sees only their own.
    policy.grantAction("Customer", AccessAction::Submit, AccessScope::Any);
    policy.grantAction("Customer", AccessAction::View, AccessScope::OwnRequest);
    policy.grantAction("Customer", AccessAction::ViewHistory, AccessScope::OwnRequest);
    policy.grantAction("Customer", AccessAction::Cancel, AccessScope::OwnRequest);

    // A specialist sees the queue, takes ownership, and can then only change claims they own.
    policy.grantAction("ClaimsSpecialist", AccessAction::View, AccessScope::Any);
    policy.grantAction("ClaimsSpecialist", AccessAction::ViewHistory, AccessScope::Any);
    policy.grantAction("ClaimsSpecialist", AccessAction::TakeOwnership, AccessScope::Any);
    policy.grantAction("ClaimsSpecialist", AccessAction::CompleteTask, AccessScope::AssignedRequest);
    policy.grantAction("ClaimsSpecialist", AccessAction::SaveProgress, AccessScope::AssignedRequest);
    policy.grantAction("ClaimsSpecialist", AccessAction::Undo, AccessScope::AssignedRequest);
    policy.grantAction("ClaimsSpecialist", AccessAction::Cancel, AccessScope::AssignedRequest);

    // Approvers sign off. WHICH approver handles which amount is the approval chain's job, not the proxy's.
    policy.grantAction("TeamLead", AccessAction::View, AccessScope::Any);
    policy.grantAction("TeamLead", AccessAction::ViewHistory, AccessScope::Any);
    policy.grantAction("TeamLead", AccessAction::Approve, AccessScope::Any);
    policy.grantAction("TeamLead", AccessAction::Reject, AccessScope::Any);

    policy.grantAction("SeniorManager", AccessAction::View, AccessScope::Any);
    policy.grantAction("SeniorManager", AccessAction::ViewHistory, AccessScope::Any);
    policy.grantAction("SeniorManager", AccessAction::Approve, AccessScope::Any);
    policy.grantAction("SeniorManager", AccessAction::Reject, AccessScope::Any);
}

//-------- Service requests --------

void configureServiceRequestAccess(AccessPolicy& policy) {
    policy.grantAction("Citizen", AccessAction::Submit, AccessScope::Any);
    policy.grantAction("Citizen", AccessAction::View, AccessScope::OwnRequest);
    policy.grantAction("Citizen", AccessAction::ViewHistory, AccessScope::OwnRequest);
    policy.grantAction("Citizen", AccessAction::Cancel, AccessScope::OwnRequest);

    policy.grantAction("Dispatcher", AccessAction::View, AccessScope::Any);
    policy.grantAction("Dispatcher", AccessAction::ViewHistory, AccessScope::Any);
    policy.grantAction("Dispatcher", AccessAction::Cancel, AccessScope::Any);

    policy.grantAction("Technician", AccessAction::View, AccessScope::Any);
    policy.grantAction("Technician", AccessAction::TakeOwnership, AccessScope::Any);
    policy.grantAction("Technician", AccessAction::CompleteTask, AccessScope::AssignedRequest);
    policy.grantAction("Technician", AccessAction::SaveProgress, AccessScope::AssignedRequest);
    policy.grantAction("Technician", AccessAction::Undo, AccessScope::AssignedRequest);

    policy.grantAction("Supervisor", AccessAction::View, AccessScope::Any);
    policy.grantAction("Supervisor", AccessAction::ViewHistory, AccessScope::Any);
    policy.grantAction("Supervisor", AccessAction::Approve, AccessScope::Any);
    policy.grantAction("Supervisor", AccessAction::Reject, AccessScope::Any);
    policy.grantAction("Supervisor", AccessAction::Cancel, AccessScope::Any);
}
