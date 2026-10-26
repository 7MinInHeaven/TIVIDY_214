#ifndef INPROGRESSSTATE_H
#define INPROGRESSSTATE_H

#include "WorkState.h"

class InProgressState : public WorkState {
public: 
    void handleUpdate(WorkComponent* context) override;
    std::string getStatus() override;
    WorkState* clone() override;
};

#endif