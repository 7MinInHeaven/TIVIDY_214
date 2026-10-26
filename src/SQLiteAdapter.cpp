#include "SQLiteAdapter.h"

SQLiteAdapter::SQLiteAdapter(SQLiteAPI* sqApi, std::string table, bool takeOwnership)
    : api(sqApi), tableName(table), ownsAPI(takeOwnership) {}

SQLiteAdapter::~SQLiteAdapter() {
    if (ownsAPI && api != nullptr) {
        delete api;
        api = nullptr;
    }
}

std::string SQLiteAdapter::rmvQuotes(const std::string& input) const {
    std::string cleaned = "";
    for (size_t i = 0; i < input.length(); ++i) {
        if (input[i] == '\'') {
            cleaned += "''";
        } else {
            cleaned += input[i];
        }
    }
    return cleaned;
}

bool SQLiteAdapter::saveRecord(int taskID, std::string taskName, std::string state, std::string payload) {
    if (api == nullptr) {
        return false;
    }

    std::string safeName = rmvQuotes(taskName);
    std::string safeState = rmvQuotes(state);
    std::string safePayload = rmvQuotes(payload);

    std::string sqlQuery =
        "INSERT INTO " + tableName + " (task_id, task_name, state, payload) VALUES (" +
        std::to_string(taskID) + ", '" +
        safeName + "', '" +
        safeState + "', '" +
        safePayload + "');";

    int status = api->runStatement(sqlQuery);
    return (status == 0);
}

std::string SQLiteAdapter::getRecordState(std::string taskName) {
    if (api == nullptr) {
        return "";
    }

    std::string safeName = rmvQuotes(taskName);
    std::string sqlQuery =
        "SELECT state FROM " + tableName +
        " WHERE task_name = '" + safeName + "' ORDER BY id DESC LIMIT 1;";

    std::vector<std::string> rows = api->queryRows(sqlQuery);
    if (rows.empty()) {
        return "";
    }
    return rows[0];
}

std::vector<std::string> SQLiteAdapter::fetchAllRecords() {
    if (api == nullptr) {
        return std::vector<std::string>();
    }

    std::string sqlQuery =
        "SELECT task_id, task_name, state, payload FROM " + tableName + " ORDER BY id ASC;";

    return api->queryRows(sqlQuery);
}

bool SQLiteAdapter::isConnected() {
    if (api == nullptr) {
        return false;
    }
    if (api->isOpen()) {
        return true;
    }
    return api->openConnection();
}