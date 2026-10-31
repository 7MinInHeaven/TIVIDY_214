#include <gtest/gtest.h>
#include "Iterator.h"

TEST(IteratorTest, DepthFirstIteratorTest){
    WorkGroup group("Test Group");
    Stage* stage1 = new Stage("Test Stage One");
    Stage* stage2 = new Stage("Test Stage Two");

    SubProcess* stage1process1 = new SubProcess("Stage One Process One");
    SubProcess* stage1process2 = new SubProcess("Stage One Process Two");
    SubProcess* stage2process1 = new SubProcess("Stage Two Process One");
    SubProcess* stage2process2 = new SubProcess("Stage Two Process Two");

    WorkUnit* stage1process1unit1 = new WorkUnit("Stage One Process One Unit One");
    WorkUnit* stage1process1unit2 = new WorkUnit("Stage One Process One Unit Two");
    WorkUnit* stage1process2unit1 = new WorkUnit("Stage One Process Two Unit One");
    WorkUnit* stage1process2unit2 = new WorkUnit("Stage One Process Two Unit Two");
    WorkUnit* stage2process1unit1 = new WorkUnit("Stage Two Process One Unit One");
    WorkUnit* stage2process1unit2 = new WorkUnit("Stage Two Process One Unit Two");
    WorkUnit* stage2process2unit1 = new WorkUnit("Stage Two Process Two Unit One");
    WorkUnit* stage2process2unit2 = new WorkUnit("Stage Two Process Two Unit Two");

    stage1process1->add(stage1process1unit1);
    stage1process1->add(stage1process1unit2);
    stage1process2->add(stage1process2unit1);
    stage1process2->add(stage1process2unit2);
    stage2process1->add(stage2process1unit1);
    stage2process1->add(stage2process1unit2);
    stage2process2->add(stage2process2unit1);
    stage2process2->add(stage2process2unit2);

    stage1->add(stage1process1);
    stage1->add(stage1process2);
    stage2->add(stage2process1);
    stage2->add(stage2process2);

    group.add(stage1);
    group.add(stage2);

    DepthFirstIterator it(&group);
    EXPECT_EQ(it.getCurrent(), stage1process1unit1);
    it.next();
    EXPECT_EQ(it.getCurrent(), stage1process1unit2);
    it.next();

    EXPECT_EQ(it.getCurrent(), stage1process1);
    it.next();

    EXPECT_EQ(it.getCurrent(), stage1process2unit1);
    it.next();
    EXPECT_EQ(it.getCurrent(), stage1process2unit2);
    it.next();

    EXPECT_EQ(it.getCurrent(), stage1process2);
    it.next();

    EXPECT_EQ(it.getCurrent(), stage1);
    it.next();

    EXPECT_EQ(it.getCurrent(), stage2process1unit1);
    it.next();
    EXPECT_EQ(it.getCurrent(), stage2process1unit2);
    it.next();

    EXPECT_EQ(it.getCurrent(), stage2process1);
    it.next();

    EXPECT_EQ(it.getCurrent(), stage2process2unit1);
    it.next();
    EXPECT_EQ(it.getCurrent(), stage2process2unit2);
    it.next();
    
    EXPECT_EQ(it.getCurrent(), stage2process2);
    it.next();

    EXPECT_EQ(it.getCurrent(), stage2);
    it.next();

    EXPECT_EQ(it.getCurrent(), &group);
    it.next();
    
    EXPECT_TRUE(it.isDone());
}

TEST(IteratorTest, StateIteratorTest){
    WorkGroup group("Test Group");
    WorkUnit* pendingUnit = new WorkUnit("Pending Unit");
    WorkUnit* inProgressUnit = new WorkUnit("In Progress Unit");
    inProgressUnit->nextState();

    WorkUnit* completedUnit = new WorkUnit("Completed Unit");
    completedUnit->nextState();
    completedUnit->nextState();
    
    group.add(inProgressUnit);
    group.add(completedUnit);
    group.add(pendingUnit);

    StateIterator pendingIterator(&group, "Pending");
    StateIterator inProgressIterator(&group, "In Progress");
    StateIterator completedIterator(&group, "Completed");

    // Only the untouched unit is Pending; the group is In Progress because
    // some of its children have started.
    EXPECT_EQ(pendingIterator.getCurrent(), pendingUnit);
    pendingIterator.next();
    EXPECT_TRUE(pendingIterator.isDone());

    EXPECT_EQ(inProgressIterator.getCurrent(), inProgressUnit);
    inProgressIterator.next();
    EXPECT_EQ(inProgressIterator.getCurrent(), &group); // derived from its children
    inProgressIterator.next();
    EXPECT_TRUE(inProgressIterator.isDone());

    EXPECT_EQ(completedIterator.getCurrent(), completedUnit);
    completedIterator.next();
    EXPECT_TRUE(completedIterator.isDone()); 
}

