/**
 * @file ChainBuilder.h
 * @brief Abstract Builder interface for assembling a chain of WorkGroups.
 */

#ifndef CHAINBUILDER_H
#define CHAINBUILDER_H
#include <string>
#include <vector>
#include <stdexcept>
#include "Composite.h"

/**
 * @class ChainBuilder
 * @brief Abstract builder that constructs a chain of WorkGroups and their children.
 */
class ChainBuilder {
public:
    /** @brief Virtual destructor so concrete builders are destroyed correctly. 
     * 
     * */    
    virtual ~ChainBuilder() = default;

    /**
     * @brief Discards any partly built result and returns the builder to its initial state.
     */
    virtual void reset() = 0;

    /**
     * @brief Builds a top-level or nested WorkGroup and opens it for children.
     * @param workflowName Name of the group.
     */
    virtual void buildWorkGroup(const std::string& workflowName) = 0;
    
    /**
     * @brief Builds a Stage and opens it for children.
     * @param name Name of the stage.
     */
    virtual void buildStage(const std::string& name) = 0;
    
    /**
     * @brief Builds a SubProcess and opens it for children.
     * @param name Name of the sub-process.
     */
    virtual void buildSubProcess(const std::string& name) = 0;
    
    /**
     * @brief Builds a WorkUnit (a leaf task) inside the currently open group.
     * @param name Name of the unit.
     */
    virtual void buildWorkUnit(const std::string& name) = 0;
    
    /**
     * @brief Adds a decorator when the concrete builder supports decorators.
     * @throws std::logic_error when decorators are not implemented.
     */
    virtual void addDecorator(const std::string&) {
        throw std::logic_error("this ChainBuilder does not support decorators");
    }

    /**
     * @brief Adds a configured decorator when the concrete builder supports decorators.
     * @throws std::logic_error when decorators are not implemented.
     */
    virtual void addDecorator(
        const std::string&,
        const std::vector<std::string>&) {
        throw std::logic_error("this ChainBuilder does not support decorators");
    }

    /**
     * @brief Closes the most recently opened group.
     */
    virtual void endGroup() = 0;

    /**
     * @brief Returns the head of the finished chain.
     * @return First handler of the chain, or nullptr if nothing has been built.
     */
    virtual Handler* getResult() = 0;

    /**
     * @brief Returns the top-level groups in the order they were built.
     * @return The root WorkGroups of the chain.
     */
    virtual std::vector<WorkGroup*> getWorkGroups() = 0;
};

#endif /* CHAINBUILDER_H */