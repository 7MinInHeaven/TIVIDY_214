#include "WorkflowActionRegistry.h"

#include <set>

namespace {

/**
 * @brief Returns the safe action keys available for future dispatch.
 */
const std::set<std::string>& supportedActions() {
    static const std::set<std::string> actions = {
        "workflow.log",
        "workflow.validate_input",
        "workflow.set_state",
        "notification.send_result",
        "audit.record"
    };
    return actions;
}

/**
 * @brief Returns the integration capability keys available to blueprints.
 */
const std::set<std::string>& supportedCapabilities() {
    static const std::set<std::string> capabilities = {
        "sqlite_audit",
        "postgres_audit",
        "email_notification",
        "app_notification"
    };
    return capabilities;
}

} // namespace

bool WorkflowActionRegistry::isSupportedAction(
    const std::string& actionKey) {
    return supportedActions().count(actionKey) != 0;
}

bool WorkflowActionRegistry::isSupportedCapability(
    const std::string& capabilityKey) {
    return supportedCapabilities().count(capabilityKey) != 0;
}
