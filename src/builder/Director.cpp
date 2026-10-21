#include "Director.h"

Director::Director(ChainBuilder* b) : builder(b) {}

void Director::construct(const std::string& type) 
{
    builder->reset();
    builder->buildNestedProcess("Init");

    if (type == "STEP1") 
    {
        builder->buildNestedProcess("S1");
    } 

    builder->buildNestedProcess("Final");
}