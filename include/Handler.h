#ifndef HANDLER_H
#define HANDLER_H


#include <string>
#include "WorkItem.h"

class Handler
{
    protected: 
        Handler* successor;
    public: 
        void setNext(Handler* nextPhase);
        virtual Handler* clone() = 0;
        void handleRequest(WorkItem item);
};

#endif