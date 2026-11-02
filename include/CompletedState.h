/**
 * @file CompletedState.h
 * @brief State for a task or group whose work is finished.
 */
#ifndef COMPLETEDSTATE_H
#define COMPLETEDSTATE_H

#include "WorkState.h"

/**
 * @class CompletedState
 * @brief Final state in the State pattern: the work is done and no further progress is made.
 */
class CompletedState : public WorkState {
public: 
    /** @brief Creates a Completed state. 
     * 
    */
    CompletedState();  
    
    /**
     * @brief Handles an update request; a completed item stays completed.
     * @param context The work component that holds this state.
     */
    void handleUpdate(WorkComponent* context);

     /** @brief Returns the status name, "Completed". */
    std::string getStatus() const override;

    /** @brief Creates a copy of this state.
     * 
    */
    WorkState* clone() override;

    /** @brief Destroys the state. */
    virtual ~CompletedState();
};

#endif