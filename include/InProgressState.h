/**
 * @file InProgressState.h
 * @brief State for a task or group whose work has started but is not finished.
 */
#ifndef INPROGRESSSTATE_H
#define INPROGRESSSTATE_H

#include "WorkState.h"
/**
 * @class InProgressState
 * @brief State in the State pattern for work that has started; the next update completes it.
 */
class InProgressState : public WorkState {
public: 
    /** @brief Creates an In Progress state. */
    InProgressState();

    /**
     * @brief Handles an update request by moving the context to the Completed state.
     * @param context The work component that holds this state.
     */
    void handleUpdate(WorkComponent* context);

    /** @brief Returns the status name, "In Progress". */
    std::string getStatus() const override;
    
    /** @brief Creates a copy of this state */
    WorkState* clone() override;

    /** @brief Destroys the state. */
    virtual ~InProgressState();
};

#endif