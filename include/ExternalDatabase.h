/**
 * @file ExternalDatabase.h
 * @brief Declares the abstract Target (Adapter pattern) and Abstract Strategy (Strategy pattern) interface for database persistence
 */
#ifndef EXTERNALDATABASE_H
#define EXTERNALDATABASE_H

#include <string>
#include <vector>

/**
 * @class ExternalDatabase
 * @brief Target interface in the Adapter pattern and abstract Strategy in the Strategy pattern
 *
 * Defines a uniform way used by DatabaseObserver to store, query,
 * and synchronize workflow audit records regardless of the underlying database engine
 *
 * @see DatabaseObserver
 * @see PostgresAdapter
 * @see SQLiteAdapter
 */
class ExternalDatabase{
    public:
    /**
     * @brief Saves a workflow state transition and payload update record to the database
     *
     * @param[in] taskID Integer identifier of the workflow task
     * @param[in] taskName Name of the WorkComponent (task)
     * @param[in] state Current workflow state name 
     * @param[in] payload Data from the task's WorkItem.
     * @return true if the record was successfully written, false otherwise
     */
    virtual bool saveRecord(int taskID, std::string taskName, std::string state, std::string payload)=0;

     /**
     * @brief Retrieves the most recently saved state for a given task name
     *
     * @param[in] taskName Name of the workflow task to query.
     * @return The latest state string or an empty string if not found or disconnected
     */
    virtual std::string getRecordState(std::string taskName) = 0;

    /**
     * @brief Fetches all saved audit records formatted as `"task_id | task_name | state | payload"`
     *
     * @return Vector of formatted record strings in order
     */
    virtual std::vector<std::string> fetchAllRecords() = 0;

     /**
     * @brief Checks whether the underlying database connection is currently active
     *
     * @return true if connected and ready for queries, false otherwise
     */
    virtual bool isConnected() =0;

     /**
     * @brief Virtual destructor for safe polymorphic cleanup
     */
    virtual ~ExternalDatabase(){};


};


#endif