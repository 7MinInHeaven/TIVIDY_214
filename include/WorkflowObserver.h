/**
 * @file WorkflowObserver.h
 * @brief Declares the abstract Observer interface for the Workflow Observer pattern.
 */
#ifndef WORKFLOWOBSERVER_H
#define WORKFLOWOBSERVER_H

class WorkComponent;

/**
 * @class WorkflowObserver
 * @brief Abstract Observer participant in the Observer design pattern
 *
 * Defines the notification interface for objects that should be notified
 * whenever a WorkflowSubject (a WorkComponent) changes its state or payload data
 *
 * @see WorkflowSubject
 * @see DatabaseObserver
 */

class WorkflowObserver {
public:
    /**
     * @brief Virtual destructor for safe polymorphic cleanup
     */
    virtual ~WorkflowObserver() {}

    /**
     * @brief Updates the active subject pointer before notification
     *
     * This allows a single observer instance to monitor multiple WorkComponent subjects
     *
     * @param[in] subject Pointer to the WorkComponent that triggered the notification
     */
    virtual void setSubject(WorkComponent*) {}

    /**
     * @brief Pure virtual pull-notification hook called by WorkflowSubject::notify()
     */
    virtual void update() = 0;
};

#endif