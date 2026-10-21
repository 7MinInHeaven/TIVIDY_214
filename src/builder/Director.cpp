#include "Director.h"

Director::Director(ChainBuilder* b) : builder(b) {}

void construct(const std::string& type) 
{
        builder->reset();
        builder->buildStep("Initialisation");
        builder->buildStep("Routing");

        if (type == "STEP1") {
            builder->buildStep("S1");
        } 
        else if (type == "STEP2") 
        {
            builder->buildStep("S2");
        } 
        else if (type == "STEP3") 
        {
            builder->buildStep("S3");
        }

        builder->buildStep("Final");
    }