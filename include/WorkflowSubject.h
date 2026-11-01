/**
 * @file WorkflowSubject.h
 * @brief Declares the Subject participant for the Observer pattern
 */
#ifndef WORKFLOWSUBJECT_H
#define WORKFLOWSUBJECT_H

#include "WorkflowObserver.h"
#include <vector>

/**
 * @class WorkflowSubject
 * @brief Subject participant in the Observer design pattern
 *
 * Maintains a list of attached WorkflowObserver pointers and broadcasts
 * notifications whenever the subject's state or payload changes
 *
 * @see WorkflowObserver
 * @see WorkComponent
 */

class WorkflowSubject {
protected:
    /** @brief Collection of registered observers watching this subject. */
    std::vector<WorkflowObserver*> observerList;

public:
    /**
     * @brief Default constructor. Creates a WorkFlowSubject with no attached observers
     */
    WorkflowSubject();
     /**
     * @brief Virtual destructor. Does not delete attached observers (aggregation relationship)
     */
    virtual ~WorkflowSubject();

     /**
     * @brief Registers an observer to receive state and payload change notifications
     *
     * @param[in] observer Pointer to the WorkflowObserver to attach (ignored if null or already attached)
     */
    void attach(WorkflowObserver* observer);

    /**
     * @brief Unregisters a previously attached observer
     *
     * @param[in] observer Pointer to the WorkflowObserver to remove
     */
    void detach(WorkflowObserver* observer);

    /**
     * @brief Notifies all attached observers by setting the active subject and calling update()
     */
    void notify();
};

#endif