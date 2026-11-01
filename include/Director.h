#ifndef DIRECTOR_H
#define DIRECTOR_H

#include <string>
#include <nlohmann/json.hpp>
#include "ChainBuilder.h"
#include "WorkItem.h"

using json = nlohmann::json;

class Director {
private:
    ChainBuilder* builder;
    WorkItem* item;
    bool input;

    void parseNode(const json& node);
    void parseChildren(const json& parentNode);
    void parseDecorators(const json& node);

public:
    Director(ChainBuilder* b, WorkItem* i = nullptr, bool input = false);
    ~Director();
    void construct(const json& data);
};

#endif /* DIRECTOR_H */