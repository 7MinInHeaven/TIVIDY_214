/**
 * @file SQLiteAPI.h
 * @brief Declares the SQLite Adaptee wrapping low-level sqlite3 C API calls
 */

#ifndef SQLITEAPI_H
#define SQLITEAPI_H

#include <string>
#include <vector>

/**
 * @class SQLiteAPI
 * @brief Adaptee participant in the Adapter design pattern for local SQLite
 *
 * Encapsulates low-level `<sqlite3.h>` file connection handling and prepared statement
 * execution (`sqlite3_prepare_v2`, `sqlite3_bind_text`, `sqlite3_step`) on a local `.db` file
 *
 * @see SQLiteAdapter
 */
class SQLiteAPI {
private:
    std::string dbFilePath; /**< File path to the local SQLite database file. */
    bool connected;         /**< Tracks whether the local SQLite database is open and accessible. */

public:
    /**
     * @brief Constructs a SQLiteAPI instance pointing to a local database file
     *
     * @param[in] filePath Path to the SQLite `.db` file (defaults to `"tividy_local.db"`)
     */
    SQLiteAPI(std::string filePath = "tividy_local.db");

    /**
     * @brief Destructor that closes the connection 
     */
    virtual ~SQLiteAPI();

    /**
     * @brief Opens and verifies access to the local SQLite database file via `sqlite3_open`
     *
     * @return true if the SQLite file was opened successfully, false otherwise
     */
    virtual bool openConnection();

    /**
     * @brief Marks the local SQLite connection as closed
     */
    virtual void closeConnection();

    /**
     * @brief Executes a parameterized SQL write statement (`INSERT`, `UPDATE`, `CREATE TABLE`)
     *
     * @param[in] sqlQuery SQL statement with `?` placeholders.
     * @param[in] params Parameter values bound safely with `sqlite3_bind_text`
     * @return Number of rows modified (`sqlite3_changes`), or `-1` on error
     */
    virtual int runStatement(std::string sqlQuery, const std::vector<std::string>& params = std::vector<std::string>());

    /**
     * @brief Executes a parameterized SQL `SELECT` query and returns formatted result rows
     *
     * @param[in] sqlQuery SQL `SELECT` statement with optional `?` placeholders
     * @param[in] params Parameter values bound with `sqlite3_bind_text`
     * @return Vector of `" | "`-delimited row strings
     */
    virtual std::vector<std::string> queryRows(std::string sqlQuery, const std::vector<std::string>& params = std::vector<std::string>());

    /**
     * @brief Returns whether the local SQLite connection is currently open
     *
     * @return true if open, false otherwise.
     */
    virtual bool isOpen() const;

    /**
     * @brief Returns the file path of the local SQLite database
     *
     * @return Database file path string
     */
    std::string getDbFilePath() const;
};

#endif