#ifndef WORKSTATE_H
#define WORKSTATE_H

#include <string>

class WorkComponent; 

class WorkState {
public: 
    virtual ~WorkState() {}    
    virtual void handleUpdate(WorkComponent* context) = 0;
    virtual std::string getStatus() = 0;
};

#endif