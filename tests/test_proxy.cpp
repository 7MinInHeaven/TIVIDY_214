#include <gtest/gtest.h>

#include <map>
#include <string>
#include <vector>

#include "Proxy.h"
#include "Common.h"
#include "FakeWorkflowService.h"
#include "ScenarioAccessPolicies.h"

using namespace std;

namespace {

map<string, string> sampleClaim() {
    map<string, string> payload;
    payload["policy_number"] = "POL-99182";
    payload["amount"] = "300000";
    return payload;
}

// Bundles the shared pieces used by every test.
struct Fixture {
    FakeWorkflowService engine;
    AccessPolicy policy;
    AuditLog log;
    Participant bob;
    Participant carol;
    Participant sipho;
    Participant thandi;
    Participant lena;

    Fixture()
        : bob(1, "Bob", "Customer"),
          carol(2, "Carol", "Customer"),
          sipho(10, "Sipho", "ClaimsSpecialist"),
          thandi(11, "Thandi", "ClaimsSpecialist"),
          lena(20, "Lena", "TeamLead") {
        configureInsuranceAccess(policy);
    }

    AccessControlProxy proxyFor(const Participant& who) {
        return AccessControlProxy(&engine, &policy, who, &log);
    }
};

} // namespace

//-------- Submitting and seeing only your own --------

TEST(ProxyTest, ACustomerCanSubmitAndThenSeesTheirOwnRequest) {
    Fixture f;
    AccessControlProxy bob = f.proxyFor(f.bob);

    int id = bob.submitRequest(1, sampleClaim());
    EXPECT_TRUE(bob.getStatus(1, id) == "Pending");
    EXPECT_TRUE(bob.getHistory(1, id).size() == 1);
    EXPECT_TRUE(f.engine.countCalls("submitRequest") == 1);
}

TEST(ProxyTest, ACustomerCannotSeeAnotherCustomerSRequestAndTheDenialIsLogged) {
    Fixture f;
    AccessControlProxy bob = f.proxyFor(f.bob);
    AccessControlProxy carol = f.proxyFor(f.carol);
    int bobsClaim = bob.submitRequest(1, sampleClaim());

    EXPECT_THROW(carol.getStatus(2, bobsClaim), AccessDeniedError);
    EXPECT_THROW(carol.getHistory(2, bobsClaim), AccessDeniedError);
    EXPECT_TRUE(f.log.containsText("Access denied: Carol"));
}

TEST(ProxyTest, ListInstanceIdsShowsACustomerOnlyTheirOwnRequestsButASpecialistAll) {
    Fixture f;
    AccessControlProxy bob = f.proxyFor(f.bob);
    AccessControlProxy carol = f.proxyFor(f.carol);
    AccessControlProxy sipho = f.proxyFor(f.sipho);
    int bobsClaim = bob.submitRequest(1, sampleClaim());
    int carolsClaim = carol.submitRequest(2, sampleClaim());

    vector<int> bobSees = bob.listInstanceIds(1);
    ASSERT_TRUE(bobSees.size() == 1);
    EXPECT_TRUE(bobSees[0] == bobsClaim);

    EXPECT_TRUE(sipho.listInstanceIds(10).size() == 2);
    EXPECT_TRUE(carolsClaim != bobsClaim);
}

// TEST(ProxyTest, ListTasksFollowsTheViewPermission) {
//     Fixture f;
//     AccessControlProxy bob = f.proxyFor(f.bob);
//     AccessControlProxy carol = f.proxyFor(f.carol);
//     int id = bob.submitRequest(1, sampleClaim());

//     EXPECT_TRUE(bob.listTasks(1, id).empty());                              // allowed, fake engine has no tasks
//     EXPECT_THROW(carol.listTasks(2, id), AccessDeniedError);       // someone else's request
// }

//-------- Role checks --------

TEST(ProxyTest, RolesAreLimitedToWhatTheyWereGranted) {
    Fixture f;
    AccessControlProxy bob = f.proxyFor(f.bob);
    AccessControlProxy sipho = f.proxyFor(f.sipho);
    AccessControlProxy lena = f.proxyFor(f.lena);
    int id = bob.submitRequest(1, sampleClaim());

    EXPECT_THROW(bob.approveRequest(1, id), AccessDeniedError);       // customer cannot approve
    EXPECT_THROW(sipho.approveRequest(10, id), AccessDeniedError);    // specialist cannot approve
    EXPECT_THROW(lena.submitRequest(20, sampleClaim()), AccessDeniedError);  // team lead cannot submit
    lena.approveRequest(20, id);                                          // team lead can
    EXPECT_TRUE(f.engine.countCalls("approveRequest") == 1);
}

