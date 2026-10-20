#include "PostgresAPI.h"
#include <pqxx/pqxx>
#include <iostream>
#include <cstdlib>

PostgresAPI::PostgresAPI(std::string connStr) : connString(connStr), connected(false) {
    if (this->connString.empty()) {
        const char* envUrl = std::getenv("DATABASE_URL");
        if (envUrl != nullptr) {
            this->connString = std::string(envUrl);
        }
    }
}

PostgresAPI::~PostgresAPI() {
    disconnect();
}

bool PostgresAPI::connect() {
    if (connString.empty()) {
        connected = false;
        return false;
    }
    try {
        pqxx::connection conn(connString);
        connected = conn.is_open();
        return connected;
    } catch (const std::exception& e) {
        std::cerr << "[PostgresAPI::connect Error] " << e.what() << std::endl;
        connected = false;
        return false;
    }
}

void PostgresAPI::disconnect() {
    connected = false;
}

int PostgresAPI::executeTransaction(std::string sqlQuery) {
    if (connString.empty()) {
        return -1;
    }
    try {
        pqxx::connection conn(connString);
        pqxx::work txn(conn);
        txn.exec(sqlQuery);
        txn.commit();
        connected = true;
        return 0; // 0 = SQL success code
    } catch (const std::exception& e) {
        std::cerr << "[PostgresAPI::executeTransaction Error] " << e.what() << std::endl;
        return -1; // -1 = SQL error code
    }
}

std::vector<std::string> PostgresAPI::executeSelect(std::string sqlQuery) {
    std::vector<std::string> results;
    if (connString.empty()) {
        return results;
    }
    try {
        pqxx::connection conn(connString);
        pqxx::nontransaction ntx(conn);
        pqxx::result res = ntx.exec(sqlQuery);

        for (pqxx::result::const_iterator row = res.begin(); row != res.end(); ++row) {
            std::string rowStr = "";
            for (pqxx::row::const_iterator field = row->begin(); field != row->end(); ++field) {
                if (field != row->begin()) {
                    rowStr += " | ";
                }
                rowStr += field->c_str();
            }
            results.push_back(rowStr);
        }
        connected = true;
    } catch (const std::exception& e) {
        std::cerr << "[PostgresAPI::executeSelect Error] " << e.what() << std::endl;
    }
    return results;
}

bool PostgresAPI::getConnectionStatus() const {
    return connected;
}

std::string PostgresAPI::getConnString() const {
    return connString;
}