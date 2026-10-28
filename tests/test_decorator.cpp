#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "Decorator.h"

using namespace std;

namespace {

shared_ptr<const ValidationRule> required(const string& key) {
    return shared_ptr<const ValidationRule>(new RequiredKeyRule(key));
}

// Counts how many times it was told something changed.
class CountingObserver : public WorkflowObserver {
public:
    CountingObserver() : updates(0) {}
    void update() override { ++updates; }
    int updates;
};

}

//-------- ValidatedTask: transitions and invalid operations --------

TEST(DecoratorTest, ValidatedTaskLetsATaskSTARTEvenWhenItsCompletionDataIsMissing) {
    ValidatedTask task(new WorkUnit("Draft response"), "InProgress");
    task.addRule(required("response_email"));

    task.nextState();
    EXPECT_TRUE(task.getState()->getStatus() == "In Progress");
}

TEST(DecoratorTest, ValidatedTaskBlocksCompletionWhenARequiredKeyIsEmptyAndTheStateDoesNot) {
    ValidatedTask task(new WorkUnit("Draft response"), "InProgress");
    task.addRule(required("response_email"));
    task.nextState();

    EXPECT_THROW(task.nextState(), ValidationError);
    EXPECT_TRUE(task.getState()->getStatus() == "In Progress");
    EXPECT_FALSE(task.isComplete());
}

TEST(DecoratorTest, ValidatedTaskReportsEveryFailedRuleAndNamesTheTask) {
    ValidatedTask task(new WorkUnit("Verify policy"), "InProgress");
    task.addRule(required("policy_number"));
    task.addRule(required("customer_email"));
    task.setData("policy_number", "   ");  // blank counts as missing
    task.nextState();

    try {
        task.nextState();
        FAIL() << "expected a ValidationError";
    } catch (const ValidationError& error) {
        EXPECT_TRUE(error.getFailures().size() == 2);
        EXPECT_TRUE(string(error.what()).find("Verify policy") != string::npos);
        EXPECT_TRUE(string(error.what()).find("policy_number") != string::npos);
    }
}

TEST(DecoratorTest, ValidatedTaskAllowsCompletionOnceTheDataIsValid) {
    ValidatedTask task(new WorkUnit("Draft response"), "InProgress");
    task.addRule(required("response_email"));
    task.nextState();
    task.setData("response_email", "Your claim has been approved");

    task.nextState();
    EXPECT_TRUE(task.isComplete());
}

TEST(DecoratorTest, ValidatedTaskWithoutRulesBehavesLikeThePlainTask) {
    ValidatedTask task(new WorkUnit("Open task"), "InProgress");
    task.nextState();
    task.nextState();
    EXPECT_TRUE(task.isComplete());
}

TEST(DecoratorTest, BadConstructorAndRuleArgumentsAreRejected) {
    ValidatedTask task(new WorkUnit("X"), "InProgress");
    EXPECT_THROW(task.addRule(shared_ptr<const ValidationRule>()), invalid_argument);
    EXPECT_THROW(ValidatedTask(nullptr, "InProgress"), invalid_argument);
    EXPECT_THROW(ValidatedTask(new WorkUnit("Y"), ""), invalid_argument);
}

TEST(DecoratorTest, ValidatedTaskValidatePreviewsProblemsWithoutChangingState) {
    ValidatedTask task(new WorkUnit("Check amount"), "InProgress");
    task.addRule(shared_ptr<const ValidationRule>(new NumericRangeRule("amount", 1, 1000000)));
    task.setData("amount", "abc");
    vector<string> failures;

    EXPECT_FALSE(task.validate(failures));
    EXPECT_TRUE(failures.size() == 1);
    EXPECT_TRUE(task.getState()->getStatus() == "Pending");
}

//----------------------------rules--------------------------

