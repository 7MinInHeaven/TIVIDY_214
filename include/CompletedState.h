#ifndef COMPLETEDSTATE_H
#define COMPLETEDSTATE_H

#include "WorkState.h"

class CompletedState : public WorkState {
public: 
    void handleUpdate(WorkComponent* context) override;
    std::string getStatus() override;
};

#endif