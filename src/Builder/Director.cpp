#include "Director.h"
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {


const int kMaxInputAttempts = 3;

} // namespace



Director::Director(ChainBuilder* b, WorkItem* i, bool in): builder(b), item(i), input(in) {}

Director::~Director() {};

void Director::construct(const json& data) {
    if (builder == nullptr) 
    {
        return;
    }

    builder->reset();

    if (item != nullptr && data.contains("required_inputs"))
    {
        for (const auto& req : data["required_inputs"])
        {
            if (!req.contains("key")) continue;

            std::string key = req["key"];
            std::string prompt = req.value("prompt", key);

            std::string value;
            bool accepted = false;
            for (int attempt = 1; attempt <= kMaxInputAttempts; ++attempt) {
                std::cout << prompt << " (" << key << "): " << std::endl;
                if (!std::getline(std::cin, value)) {
                    throw std::runtime_error(
                        "input stream closed while reading required input: " +
                        key);
                }
                if (!value.empty()) {
                    item->setData(key, value);
                    accepted = true;
                    break;
                }
                std::cerr << "Input cannot be empty. Attempt "
                          << attempt << " of " << kMaxInputAttempts << ".\n";
            }

            if (!accepted) {
                throw std::invalid_argument(
                    "required input was empty after " +
                    std::to_string(kMaxInputAttempts) + " attempts: " + key);
            }
        }
    }

    //move through chain
    if (!data.contains("chain")) 
    {
        return;
    }

    for (const auto& workGroup : data["chain"]) 
    {
        parseNode(workGroup);
    }
}

void Director::parseNode(const json& node) {
    std::string type;
    if (node.contains("type"))
    {
        type = std::string(node["type"]);
    }
    else 
    {
        type = "WorkGroup"; //default
    }
    std::string name = node["name"];

    if (type == "WorkGroup") 
    {
        builder->buildWorkGroup(name);
        parseChildren(node);
        builder->endGroup();
    } else if (type == "Stage") 
    {
        builder->buildStage(name);
        parseChildren(node);
        builder->endGroup();
    } else if (type == "SubProcess") 
    {
        builder->buildSubProcess(name);
        parseChildren(node);
        builder->endGroup();
    } else 
    {
        builder->buildWorkUnit(name);
    }
}

void Director::parseChildren(const json& parent) 
{
    if (!parent.contains("children")) {
        return;
    }

    for (const auto& child : parent["children"]) 
    {
        parseNode(child);
    }
}
