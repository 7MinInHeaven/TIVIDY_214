#include <gtest/gtest.h>
#include "Composite.h"
#include "Memento.h"

TEST(MementoTests, SaveAndRestoreWorkUnitState){
    //1.Initialize task and assign initial worker
    WorkUnit task("Task-100");
    task.assignTask("Original_Worker");
    EXPECT_EQ(task.getAssignedWorkerId(), "Original_Worker");

    //2.create memento capturing state
    WorkMemento* memento = task.createMemento();
    ASSERT_NE(memento, nullptr);

    //3.change data inside workunit obj(after snapshot)
    task.assignTask("Modified_Worker");
    EXPECT_EQ(task.getAssignedwWorkerId(), "Modified_Worker");

    //4.restore state via memento and verify it reverted
    task.setMemento(memento);
    EXPECT_EQ(task.getAssignedWorkerId(), "Original_Worker");

    //5.cleanup allocated memory
    delete memento;

}

