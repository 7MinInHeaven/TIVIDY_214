#ifndef CHAINBUILDER_H
#define CHAINBUILDER_H
#include <string>
#include <vector>
#include "Composite.h"

class ChainBuilder {
public:
    virtual ~ChainBuilder() = default;

    virtual void reset() = 0;

    virtual void buildWorkGroup(const std::string& workflowName) = 0;
    virtual void buildStage(const std::string& name) = 0;
    virtual void buildSubProcess(const std::string& name) = 0;
    virtual void buildWorkUnit(const std::string& name) = 0;
    // virtual void addDecorator(const std::string& type,const std::vector<std::string>& requiredKeys = {}) = 0;
    // virtual void addRule(const std::string& key,const std::string& op,const std::string& value) = 0;
    virtual void endGroup() = 0;
    virtual Handler* getResult() = 0;
};

#endif /* CHAINBUILDER_H */