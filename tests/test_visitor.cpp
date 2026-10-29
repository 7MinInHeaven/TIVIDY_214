#include <gtest/gtest.h>
#include "WorkVisitor.h"
#include "Composite.h"
#include "Decorator.h"

TEST(VisitorTest, CreateVisitorButDontGenerateReport){
    ProgressReportVisitor visitor;
}

TEST(VisitorTest, CreateReportBasicReport){
    WorkGroup group("Test Group");
    Stage* stageProcesses = new Stage("Test Stage One");
    Stage* stageNoProcess = new Stage("Test Stage Two");
    
    SubProcess* processUnits = new SubProcess("Test Process One");
    SubProcess* processNoUnit = new SubProcess("Test Process Two");

    WorkUnit* unit1 = new WorkUnit("Test Unit One");
    WorkUnit* unit2 = new WorkUnit("Test Unit Two");

    processUnits->add(unit1);
    processUnits->add(unit2);

    stageProcesses->add(processUnits);
    stageProcesses->add(processNoUnit);

    group.add(stageProcesses);
    group.add(stageNoProcess);

    ProgressReportVisitor visitor;
    visitor.createReport(&group);
}

TEST(VisitorTest, AllPriorityPrioritisedTask){
    ProgressReportVisitor visitor;

    SystemClock* clock = new SystemClock();
    WorkComponent* lowPrioritisedUnit = new PrioritisedTask(new WorkUnit("Test Unit Priority Low"), PriorityLevel::Low, 7, clock);
    WorkComponent* normalPrioritisedUnit = new PrioritisedTask(new WorkUnit("Test Unit Priority Normal"), PriorityLevel::Normal, 7, clock);
    WorkComponent* highPrioritisedUnit = new PrioritisedTask(new WorkUnit("Test Unit Priority High"), PriorityLevel::High, 7, clock);
    WorkComponent* urgentPrioritisedUnit = new PrioritisedTask(new WorkUnit("Test Unit Priority Urgent"), PriorityLevel::Urgent, 7, clock);

    WorkComponent* overduePrioritisedUnit = new PrioritisedTask(new WorkUnit("Test Unit Priority Overdue"), PriorityLevel::Urgent, 0, clock);

    WorkGroup group("Test Group");

    //because group cannot accept WorkComponent
    group.add(lowPrioritisedUnit);
    group.add(normalPrioritisedUnit);
    group.add(highPrioritisedUnit);
    group.add(urgentPrioritisedUnit);
    group.add(overduePrioritisedUnit);

    visitor.createReport(&group);

    delete clock;
}

TEST(VisitorTest, AllRulesValidatedTask){
    std::shared_ptr<const RequiredKeyRule> keyRule = std::make_shared<const RequiredKeyRule>("test");
    std::shared_ptr<const NumericRangeRule> rangeRule = std::make_shared<const NumericRangeRule>("test", 0, 100);
    std::shared_ptr<const EmailFormatRule> emailRule = std::make_shared<const EmailFormatRule>("test");

    ValidatedTask* validatedUnit = new ValidatedTask(new WorkUnit("Test Unit Validate"), "Completed");
    validatedUnit->addRule(keyRule);
    validatedUnit->addRule(rangeRule);
    validatedUnit->addRule(emailRule);

    ProgressReportVisitor visitor;
    WorkGroup group("Test Group");

    group.add(validatedUnit);

    visitor.createReport(&group);
}

TEST(VisitorTest, LargeAuditAuditedTask){
    const ManualClock* clock = new ManualClock(); 
    AuditLog* testLog = new AuditLog(clock);
    testLog->record("test1");
    testLog->record("test2");
    testLog->record("test3");
    testLog->record("test4");
    testLog->record("test5");

    WorkComponent* auditedUnit = new AuditedTask(new WorkUnit("Test Unit Audit"), testLog);\

    ProgressReportVisitor visitor;
    WorkGroup group("Test Group");

    group.add(auditedUnit);

    visitor.createReport(&group);

    delete testLog;
    delete clock;
}

TEST(VisitorTest, AllDecoratedCombinationsUnit){
    AuditLog* log = new AuditLog();
    SystemClock* clock = new SystemClock();
    PriorityLevel level = PriorityLevel::Normal;
    int days = 7;
    std::string status = "Completed";

    TaskDecorator* auditValidatePriorityUnit = new AuditedTask(new ValidatedTask(new PrioritisedTask(new WorkUnit("Audited Validated Prioritised Test Unit"), level, days, clock), status), log);
    TaskDecorator* auditPriorityValidateUnit = new AuditedTask(new PrioritisedTask(new ValidatedTask(new WorkUnit("Audited Prioritised Validated Test Unit"), status), level, days, clock), log);
    TaskDecorator* validateAuditPriorityUnit = new ValidatedTask(new AuditedTask(new PrioritisedTask(new WorkUnit("Validated Audited Prioritised Test Unit"), level, days, clock), log), status);
    TaskDecorator* validatePriorityAuditUnit = new ValidatedTask(new PrioritisedTask(new AuditedTask(new WorkUnit("Validated Prioritised Audited Test Unit"), log), level, days, clock), status);
    TaskDecorator* priorityAuditValidateUnit = new PrioritisedTask(new AuditedTask(new ValidatedTask(new WorkUnit("Prioritised Audited Validated Test Unit"), status), log), level, days, clock);
    TaskDecorator* priorityValidateAuditUnit = new PrioritisedTask(new ValidatedTask(new AuditedTask(new WorkUnit("Prioritised Validated Audited Test Unit"), log), status), level, days, clock);

    ProgressReportVisitor visitor;
    WorkGroup group("Test Group");

    group.add(auditValidatePriorityUnit);
    group.add(auditPriorityValidateUnit);
    group.add(validateAuditPriorityUnit);
    group.add(validatePriorityAuditUnit);
    group.add(priorityAuditValidateUnit);
    group.add(priorityValidateAuditUnit);

    visitor.createReport(&group);

    delete log;
    delete clock;
}

TEST(VisitorTest, WorkGroupDecorated){
    AuditLog* log = new AuditLog();
    SystemClock* clock = new SystemClock();
    TaskDecorator* validatedStage = new ValidatedTask(new Stage("Validated Test Stage"), "Completed");
    TaskDecorator* auditedStage = new AuditedTask(new Stage("Audited Test Stage"), log);
    TaskDecorator* prioritisedStage = new PrioritisedTask(new Stage("Prioritised Test Stage"), PriorityLevel::Normal, 7, clock);

    ProgressReportVisitor visitor;
    WorkGroup group("Test Group");

    group.add(validatedStage);
    group.add(auditedStage);
    group.add(prioritisedStage);

    visitor.createReport(&group);

    delete log;
    delete clock;
}