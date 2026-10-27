#ifndef HANDLER_H
#define HANDLER_H


#include <string>
#include "WorkItem.h"

class Handler
{
    protected: 
        Handler* successor = nullptr;;
    public: 
        void setNext(Handler* nextPhase);
        virtual Handler* clone() = 0;
        virtual void handleRequest(WorkItem item);
};

#endif