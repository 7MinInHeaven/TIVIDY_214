#ifndef PENDINGSTATE_H
#define PENDINGSTATE_H

#include "WorkState.h"

class PendingState : public WorkState {
public: 
    void handleUpdate(WorkComponent* context);
    std::string getStatus() override;
    WorkState* clone() override;
};

#endif