TEST(ProxyTest, AnUnknownRoleIsDeniedEverything) {
    Fixture f;
    Participant stranger(99, "Eve", "Hacker");
    AccessControlProxy eve = f.proxyFor(stranger);
    EXPECT_THROW(eve.submitRequest(99, sampleClaim()), AccessDeniedError);
    EXPECT_TRUE(f.engine.calls.empty());
}

TEST(ProxyTest, AUserCannotActAsSomeoneElse) {
    Fixture f;
    AccessControlProxy bob = f.proxyFor(f.bob);
    EXPECT_THROW(bob.submitRequest(2, sampleClaim()), AccessDeniedError);  // claims to be Carol
    EXPECT_THROW(bob.listInstanceIds(2), AccessDeniedError);
    EXPECT_TRUE(f.engine.calls.empty());
}

//-------- Ownership lock (no overwriting a claim another agent holds) --------

TEST(ProxyTest, ASecondAgentCannotTakeOrChangeAClaimAnotherAgentOwns) {
    Fixture f;
    AccessControlProxy bob = f.proxyFor(f.bob);
    AccessControlProxy sipho = f.proxyFor(f.sipho);
    AccessControlProxy thandi = f.proxyFor(f.thandi);
    int id = bob.submitRequest(1, sampleClaim());

    sipho.takeOwnership(10, id);
    EXPECT_TRUE(f.policy.getAssigneeId(id) == 10);

    EXPECT_THROW(thandi.takeOwnership(11, id), AccessDeniedError);
    EXPECT_THROW(thandi.completeTask(11, id, 1), AccessDeniedError);  // not the owner
    EXPECT_TRUE(f.engine.countCalls("takeOwnership") == 1);  // the refused attempt never reached the engine
    EXPECT_TRUE(f.engine.countCalls("completeTask") == 0);

    sipho.completeTask(10, id, 1);                       // the owner may work on it
    EXPECT_TRUE(f.engine.countCalls("completeTask") == 1);
}

TEST(ProxyTest, AnAgentCannotWorkOnAClaimBeforeTakingOwnership) {
    Fixture f;
    AccessControlProxy bob = f.proxyFor(f.bob);
    AccessControlProxy sipho = f.proxyFor(f.sipho);
    int id = bob.submitRequest(1, sampleClaim());
    EXPECT_THROW(sipho.completeTask(10, id, 1), AccessDeniedError);
}

TEST(ProxyTest, TakingOwnershipTwiceIsHarmlessAndTheLockCanBeReleased) {
    Fixture f;
    AccessControlProxy bob = f.proxyFor(f.bob);
    AccessControlProxy sipho = f.proxyFor(f.sipho);
    AccessControlProxy thandi = f.proxyFor(f.thandi);
    int id = bob.submitRequest(1, sampleClaim());

    sipho.takeOwnership(10, id);
    sipho.takeOwnership(10, id);
    f.policy.releaseAssignment(id, 11);          // someone else cannot release it
    EXPECT_TRUE(f.policy.getAssigneeId(id) == 10);
    f.policy.releaseAssignment(id, 10);
    EXPECT_TRUE(f.policy.getAssigneeId(id) == -1);
    thandi.takeOwnership(11, id);                // now free for another agent
    EXPECT_TRUE(f.policy.getAssigneeId(id) == 11);
}

TEST(ProxyTest, IfTheEngineRefusesTakeOwnershipTheLockIsNotKept) {
    Fixture f;
    AccessControlProxy bob = f.proxyFor(f.bob);
    AccessControlProxy sipho = f.proxyFor(f.sipho);
    int id = bob.submitRequest(1, sampleClaim());

    f.engine.failOnTakeOwnership = true;
    EXPECT_ANY_THROW(sipho.takeOwnership(10, id));
    EXPECT_TRUE(f.policy.getAssigneeId(id) == -1);
}

//-------- Policy shared across proxies, scenario reuse --------

TEST(ProxyTest, TwoProxiesShareOnePolicySoALockTakenOnOneIsSeenByTheOther) {
    Fixture f;
    AccessControlProxy bob = f.proxyFor(f.bob);
    int id = bob.submitRequest(1, sampleClaim());
    AccessControlProxy first = f.proxyFor(f.sipho);
    AccessControlProxy second = f.proxyFor(f.thandi);

    first.takeOwnership(10, id);
    EXPECT_THROW(second.takeOwnership(11, id), AccessDeniedError);
}