TEST(DecoratorTest, NumericRangeRuleAcceptsOnlyFiniteNumbersInsideTheRange) {
    NumericRangeRule rule("amount", 1, 1000000);
    WorkUnit task("Check amount");

    task.setData("amount", "300000");   EXPECT_TRUE(rule.check(task));
    task.setData("amount", " 250000 "); EXPECT_TRUE(rule.check(task));
    task.setData("amount", "1");        EXPECT_TRUE(rule.check(task));
    task.setData("amount", "0");        EXPECT_FALSE(rule.check(task));
    task.setData("amount", "-5");       EXPECT_FALSE(rule.check(task));
    task.setData("amount", "1000001");  EXPECT_FALSE(rule.check(task));
    task.setData("amount", "12abc");    EXPECT_FALSE(rule.check(task));
    task.setData("amount", "nan");      EXPECT_FALSE(rule.check(task));
    task.setData("amount", "inf");      EXPECT_FALSE(rule.check(task));
    task.setData("amount", "");         EXPECT_FALSE(rule.check(task));
    EXPECT_TRUE(rule.getMessage() == "'amount' must be a number between 1 and 1000000");
}

TEST(DecoratorTest, EmailFormatRuleRecognisesPlausibleAndImplausibleAddresses) {
    EmailFormatRule rule("customer_email");
    WorkUnit task("Check email");

    task.setData("customer_email", "bob@example.com");  EXPECT_TRUE(rule.check(task));
    task.setData("customer_email", "bob@example");      EXPECT_FALSE(rule.check(task));
    task.setData("customer_email", "@example.com");     EXPECT_FALSE(rule.check(task));
    task.setData("customer_email", "bob@@example.com"); EXPECT_FALSE(rule.check(task));
    task.setData("customer_email", "bob smith@x.com");  EXPECT_FALSE(rule.check(task));
    task.setData("customer_email", "bob@.com");         EXPECT_FALSE(rule.check(task));
    task.setData("customer_email", "bob@example.");     EXPECT_FALSE(rule.check(task));
    task.setData("customer_email", "");                 EXPECT_FALSE(rule.check(task));
}


TEST(DecoratorTest, StackedDecoratorsForwardEverythingToTheRealTask) {
    AuditLog log;
    ManualClock clock(1000);
    WorkUnit* unit = new WorkUnit("Isolate water");
    WorkComponent* stacked = new PrioritisedTask(
        new AuditedTask(new ValidatedTask(unit, "InProgress"), &log), PriorityLevel::High, 0, &clock);

    EXPECT_TRUE(stacked->getName() == "Isolate water");
    EXPECT_TRUE(stacked->getPriority() == PriorityLevel::High);   // outermost priority wins
    EXPECT_TRUE(stacked->getState() == unit->getState());
    EXPECT_FALSE(stacked->isComplete());

    stacked->setData("note", "valve closed");
    EXPECT_TRUE(unit->getData("note") == "valve closed");

    stacked->nextState();
    stacked->nextState();
    EXPECT_TRUE(unit->isComplete());
    EXPECT_TRUE(stacked->isComplete());

    delete stacked; 
}

TEST(DecoratorTest, ATaskNobodyPrioritisedReportsNormalPriorityThroughTheDecorators) {
    AuditLog log;
    AuditedTask task(new WorkUnit("Plain"), &log);
    EXPECT_TRUE(task.getPriority() == PriorityLevel::Normal);
}

TEST(DecoratorTest, AnObserverAttachedToTheOutermostWrapperHearsEveryStateChangeOnce) {
    AuditLog log;
    CountingObserver observer;
    AuditedTask task(new WorkUnit("Observed"), &log);
    task.attach(&observer);

    task.nextState();
    task.nextState();
    EXPECT_TRUE(observer.updates == 2);
}

//-------- AuditedTask --------

TEST(DecoratorTest, AuditedTaskRecordsEachTransitionWithBeforeAndAfterStatus) {
    AuditLog log;
    AuditedTask task(new WorkUnit("Approve payout"), &log);
    task.nextState();
    task.nextState();

    EXPECT_TRUE(log.size() == 2);
    EXPECT_TRUE(log.containsText("[Approve payout] Pending -> In Progress"));
    EXPECT_TRUE(log.containsText("[Approve payout] In Progress -> Completed"));
}

