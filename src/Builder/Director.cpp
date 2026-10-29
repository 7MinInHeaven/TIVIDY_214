#include "Director.h"
#include <iostream>
#include <vector>

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

            std::cout << prompt << " (" << key << "): " << std::endl;
            std::string value;
            std::getline(std::cin, value);
            item->setData(key, value);
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
        // parseRule(node);
        // parseDecorators(node);
        parseChildren(node);
        builder->endGroup();
    } else if (type == "Stage") 
    {
        builder->buildStage(name);
        // parseRule(node);
        // parseDecorators(node);
        parseChildren(node);
        builder->endGroup();
    } else if (type == "SubProcess") 
    {
        builder->buildSubProcess(name);
        // parseRule(node);
        // parseDecorators(node);
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

// void Director::parseDecorators(const json& node) 
// {
//     if (!node.contains("decorators")) {
//         return;
//     }

//     for (const auto& decorator : node["decorators"]) 
//     {
//         std::string type = decorator["type"];
//         std::vector<std::string> requiredKeys;

//         if (decorator.contains("required_keys")) 
//         {
//             for (const auto& k : decorator["required_keys"]) //needed for decorator constructors
//             {
//                 requiredKeys.push_back(k);
//             }
//         }

//         builder->addDecorator(type, requiredKeys);
//     }
// }

// void Director::parseRule(const json& node) 
// {
//     if (node.contains("has_rule") && node["has_rule"] == true && node.contains("rule")) 
//     {
//         const auto& rule = node["rule"]; //get the rule object from the JSON
//         builder->addRule(rule["key"], rule["op"], rule["value"]);
//     }
// }