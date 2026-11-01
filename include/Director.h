/**
 * @file Director.h
 * @brief defines the way objects are created and linked
 */

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

    /**
     * @brief Take a portion of the JSON file and create the assosiated class and its children
     * @param[in] node The portion of the JSON file we are using to create the classes
     */
    void parseNode(const json& node);

    /**
     * @brief Determine if a parent has children and if so hand them over to the node parser
     * @param[in] parent The portion of the JSON file that the contains children Nodes that need to be parsed
     */
    void parseChildren(const json& parentNode);

public:
    /**
     * @brief Constructs a new Director instance 
     * 
     * @param[in] b Pointer to ChainBuilder used to construct components
     * @param[in,out] i Pointer to the WorkItem where required input data is placed
     * @param[in] in Flag indicating whether interactive input prompting is enabled
     */
    Director(ChainBuilder* b, WorkItem* i = nullptr, bool input = false);

    /**
     * @brief Destroys the Director instance.
     */
    ~Director();

    /**
     * @brief Reads in input from the JSON to determine if items need to be added to the WorkItem and creates Composite Nodes from the JSON blueprint
     * @param[in] data The JSON file that forms the blueprint of Builder
     */
    void construct(const json& data);
};

#endif /* DIRECTOR_H */