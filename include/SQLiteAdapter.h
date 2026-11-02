/**
 * @file SQLiteAdapter.h
 * @brief Declares the Adapter and ConcreteStrategy for local SQLite
 */

#ifndef SQLiteADAPTER_H
#define SQLiteADAPTER_H

#include "ExternalDatabase.h"
#include "SQLiteAPI.h"
#include <string>
#include <vector>

/**
 * @class SQLiteAdapter
 * @brief Adapter in the Adapter pattern and ConcreteStrategy in the Strategy pattern for SQLite
 *
 * Adapts the low-level SQLiteAPI (Adaptee) to the ExternalDatabase (Target)
 * interface so DatabaseObserver can persist and retrieve workflow records locally in a `.db` file
 *
 * @see ExternalDatabase
 * @see SQLiteAPI
 * @see DatabaseObserver
 */
class SQLiteAdapter : public ExternalDatabase {
private:
    SQLiteAPI* api;        /**< Pointer to the wrapped SQLiteAPI Adaptee */
    std::string tableName; /**< Sanitized SQL table name used for storing audit records */
    bool ownsAPI;          /**< Indicates whether this adapter owns and deletes `api` */

    /**
     * @brief Strips single quotes from strings to sanitize identifiers and fallback queries
     *
     * @param[in] input Raw input string
     * @return Sanitized string with `'` characters removed
     */
    std::string rmvQuotes(const std::string& input) const;

public:
    /**
     * @brief Constructs a SQLiteAdapter, opens the SQLite connection, and ensures the target table exists
     *
     * @param[in] sqApi Pointer to the SQLiteAPI Adaptee instance
     * @param[in] table Target SQL table name (defaults to `"workflow_audit_log"`)
     * @param[in] takeOwnership If true, the adapter deletes `sqApi` in its destructor
     */
    SQLiteAdapter(SQLiteAPI* sqApi, std::string table = "workflow_audit_log", bool takeOwnership = true);

    /**
     * @brief Destructor that closes the connection and optionally deletes the wrapped SQLiteAPI
     */
    virtual ~SQLiteAdapter();

    /**
     * @brief Inserts a workflow audit record into the local SQLite table.
     *
     * @param[in] taskID Numeric identifier of the task
     * @param[in] taskName Name of the WorkComponent
     * @param[in] state Current workflow state string
     * @param[in] payload payload data
     * @return true if the statement succeeded, false otherwise
     */
    virtual bool saveRecord(int taskID, std::string taskName, std::string state, std::string payload) override;

    /**
     * @brief Queries the latest saved state for the specified task name from SQLite
     *
     * @param[in] taskName Name of the task to look up
     * @return Latest state string, or an empty string if not found
     */
    virtual std::string getRecordState(std::string taskName) override;

    /**
     * @brief Retrieves all audit records from the local SQLite table ordered by `id ASC`
     *
     * @return Vector of records formatted as `"task_id | task_name | state | payload"`
     */
    virtual std::vector<std::string> fetchAllRecords() override;

    /**
     * @brief Checks whether the underlying SQLiteAPI file connection is open.
     *
     * @return true if open, false otherwise.
     */
    virtual bool isConnected() override;
};

#endif