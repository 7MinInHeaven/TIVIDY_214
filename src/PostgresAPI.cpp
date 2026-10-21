#include "PostgresAPI.h"
#include <libpq-fe.h>
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
    PGconn* conn = PQconnectdb(connString.c_str());
    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "[PostgresAPI::connect Error] " << PQerrorMessage(conn) << std::endl;
        PQfinish(conn);
        connected = false;
        return false;
    }
    PQfinish(conn);
    connected = true;
    return true;
}

void PostgresAPI::disconnect() {
    connected = false;
}

int PostgresAPI::executeTransaction(std::string sqlQuery) {
    if (connString.empty()) {
        return -1;
    }
    PGconn* conn = PQconnectdb(connString.c_str());
    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "[PostgresAPI::executeTransaction Connection Error] " << PQerrorMessage(conn) << std::endl;
        PQfinish(conn);
        return -1;
    }

    PGresult* res = PQexec(conn, sqlQuery.c_str());
    ExecStatusType status = PQresultStatus(res);

    if (status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK) {
        std::cerr << "[PostgresAPI::executeTransaction Query Error] " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        PQfinish(conn);
        return -1;
    }

    PQclear(res);
    PQfinish(conn);
    connected = true;
    return 0; // 0 = SQL success code
}

std::vector<std::string> PostgresAPI::executeSelect(std::string sqlQuery) {
    std::vector<std::string> results;
    if (connString.empty()) {
        return results;
    }

    PGconn* conn = PQconnectdb(connString.c_str());
    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "[PostgresAPI::executeSelect Connection Error] " << PQerrorMessage(conn) << std::endl;
        PQfinish(conn);
        return results;
    }

    PGresult* res = PQexec(conn, sqlQuery.c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "[PostgresAPI::executeSelect Query Error] " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        PQfinish(conn);
        return results;
    }

    int rows = PQntuples(res);
    int cols = PQnfields(res);

    for (int r = 0; r < rows; ++r) {
        std::string rowStr = "";
        for (int c = 0; c < cols; ++c) {
            if (c > 0) {
                rowStr += " | ";
            }
            const char* val = PQgetvalue(res, r, c);
            if (val != nullptr) {
                rowStr += val;
            }
        }
        results.push_back(rowStr);
    }

    PQclear(res);
    PQfinish(conn);
    connected = true;
    return results;
}

bool PostgresAPI::getConnectionStatus() const {
    return connected;
}

std::string PostgresAPI::getConnString() const {
    return connString;
}