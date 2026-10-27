#ifndef INPROGRESSSTATE_H
#define INPROGRESSSTATE_H

#include "WorkState.h"

class InProgressState : public WorkState {
public: 
    InProgressState();
    void handleUpdate(WorkComponent* context);
    std::string getStatus() override;
    WorkState* clone() override;
    virtual ~InProgressState();
};

#endif