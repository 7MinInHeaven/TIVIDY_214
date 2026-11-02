#ifndef WORKFLOWACTIONREGISTRY_H
#define WORKFLOWACTIONREGISTRY_H

#include <string>

/**
 * @brief Defines the action and integration keys accepted in a blueprint.
 */
class WorkflowActionRegistry {
public:
    /**
     * @brief Checks whether an action key is allow-listed.
     * @param actionKey Action key from a workflow blueprint.
     * @return True when the action key is supported by the registry.
     */
    static bool isSupportedAction(const std::string& actionKey);

    /**
     * @brief Checks whether a capability key is allow-listed.
     * @param capabilityKey Capability key from a workflow blueprint.
     * @return True when the capability key is supported by the registry.
     */
    static bool isSupportedCapability(const std::string& capabilityKey);
};

#endif
