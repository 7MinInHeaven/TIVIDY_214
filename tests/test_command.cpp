#include <gtest/gtest.h>
#include "Composite.h"
#include "Command.h"

TEST(CommandTests,AssignTaskCommandSuccess){
    WorkUnit task("TASK-001");
    AssignTaskCommand assignCmd(&task, "Worker_Alice");

    bool success = assignCmd.execute();

    EXPECT_TRUE(success);
    EXPECT_EQ(task.getAssignedWorkerId(), "Worker_Alice");

}

TEST(CommandTests, StartTaskCommandExecution){
    WorkUnit task("Task-002");
    task.assignTask("Worker_Bob");

    StartTaskCommand startCmd(&task);
    bool success = startCmd.execute();

    EXPECT_TRUE(success);
}

TEST(CommandTests, CompleteTaskCommandExecution){
    WorkUnit task("Task-003");
    task.assignTask("Worker_Charlie");

    CompleteTaskCommand completeCmd(&task);
    bool success = completeCmd.execute();

    EXPECT_TRUE(success);
}
