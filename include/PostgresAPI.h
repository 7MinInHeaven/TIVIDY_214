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
 * @brief Adaptee participant in the Adapter design pattern for Cloud PostgreSQL 
 *
 * Encapsulates low-level libpq connection management and parameterized SQL execution
 * (`PQexecParams`) for communicating with a remote PostgreSQL server
 *
 * @see PostgresAdapter
 */
class PostgresAPI {
private:
    /**< PostgreSQL libpq connection URI */
    std::string connString;
    /**< Tracks whether the connection is open */
    bool connected;

public:
    /**
     * @brief Constructs a PostgresAPI instance with the given connection string.
     *
     * @param[in] connStr PostgreSQL connection URI (e.g., from `DATABASE_URL`).
     */
    PostgresAPI(std::string connStr = "");

    /**
     * @brief Destructor that ensures disconnection cleanup
     */
    virtual ~PostgresAPI();

    /**
     * @brief Establishes and verifies a live connection to the PostgreSQL server using `PQconnectdb`
     *
     * @return true if the connection succeeded (`CONNECTION_OK`), false otherwise
     */
    virtual bool connect();

     /**
     * @brief Closes the connection
     */
    virtual void disconnect();

     /**
     * @brief Executes a parameterized SQL write transaction (`INSERT`, `UPDATE`, `CREATE TABLE`)
     *
     * @param[in] sqlQuery SQL statement with `$1, $2, ...` placeholders
     * @param[in] params Parameter values bound safely with `PQexecParams`
     * @return Number of affected rows, or `-1` on failure
     */
    virtual int executeTransaction(std::string sqlQuery, const std::vector<std::string>& params = std::vector<std::string>());

    /**
     * @brief Executes a parameterized SQL `SELECT` query and returns formatted result rows
     *
     * @param[in] sqlQuery SQL `SELECT` statement with optional `$1, $2, ...` placeholders
     * @param[in] params Parameter values bound with `PQexecParams`
     * @return Vector of `" | "` delimited row strings
     */
    virtual std::vector<std::string> executeSelect(std::string sqlQuery, const std::vector<std::string>& params = std::vector<std::string>());

    /**
     * @brief Returns whether the API is currently marked as connected
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