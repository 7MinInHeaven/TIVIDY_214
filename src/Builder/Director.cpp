#include "Director.h"
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {

/**
 * @brief Limits retries for missing required workflow input values.
 */
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
        // parseDecorators(node);
        parseChildren(node);
        builder->endGroup();
    } else if (type == "Stage") 
    {
        builder->buildStage(name);
        // parseDecorators(node);
        parseChildren(node);
        builder->endGroup();
    } else if (type == "SubProcess") 
    {
        builder->buildSubProcess(name);
        // parseDecorators(node);
        parseChildren(node);
        builder->endGroup();
    } else
    {
        builder->buildWorkUnit(name);
        parseDecorators(node);
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

/**
 * @brief Applies the decorators declared on the current WorkUnit node.
 */
void Director::parseDecorators(const json& node)
{
    if (!node.contains("decorators")) {
        return;
    }

    for (const auto& decorator : node["decorators"]) {
        const std::string type = decorator.at("type").get<std::string>();
        std::vector<std::string> requiredKeys;
        if (decorator.contains("required_keys")) {
            for (const auto& key : decorator["required_keys"]) {
                requiredKeys.push_back(key.get<std::string>());
            }
        }

        if (requiredKeys.empty()) {
            builder->addDecorator(type);
        } else {
            builder->addDecorator(type, requiredKeys);
        }
    }
}
