#include "PostgresAdapter.h"

PostgresAdapter::PostgresAdapter(PostgresAPI* api, std::string table, bool takeOwnership) : pgApi(api), tableName(table), ownsAPI(takeOwnership) {}

PostgresAdapter::~PostgresAdapter() {
    if (ownsAPI && pgApi != nullptr) {
        delete pgApi;
        pgApi = nullptr;
    }
}

std::string PostgresAdapter::rmvQuotes(const std::string& input) const {
    std::string sanitized = "";
    for (size_t i = 0; i < input.length(); ++i) {
        if (input[i] == '\'') {
            sanitized += "''"; // escape single quote for SQL safety
        } else {
            sanitized += input[i];
        }
    }
    return sanitized;
}

bool PostgresAdapter::saveRecord(int taskID, std::string taskName, std::string state, std::string payload) {
    if (pgApi == nullptr) {
        return false;
    }

     //automatically create the table in PostgreSQL if this is a new scenario/tableName
    std::string createTableSql =
        "CREATE TABLE IF NOT EXISTS " + tableName + " ("
        "id SERIAL PRIMARY KEY, "
        "task_id INT, "
        "task_name VARCHAR(255), "
        "state VARCHAR(100), "
        "payload TEXT, "
        "updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);";
    pgApi->executeTransaction(createTableSql);
    
    std::string sqlQuery =
        "INSERT INTO " + tableName + " (task_id, task_name, state, payload) VALUES ($1, $2, $3, $4);";
    std::vector<std::string> params;
    params.push_back(std::to_string(taskID));
    params.push_back(taskName);
    params.push_back(state);
    params.push_back(payload);
    int statusCode = pgApi->executeTransaction(sqlQuery, params);
    return (statusCode == 0);
}

std::string PostgresAdapter::getRecordState(std::string taskName) {
    if (pgApi == nullptr) {
        return "";
    }
    std::string sqlQuery =
        "SELECT state FROM " + tableName +
        " WHERE task_name = $1 ORDER BY id DESC LIMIT 1;";
    std::vector<std::string> params;
    params.push_back(taskName);
    std::vector<std::string> rows = pgApi->executeSelect(sqlQuery, params);
    if (rows.empty()) {
        return "";
    }
    return rows[0];
}

std::vector<std::string> PostgresAdapter::fetchAllRecords() {
    if (pgApi == nullptr) {
        return std::vector<std::string>();
    }

    std::string sqlQuery =
        "SELECT task_id, task_name, state, payload FROM " + tableName + " ORDER BY id ASC;";

    return pgApi->executeSelect(sqlQuery);
}

bool PostgresAdapter::isConnected() {
    if (pgApi == nullptr) {
        return false;
    }
    if (pgApi->getConnectionStatus()) {
        return true;
    }
    return pgApi->connect();
}