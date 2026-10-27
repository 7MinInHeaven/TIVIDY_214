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
    if (sqlite3_open(dbFilePath.c_str(), &db) != SQLITE_OK) {
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
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, initTableSql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "[SQLiteAPI Prepare Error] " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        connected = false;
        return false;
    }
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    if (rc != SQLITE_DONE) {
        connected = false;
        return false;
    }
    connected = true;
    return true;
}

void SQLiteAPI::closeConnection() {
    connected = false;
}

int SQLiteAPI::runStatement(std::string sqlQuery, const std::vector<std::string>& params) {
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
    //compile the SQL template
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sqlQuery.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "[SQLiteAPI::runStatement Prepare Error] " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return -1;
    }
    //bind each parameter to the ? placeholders
    for (size_t i = 0; i < params.size(); ++i) {
        sqlite3_bind_text(stmt, static_cast<int>(i + 1), params[i].c_str(), -1, SQLITE_TRANSIENT);
    }
    //execute prepared statement
    int rc = sqlite3_step(stmt);
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
        return -1;
    }
    connected = true;
    return 0;
}
std::vector<std::string> SQLiteAPI::queryRows(std::string sqlQuery, const std::vector<std::string>& params) {
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
    //prepare SELECT query
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sqlQuery.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "[SQLiteAPI::queryRows Prepare Error] " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return results;
    }
    //bind WHERE clause parameters to ? placeholders
    for (size_t i = 0; i < params.size(); ++i) {
        sqlite3_bind_text(stmt, static_cast<int>(i + 1), params[i].c_str(), -1, SQLITE_TRANSIENT);
    }
    
    int cols = sqlite3_column_count(stmt);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string rowStr = "";
        for (int c = 0; c < cols; ++c) {
            if (c > 0) {
                rowStr += " | ";
            }
            const unsigned char* val = sqlite3_column_text(stmt, c);
            if (val != nullptr) {
                rowStr += reinterpret_cast<const char*>(val);
            }
        }
        results.push_back(rowStr);
    }
    
    sqlite3_finalize(stmt);
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