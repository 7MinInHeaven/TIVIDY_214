/**
 * @file PostgresAPI.h
 * @brief Declares the PostgreSQL Adaptee wrapping low-level libpq C API calls
 */

#ifndef POSTGRESAPI_H
#define POSTGRESAPI_H

#include <string>
#include <vector>

/**
 * @class PostgresAPI
 * @brief Adaptee participant in the Adapter design pattern for Cloud PostgreSQL (Neon)
 *
 * Encapsulates low-level libpq connection management and parameterized SQL execution
 * (`PQexecParams`) for communicating with a remote PostgreSQL server
 *
 * @see PostgresAdapter
 */
class PostgresAPI {
private:
    std::string connString; /**< PostgreSQL libpq connection URI or keyword/value string */
    bool connected;         /**< Tracks whether the connection is logically open */
    void* dbConn;           /**< Persistent libpq PGconn* handle reused across queries */

public:
    /**
     * @brief Constructs a PostgresAPI instance with the given connection string
     *
     * @param[in] connStr PostgreSQL connection URI 
     */
    PostgresAPI(std::string connStr = "");

    /**
     * @brief Destructor that closes and frees any active PostgreSQL connection
     */
    virtual ~PostgresAPI();

    /**
     * @brief Establishes or reuses a live connection to the PostgreSQL server via `PQconnectdb`
     *
     * @return true if the connection is active (`CONNECTION_OK`), false otherwise
     */
    virtual bool connect();

    /**
     * @brief Closes the active `PGconn` connection handle via `PQfinish`
     */
    virtual void disconnect();

    /**
     * @brief Executes a parameterized SQL write transaction (`INSERT`, `UPDATE`, `CREATE TABLE`)
     *
     * @param[in] sqlQuery SQL statement with `$1, $2, ...` positional placeholders
     * @param[in] params Parameter values bound safely via `PQexecParams`
     * @return `0` on success, or `-1` on failure
     */
    virtual int executeTransaction(std::string sqlQuery, const std::vector<std::string>& params = std::vector<std::string>());

    /**
     * @brief Executes a parameterized SQL `SELECT` query and returns formatted result rows
     *
     * @param[in] sqlQuery SQL `SELECT` statement with optional `$1, $2, ...` placeholders
     * @param[in] params Parameter values bound via `PQexecParams`
     * @return Vector of `" | "`-delimited row strings
     */
    virtual std::vector<std::string> executeSelect(std::string sqlQuery, const std::vector<std::string>& params = std::vector<std::string>());

    /**
     * @brief Returns whether the API is currently connected
     *
     * @return true if connected, false otherwise
     */
    virtual bool getConnectionStatus() const;

    /**
     * @brief Returns the configured PostgreSQL connection string
     *
     * @return Connection URI string
     */
    std::string getConnString() const;
};

#endif