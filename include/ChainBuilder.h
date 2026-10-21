#ifndef CHAINBUILDER_H
#define CHAINBUILDER_H

#include "Handler.h"


class ChainBuilder {
public:
    virtual ~ChainBuilder() = default;
    virtual void reset() = 0;
    virtual void buildNestedProcess(const std::string& config) = 0;
    virtual Handler* getResult() = 0;
};

#endif 