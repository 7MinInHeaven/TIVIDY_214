#include "PostgresAdapter.h"

PostgresAdapter::PostgresAdapter(PostgresAPI* pgApi, std::string table, bool takeOwnership)
    : api(pgApi), tableName(table), ownsApi(takeOwnership) {}

PostgresAdapter::~PostgresAdapter() {
    if (ownsApi && api != nullptr) {
        delete api;
        api = nullptr;
    }
}

std::string PostgresAdapter::rmvQuotes(const std::string& input) const {
    std::string sanitized = "";
    for (size_t i = 0; i < input.length(); ++i) {
        if (input[i] == '\'') {
            sanitized += "''"; // Escape single quote for SQL safety
        } else {
            sanitized += input[i];
        }
    }
    return sanitized;
}

bool PostgresAdapter::saveRecord(int taskID, std::string taskName, std::string state, std::string payload) {
    if (api == nullptr) {
        return false;
    }

    std::string safeName = sanitizeInput(taskName);
    std::string safeState = sanitizeInput(state);
    std::string safePayload = sanitizeInput(payload);

    std::string sqlQuery =
        "INSERT INTO " + tableName + " (task_id, task_name, state, payload) VALUES (" +
        std::to_string(taskID) + ", '" +
        safeName + "', '" +
        safeState + "', '" +
        safePayload + "');";

    int statusCode = api->executeTransaction(sqlQuery);
    return (statusCode == 0);
}

std::string PostgresAdapter::getRecordState(std::string taskName) {
    if (api == nullptr) {
        return "";
    }

    std::string safeName = sanitizeInput(taskName);
    std::string sqlQuery =
        "SELECT state FROM " + tableName +
        " WHERE task_name = '" + safeName + "' ORDER BY id DESC LIMIT 1;";

    std::vector<std::string> rows = api->executeSelect(sqlQuery);
    if (rows.empty()) {
        return "";
    }
    return rows[0];
}

std::vector<std::string> PostgresAdapter::fetchAllRecords() {
    if (api == nullptr) {
        return std::vector<std::string>();
    }

    std::string sqlQuery =
        "SELECT task_id, task_name, state, payload FROM " + tableName + " ORDER BY id ASC;";

    return api->executeSelect(sqlQuery);
}

bool PostgresAdapter::isConnected() {
    if (api == nullptr) {
        return false;
    }
    if (api->getConnectionStatus()) {
        return true;
    }
    return api->connect();
}