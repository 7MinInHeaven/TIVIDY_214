#ifndef PENDINGSTATE_H
#define PENDINGSTATE_H

#include "WorkState.h"

class PendingState : public WorkState {
public: 
    PendingState();
    void handleUpdate(WorkComponent* context);
    std::string getStatus() const override;
    WorkState* clone() override;
    virtual ~PendingState();
};

#endif