TEST(ProxyTest, TheSameAccessPolicyClassServesASecondIndustryWithoutModification) {
    FakeWorkflowService engine;
    AccessPolicy policy;
    configureServiceRequestAccess(policy);
    Participant citizen(1, "Nomsa", "Citizen");
    Participant technician(2, "Pieter", "Technician");
    Participant supervisor(3, "Zanele", "Supervisor");

    AccessControlProxy citizenProxy(&engine, &policy, citizen);
    AccessControlProxy techProxy(&engine, &policy, technician);
    AccessControlProxy supervisorProxy(&engine, &policy, supervisor);

    int ticket = citizenProxy.submitRequest(1, map<string, string>());
    EXPECT_TRUE(citizenProxy.getStatus(1, ticket) == "Pending");
    techProxy.takeOwnership(2, ticket);
    techProxy.completeTask(2, ticket, 1);
    EXPECT_THROW(citizenProxy.completeTask(1, ticket, 1), AccessDeniedError);
    supervisorProxy.approveRequest(3, ticket);
    EXPECT_THROW(techProxy.approveRequest(2, ticket), AccessDeniedError);
}

TEST(ProxyTest, AProxyNeedsSomethingToForwardToAndAPolicy) {
    FakeWorkflowService engine;
    AccessPolicy policy;
    Participant someone(1, "A", "Customer");
    EXPECT_THROW(AccessControlProxy(nullptr, &policy, someone), invalid_argument);
    EXPECT_THROW(AccessControlProxy(&engine, nullptr, someone), invalid_argument);
}

TEST(ProxyTest, AccessTypeNamesAreReadable) {
    EXPECT_TRUE(accessActionToString(AccessAction::TakeOwnership) == "TakeOwnership");
    EXPECT_TRUE(accessScopeToString(AccessScope::AssignedRequest) == "AssignedRequest");
}

//-------- Extra tests: every forwarded operation, and unusual cases --------

TEST(ProxyTest, EveryForwardedOperationReachesTheEngineWhenAllowed) {
    Fixture f;
    AccessControlProxy bob = f.proxyFor(f.bob);
    AccessControlProxy sipho = f.proxyFor(f.sipho);
    AccessControlProxy lena = f.proxyFor(f.lena);
    int id = bob.submitRequest(1, sampleClaim());

    sipho.takeOwnership(10, id);
    sipho.saveProgress(10, id);
    sipho.undoLastAction(10, id);
    lena.rejectRequest(20, id, "incomplete");
    bob.cancelRequest(1, id);

    EXPECT_EQ(f.engine.countCalls("saveProgress"), 1);
    EXPECT_EQ(f.engine.countCalls("undoLastAction"), 1);
    EXPECT_EQ(f.engine.countCalls("rejectRequest"), 1);
    EXPECT_EQ(f.engine.countCalls("cancelRequest"), 1);
    EXPECT_EQ(sipho.getCurrentParticipant().getName(), "Sipho");
}

TEST(ProxyTest, OnlyTheSubmitterMayCancelACustomerRequest) {
    Fixture f;
    AccessControlProxy bob = f.proxyFor(f.bob);
    AccessControlProxy carol = f.proxyFor(f.carol);
    int id = bob.submitRequest(1, sampleClaim());
    EXPECT_THROW(carol.cancelRequest(2, id), AccessDeniedError);
    EXPECT_THROW(carol.undoLastAction(2, id), AccessDeniedError);
    EXPECT_THROW(carol.saveProgress(2, id), AccessDeniedError);
    EXPECT_THROW(carol.rejectRequest(2, id, "no"), AccessDeniedError);
}

TEST(ProxyTest, DenialsWorkWithoutAnAuditLog) {
    FakeWorkflowService engine;
    AccessPolicy policy;
    configureInsuranceAccess(policy);
    Participant bob(1, "Bob", "Customer");
    AccessControlProxy proxy(&engine, &policy, bob);  // no log supplied
    EXPECT_THROW(proxy.approveRequest(1, 5), AccessDeniedError);
}

TEST(ProxyTest, UnknownEnumValuesHaveAName) {
    EXPECT_EQ(accessActionToString(static_cast<AccessAction>(99)), "Unknown");
    EXPECT_EQ(accessScopeToString(static_cast<AccessScope>(99)), "Unknown");
}
