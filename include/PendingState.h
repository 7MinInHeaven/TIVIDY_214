/**
 * @file PendingState.h
 * @brief State for a task or group whose work has not started yet.
 */
#ifndef PENDINGSTATE_H
#define PENDINGSTATE_H

#include "WorkState.h"

/**
 * @class PendingState
 * @brief Initial state in the State pattern; the next update moves the work to In Progress.
 */
class PendingState : public WorkState {
public: 
    /** @brief Creates a Pending state. */
    PendingState();

    /**
     * @brief Handles an update request by moving the context to the In Progress state.
     * @param context The work component that holds this state.
     */
    void handleUpdate(WorkComponent* context);

    /** @brief Returns the status name, "Pending". */
    std::string getStatus() const override;
    
    /** @brief Creates a copy of this state. */
    WorkState* clone() override;

    /** @brief Destroys the state. */
    virtual ~PendingState();
};

#endif