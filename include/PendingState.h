#ifndef PENDINGSTATE_H
#define PENDINGSTATE_H

#include "WorkState.h"

class PendingState : public WorkState {
public: 
    void handleUpdate(WorkComponent* context) override;
    std::string getStatus() override;
};

#endif