TEST(IteratorTest, IncompleteIteratorTest){
    WorkGroup group("Test Group");
    
    WorkUnit* pendingUnit = new WorkUnit("Pending Unit");
    WorkUnit* inProgressUnit = new WorkUnit("In Progress Unit");
    inProgressUnit->nextState();
    WorkUnit* completedUnit = new WorkUnit("Completed Unit");
    completedUnit->nextState();
    completedUnit->nextState();

    group.add(inProgressUnit);
    group.add(completedUnit);
    group.add(pendingUnit);

    IncompleteIterator it(&group);

    EXPECT_EQ(it.getCurrent(), inProgressUnit);
    it.next();
    EXPECT_EQ(it.getCurrent(), pendingUnit);
    it.next();
    EXPECT_EQ(it.getCurrent(), &group);
    it.next();
    EXPECT_TRUE(it.isDone());
}

TEST(IteratorTest, PriorityIteratorTest){
    WorkGroup group("Test Group");

    ManualClock* clock = new ManualClock(1'000'000);

    TaskDecorator* lowPriority = new PrioritisedTask(new WorkUnit("Low Prior Unit"), PriorityLevel::Low, 7);
    TaskDecorator* normalPriority = new PrioritisedTask(new WorkUnit("Normal Prior Unit"), PriorityLevel::Normal, 7);
    TaskDecorator* highPriority = new PrioritisedTask(new WorkUnit("High Prior Unit"), PriorityLevel::High, 7);
    TaskDecorator* urgentPriority = new PrioritisedTask(new WorkUnit("Urgent Prior Unit"), PriorityLevel::Urgent, 7);

    TaskDecorator* lowOverduePriority = new PrioritisedTask(new WorkUnit("Low Overdue Unit"), PriorityLevel::Low, 1, clock);
    lowOverduePriority->nextState();
    TaskDecorator* normalOverduePriority = new PrioritisedTask(new WorkUnit("Normal Overdue Unit"), PriorityLevel::Normal, 1, clock);
    normalOverduePriority->nextState();
    TaskDecorator* highOverduePriority = new PrioritisedTask(new WorkUnit("High Overdue Unit"), PriorityLevel::High, 1, clock);
    highOverduePriority->nextState();
    TaskDecorator* urgentOverduePriority = new PrioritisedTask(new WorkUnit("Urgent Overdue Unit"), PriorityLevel::Urgent, 1, clock);
    urgentOverduePriority->nextState();

    clock->advanceDays(3);

    group.add(lowPriority);
    group.add(highPriority);
    group.add(urgentPriority);
    group.add(normalPriority);

    group.add(highOverduePriority);
    group.add(urgentOverduePriority);
    group.add(lowOverduePriority);
    group.add(normalOverduePriority);

    PriorityIterator it(&group);

    EXPECT_EQ(it.getCurrent(), urgentOverduePriority);
    it.next();
    EXPECT_EQ(it.getCurrent(), highOverduePriority);
    it.next();
    EXPECT_EQ(it.getCurrent(), normalOverduePriority);
    it.next();
    EXPECT_EQ(it.getCurrent(), lowOverduePriority);
    it.next();
    
    EXPECT_EQ(it.getCurrent(), urgentPriority);
    it.next();
    EXPECT_EQ(it.getCurrent(), highPriority);
    it.next();
    EXPECT_EQ(it.getCurrent(), normalPriority);
    it.next();
    EXPECT_EQ(it.getCurrent(), lowPriority);
    it.next();
    EXPECT_TRUE(it.isDone());

    delete clock;
}