#ifndef COMPLETEDSTATE_H
#define COMPLETEDSTATE_H

#include "WorkState.h"

class CompletedState : public WorkState {
public: 
    void handleUpdate(WorkComponent* context);
    std::string getStatus() override;
    WorkState* clone() override;
};

#endif