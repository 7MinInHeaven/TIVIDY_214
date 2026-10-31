#ifndef CHAINBUILDER_H
#define CHAINBUILDER_H
#include <string>
#include <vector>
#include <stdexcept>
#include "Composite.h"

class ChainBuilder {
public:
    virtual ~ChainBuilder() = default;

    virtual void reset() = 0;

    virtual void buildWorkGroup(const std::string& workflowName) = 0;
    virtual void buildStage(const std::string& name) = 0;
    virtual void buildSubProcess(const std::string& name) = 0;
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
    virtual void endGroup() = 0;
    virtual Handler* getResult() = 0;
    virtual std::vector<WorkGroup*> getWorkGroups() = 0;
};

#endif /* CHAINBUILDER_H */