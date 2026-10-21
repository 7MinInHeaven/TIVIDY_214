#ifndef DIRECTOR_H
#define DIRECTOR
#include "ChainBuilder.h"
#include <string>

class Director 
{
    private:
        ChainBuilder* builder;

    public:
        Director(ChainBuilder* b);
        void construct(const std::string& type); //enum in future
};

#endif