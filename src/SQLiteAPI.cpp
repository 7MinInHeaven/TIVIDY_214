#include "SQLiteAPI.h"
#include <sqlite3.h>
#include <iostream>

SQLiteAPI::SQLiteAPI(std::string filePath) : dbFilePath(filePath), connected(false) {
    openConnection();
}

SQLiteAPI::~SQLiteAPI() {
    closeConnection();
}

bool SQLiteAPI::openConnection() {
    if (dbFilePath.empty()) {
        connected = false;
        return false;
    }

    sqlite3* db = nullptr;
    int rc = sqlite3_open(dbFilePath.c_str(), &db);
    if (rc != SQLITE_OK) {
        std::cerr << "[SQLiteAPI::openConnection Error] " << sqlite3_errmsg(db) << std::endl;
        if (db != nullptr) {
            sqlite3_close(db);
        }
        connected = false;
        return false;
    }

    const char* initTableSql =
        "CREATE TABLE IF NOT EXISTS workflow_audit_log ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "task_id INTEGER NOT NULL, "
        "task_name TEXT NOT NULL, "
        "state TEXT NOT NULL, "
        "payload TEXT, "
        "updated_at DATETIME DEFAULT CURRENT_TIMESTAMP);";

    char* errMsg = nullptr;
    rc = sqlite3_exec(db, initTableSql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "[SQLiteAPI Table Init Error] " << (errMsg ? errMsg : "") << std::endl;
        sqlite3_free(errMsg);
        sqlite3_close(db);
        connected = false;
        return false;
    }

    sqlite3_close(db);
    connected = true;
    return true;
}

void SQLiteAPI::closeConnection() {
    connected = false;
}

int SQLiteAPI::runStatement(std::string sqlQuery) {
    if (dbFilePath.empty()) {
        return -1;
    }

    sqlite3* db = nullptr;
    if (sqlite3_open(dbFilePath.c_str(), &db) != SQLITE_OK) {
        if (db != nullptr) {
            sqlite3_close(db);
        }
        return -1;
    }

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sqlQuery.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "[SQLiteAPI::runStatement Error] " << (errMsg ? errMsg : "") << std::endl;
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return -1;
    }

    sqlite3_close(db);
    connected = true;
    return 0; // 0 is SQL success code
}

static int selectCallback(void* data, int argc, char** argv, char** /*azColName*/) {
    std::vector<std::string>* rows = static_cast<std::vector<std::string>*>(data);
    std::string rowStr = "";
    for (int i = 0; i < argc; ++i) {
        if (i > 0) {
            rowStr += " | ";
        }
        rowStr += (argv[i] ? argv[i] : "");
    }
    rows->push_back(rowStr);
    return 0;
}

std::vector<std::string> SQLiteAPI::queryRows(std::string sqlQuery) {
    std::vector<std::string> results;
    if (dbFilePath.empty()) {
        return results;
    }

    sqlite3* db = nullptr;
    if (sqlite3_open(dbFilePath.c_str(), &db) != SQLITE_OK) {
        if (db != nullptr) {
            sqlite3_close(db);
        }
        return results;
    }

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sqlQuery.c_str(), selectCallback, &results, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "[SQLiteAPI::queryRows Error] " << (errMsg ? errMsg : "") << std::endl;
        sqlite3_free(errMsg);
    }

    sqlite3_close(db);
    connected = true;
    return results;
}

bool SQLiteAPI::isOpen() const {
    return connected;
}

std::string SQLiteAPI::getDbFilePath() const {
    return dbFilePath;
}