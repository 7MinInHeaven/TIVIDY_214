#include <gtest/gtest.h>

#include "WorkflowActionRegistry.h"

/**
 * @brief Verifies that the registry accepts the documented demo actions.
 */
TEST(WorkflowActionRegistryTest, AcceptsSupportedActions) {
    EXPECT_TRUE(WorkflowActionRegistry::isSupportedAction("workflow.log"));
    EXPECT_TRUE(
        WorkflowActionRegistry::isSupportedAction("workflow.validate_input"));
    EXPECT_TRUE(
        WorkflowActionRegistry::isSupportedAction("notification.send_result"));
}

/**
 * @brief Verifies that unknown actions are rejected.
 */
TEST(WorkflowActionRegistryTest, RejectsUnknownActions) {
    EXPECT_FALSE(
        WorkflowActionRegistry::isSupportedAction("job_application.run_code"));
}

/**
 * @brief Verifies that known integration capabilities are accepted.
 */
TEST(WorkflowActionRegistryTest, AcceptsSupportedCapabilities) {
    EXPECT_TRUE(
        WorkflowActionRegistry::isSupportedCapability("sqlite_audit"));
    EXPECT_TRUE(
        WorkflowActionRegistry::isSupportedCapability("email_notification"));
}

/**
 * @brief Verifies that unknown integration capabilities are rejected.
 */
TEST(WorkflowActionRegistryTest, RejectsUnknownCapabilities) {
    EXPECT_FALSE(
        WorkflowActionRegistry::isSupportedCapability("unknown_service"));
}
