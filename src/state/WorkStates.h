#include <string>
#include "WorkComponent.h"

class WorkComponent;

class WorkState {
    public: 
    virtual ~WorkState() {}    
    virtual void handleUpdate(WorkComponent* context) = 0;
    virtual std::string getStatus() = 0;
};
