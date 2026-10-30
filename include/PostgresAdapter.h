/**
 * @file PostgresAdapter.h
 * @brief Declares the Adapter and ConcreteStrategy for Cloud PostgreSQL 
 */

#ifndef POSTGRESADAPTER_H
#define POSTGRESADAPTER_H

#include "ExternalDatabase.h"
#include "PostgresAPI.h"
#include <string>
#include <vector>

/**
 * @class PostgresAdapter
 * @brief Adapter in the Adapter pattern and ConcreteStrategy in the Strategy pattern for PostgreSQL
 *
 * Adapts the low-level PostgresAPI (`Adaptee`) to the ExternalDatabase (Target)
 * interface so DatabaseObserver can persist and retrieve workflow records in Neon Cloud PostgreSQL
 *
 * @see ExternalDatabase
 * @see PostgresAPI
 * @see DatabaseObserver
 */
class PostgresAdapter : public ExternalDatabase {
private:
    PostgresAPI* pgApi;    /**< Pointer to the wrapped PostgresAPI Adaptee */
    std::string tableName; /**< Sanitized SQL table name used for storing audit records */
    bool ownsAPI;          /**< Indicates whether this adapter owns and deletes `pgApi` */
    bool tableEnsured;     /**< Tracks whether `CREATE TABLE IF NOT EXISTS` has already run for this adapter */

    /**
     * @brief Escapes single quotes in strings for SQL safety
     *
     * @param[in] input Raw input string
     * @return Sanitized string with `'` escaped
     */
    std::string rmvQuotes(const std::string& input) const;

public:
    /**
     * @brief Constructs a PostgresAdapter wrapping a PostgresAPI instance
     *
     * @param[in] pgApi Pointer to the PostgresAPI Adaptee instance
     * @param[in] table Target SQL table name (defaults to `"workflow_audit_log"`)
     * @param[in] takeOwnership If true, the adapter deletes `pgApi` in its destructor
     */
    PostgresAdapter(PostgresAPI* pgApi, std::string table = "workflow_audit_log", bool takeOwnership = true);

    /**
     * @brief Destructor that deletes the wrapped PostgresAPI.
     */
    virtual ~PostgresAdapter();

    /**
     * @brief Inserts a workflow audit record into the PostgreSQL table
     *
     * @param[in] taskID Integer identifier of the task
     * @param[in] taskName Name of the WorkComponent
     * @param[in] state Current workflow state string
     * @param[in] payload Serialized key-value metadata
     * @return true if the transaction succeeded, false otherwise
     */
    virtual bool saveRecord(int taskID, std::string taskName, std::string state, std::string payload) override;

    /**
     * @brief Queries the latest persisted state for the specified task name from PostgreSQL
     *
     * @param[in] taskName Name of the task to look up
     * @return Latest state string, or an empty string if not found
     */
    virtual std::string getRecordState(std::string taskName) override;

    /**
     * @brief Retrieves all audit records from the PostgreSQL table ordered by `id ASC`
     *
     * @return Vector of records formatted as `"task_id | task_name | state | payload"`
     */
    virtual std::vector<std::string> fetchAllRecords() override;

    /**
     * @brief Checks whether the underlying PostgresAPI connection is active.
     *
     * @return true if connected, false otherwise.
     */
    virtual bool isConnected() override;
};

#endif