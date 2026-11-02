/**
 * @file WorkState.h
 * @brief Abstract state interface for the State pattern.
 */
#ifndef WORKSTATE_H
#define WORKSTATE_H

#include <string>

class WorkComponent;

/**
 * @class WorkState
 * @brief Base class for the states a WorkComponent can be in.
 */
class WorkState {
public: 
    /** @brief Virtual destructor so concrete states are destroyed correctly. */
    virtual ~WorkState() {}    

    /**
     * @brief Handles an update request for the given component.
     *      
     * @param context The work component that holds this state.
     */
    virtual void handleUpdate(WorkComponent* context) = 0;
    
    /**
     * @brief Returns the human-readable name of the state.
     * 
     * @return "Pending", "In Progress" or "Completed" for the built-in states.
     */
    virtual std::string getStatus() const = 0;

    /**
     * @brief Creates a copy of this state
     * 
     * @return A new copy of the State
     */
    virtual WorkState* clone() = 0;
};

#endif