TEST(DecoratorTest, AuditedTaskRecordsAnInvalidTransitionAndStillRethrowsIt) {
    AuditLog log;
    AuditedTask task(new WorkUnit("Approve payout"), &log);
    task.nextState();
    task.nextState();

    // mo changed: manually catch to avoid GoogleTest's strict behavior
    bool threw = false;
    try {
        task.nextState();
    } catch (const logic_error&) {
        threw = true;
    }
    EXPECT_TRUE(threw);
    EXPECT_TRUE(log.containsText("Completed -> FAILED:"));
}

TEST(DecoratorTest, AuditedTaskRecordsAValidationFailureRaisedByATaskItWraps) {
    AuditLog log;
    ValidatedTask* validated = new ValidatedTask(new WorkUnit("Draft response"), "InProgress");
    validated->addRule(required("response_email"));
    AuditedTask task(validated, &log);
    task.nextState();

    EXPECT_THROW(task.nextState(), ValidationError);
    EXPECT_TRUE(log.containsText("In Progress -> FAILED"));
}

TEST(DecoratorTest, AuditedTaskNeedsALog) {
    EXPECT_THROW(AuditedTask(new WorkUnit("X"), nullptr), invalid_argument);
}

//-------- PrioritisedTask: SLA --------

TEST(DecoratorTest, PrioritisedTaskIsOverdueOnlyAfterItsSLADaysPassAndNotOnceComplete) {
    ManualClock clock(1000000);
    PrioritisedTask task(new WorkUnit("Assess claim"), PriorityLevel::Urgent, 7, &clock);

    EXPECT_FALSE(task.hasDeadline());  // the SLA starts at the first state change
    EXPECT_FALSE(task.isOverdue());

    task.nextState();
    EXPECT_TRUE(task.hasDeadline());
    EXPECT_TRUE(task.getSecondsRemaining() == 7 * 86400);

    clock.advanceDays(6);
    EXPECT_FALSE(task.isOverdue());
    clock.advanceDays(2);
    EXPECT_TRUE(task.isOverdue());
    EXPECT_TRUE(task.getSecondsRemaining() < 0);

    task.nextState();  // complete
    EXPECT_FALSE(task.isOverdue());
}

TEST(DecoratorTest, PrioritisedTaskPriorityCanChangeANegativeSLAIsRejectedAndNoSLAMeansNoD) {
    PrioritisedTask task(new WorkUnit("X"), PriorityLevel::Low);
    EXPECT_TRUE(task.getPriority() == PriorityLevel::Low);
    task.setPriority(PriorityLevel::Urgent);
    EXPECT_TRUE(task.getPriority() == PriorityLevel::Urgent);
    EXPECT_THROW(PrioritisedTask(new WorkUnit("Y"), PriorityLevel::Low, -1), invalid_argument);

    task.nextState();
    EXPECT_FALSE(task.hasDeadline());
    EXPECT_FALSE(task.isOverdue());
}


TEST(DecoratorTest, CloningADecoratedTaskGivesAnIndependentDeepCopy) {
    AuditLog log;
    ManualClock clock(5000);
    ValidatedTask* validated = new ValidatedTask(new WorkUnit("Draft response"), "InProgress");
    validated->addRule(required("response_email"));
    PrioritisedTask* master = new PrioritisedTask(new AuditedTask(validated, &log), PriorityLevel::High, 7, &clock);

    unique_ptr<WorkComponent> copy(master->clone());
    EXPECT_TRUE(copy->getName() == "Draft response");
    EXPECT_TRUE(copy->getPriority() == PriorityLevel::High);

    // Progress on the original does not leak into the copy.
    master->nextState();
    master->setData("response_email", "approved");
    master->nextState();
    EXPECT_TRUE(master->isComplete());
    EXPECT_TRUE(copy->getState()->getStatus() == "Pending");
    EXPECT_TRUE(copy->getData("response_email") == "");

    // The copy still enforces the (shared) rule on its own.
    copy->nextState();
    EXPECT_THROW(copy->nextState(), ValidationError);

    // Deleting the original leaves the copy fully usable.
    delete master;
    copy->setData("response_email", "approved");
    copy->nextState();
    EXPECT_TRUE(copy->isComplete());
}

