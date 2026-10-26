#include <gtest/gtest.h>
#include "Composite.h"
#include "PendingState.h"
#include "InProgressState.h"
#include "CompletedState.h"

TEST(StatePatternTest, InitialStateIsPending) {
    WorkUnit* unit = new WorkUnit("Test Unit");

    EXPECT_EQ(unit->getState()->getStatus(), "Pending");

    delete unit;
}

TEST(StatePatternTest, StateTransitionsCorrectly) {
    WorkUnit* unit = new WorkUnit("Test Unit");

    unit->getState()->handleUpdate(unit);
    EXPECT_EQ(unit->getState()->getStatus(), "In Progress");

    unit->getState()->handleUpdate(unit);
    EXPECT_EQ(unit->getState()->getStatus(), "Completed");

    unit->getState()->handleUpdate(unit);
    EXPECT_EQ(unit->getState()->getStatus(), "Completed");

    delete unit;
}

TEST(StatePatternTest, CloningStatesReturnsIndependentCopies) {
    PendingState pending;
    WorkState* pendingClone = pending.clone();
    EXPECT_EQ(pendingClone->getStatus(), "Pending"); 
    EXPECT_NE(&pending, pendingClone);
    delete pendingClone;

    InProgressState inProgress;
    WorkState* inProgressClone = inProgress.clone();
    EXPECT_EQ(inProgressClone->getStatus(), "In Progress"); 
    EXPECT_NE(&inProgress, inProgressClone);
    delete inProgressClone;

    CompletedState completed;
    WorkState* completedClone = completed.clone();
    EXPECT_EQ(completedClone->getStatus(), "Completed");
    EXPECT_NE(&completed, completedClone);
    delete completedClone;
}