TEST(DecoratorTest, ACloneGetsAFreshSLADeadlineWhenItStarts) {
    ManualClock clock(1000);
    PrioritisedTask* master = new PrioritisedTask(new WorkUnit("T"), PriorityLevel::Normal, 7, &clock);
    master->nextState();
    unique_ptr<PrioritisedTask> copy(static_cast<PrioritisedTask*>(master->clone()));
    EXPECT_FALSE(copy->hasDeadline());  // the deadline is not copied

    clock.advanceDays(3);
    copy->nextState();
    EXPECT_TRUE(copy->getDeadline() == 1000 + 3 * 86400 + 7 * 86400);
    delete master;
}

//--------------auditlog----------------------------

TEST(DecoratorTest, AuditLogNumbersEntriesAndFormatsThem) {
    ManualClock clock(0);
    AuditLog log(&clock);
    log.record("first");
    log.record("second");
    EXPECT_TRUE(log.size() == 2);
    EXPECT_TRUE(log.getEntries()[1].sequenceNumber == 2);
    EXPECT_TRUE(AuditLog::formatEntry(log.getEntries()[0]).find("#1") == 0);
    EXPECT_TRUE(AuditLog::formatEntry(log.getEntries()[0]).find("first") != string::npos);
}

//-------- lowkey extra-----------------------------------------------------

namespace {

class CountingVisitor : public WorkVisitor {
public:
    CountingVisitor() : units(0) {}
    void visit(WorkUnit&) override { ++units; }
    void visit(Stage&) override {}
    void visit(SubProcess&) override {}
    int units;
};

class RudeUnit : public WorkUnit {
public:
    explicit RudeUnit(const string& name) : WorkUnit(name) {}
    void nextState() override { throw 42; }
};

// getData always throws
class SilentUnit : public WorkUnit {
public:
    explicit SilentUnit(const string& name) : WorkUnit(name) {}
    string getData(const string&) const override { throw runtime_error("no such key"); }
};

}

TEST(DecoratorTest, AcceptReachesTheRealTaskThroughEveryWrapper) {
    AuditLog log;
    AuditedTask task(new ValidatedTask(new WorkUnit("Isolate water"), "InProgress"), &log);
    CountingVisitor visitor;
    task.accept(visitor);
    EXPECT_EQ(visitor.units, 1);
}

TEST(DecoratorTest, DecoratorGettersReportTheirConfiguration) {
    ValidatedTask validated(new WorkUnit("V"), "InProgress");
    validated.addRule(required("a"));
    EXPECT_EQ(validated.getGuardedStatus(), "InProgress");
    EXPECT_EQ(validated.getRuleCount(), 1u);
    EXPECT_EQ(validated.getWrappedTask()->getName(), "V");

    ManualClock clock(10);
    PrioritisedTask task(new WorkUnit("P"), PriorityLevel::Low, 3, &clock);
    EXPECT_EQ(task.getSlaDays(), 3);
    EXPECT_EQ(task.getDeadline(), 0);
    EXPECT_EQ(task.getSecondsRemaining(), 0);  // no deadline until the task starts
}

TEST(DecoratorTest, AuditedTaskRecordsAFailureThatIsNotAStdException) {
    AuditLog log;
    AuditedTask task(new RudeUnit("Rude"), &log);
    EXPECT_THROW(task.nextState(), int);
    EXPECT_TRUE(log.containsText("unknown error"));
}

TEST(DecoratorTest, RuleTreatsAnUnreadableKeyAsMissing) {
    SilentUnit task("Silent");
    RequiredKeyRule rule("anything");
    EXPECT_FALSE(rule.check(task));
}

TEST(DecoratorTest, RuleMessagesAreReadable) {
    EXPECT_EQ(RequiredKeyRule("k").getMessage(), "\x27k\x27 is required and must not be empty");
    EXPECT_EQ(NumericRangeRule("k", 0.5, 2.5).getMessage(), "\x27k\x27 must be a number between 0.5 and 2.5");
    EXPECT_EQ(EmailFormatRule("k").getMessage(), "\x27k\x27 must be a valid email address");
}
