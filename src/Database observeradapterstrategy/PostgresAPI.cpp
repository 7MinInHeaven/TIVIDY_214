#include "PostgresAPI.h"
#include <libpq-fe.h>
#include <iostream>
#include <cstdlib>

static void ignoreNotice(void*, const PGresult*) {}

PostgresAPI::PostgresAPI(std::string connStr)
    : connString(connStr), connected(false), dbConn(nullptr) {
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

    if (dbConn != nullptr) {
        PGconn* existing = static_cast<PGconn*>(dbConn);
        if (PQstatus(existing) == CONNECTION_OK) {
            connected = true;
            return true;
        }
        PQfinish(existing);
        dbConn = nullptr;
    }

    PGconn* conn = PQconnectdb(connString.c_str());
    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "[PostgresAPI::connect Error] " << PQerrorMessage(conn) << std::endl;
        PQfinish(conn);
        connected = false;
        return false;
    }

    PQsetNoticeReceiver(conn, ignoreNotice, nullptr);
    dbConn = conn;
    connected = true;
    return true;
}

void PostgresAPI::disconnect() {
    if (dbConn != nullptr) {
        PQfinish(static_cast<PGconn*>(dbConn));
        dbConn = nullptr;
    }
    connected = false;
}

int PostgresAPI::executeTransaction(std::string sqlQuery, const std::vector<std::string>& params) {
    if (!connect()) {
        return -1;
    }

    PGconn* conn = static_cast<PGconn*>(dbConn);
    PGresult* res = nullptr;
    if (params.empty()) {
        res = PQexec(conn, sqlQuery.c_str());
    } else {
        std::vector<const char*> paramValues;
        for (size_t i = 0; i < params.size(); ++i) {
            paramValues.push_back(params[i].c_str());
        }
        res = PQexecParams(
            conn,
            sqlQuery.c_str(),
            static_cast<int>(params.size()),
            nullptr,
            paramValues.data(),
            nullptr,
            nullptr,
            0
        );
    }

    ExecStatusType status = PQresultStatus(res);
    if (status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK) {
        std::cerr << "[PostgresAPI::executeTransaction Query Error] " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return -1;
    }

    PQclear(res);
    return 0;
}

std::vector<std::string> PostgresAPI::executeSelect(std::string sqlQuery, const std::vector<std::string>& params) {
    std::vector<std::string> results;
    if (!connect()) {
        return results;
    }

    PGconn* conn = static_cast<PGconn*>(dbConn);
    PGresult* res = nullptr;
    if (params.empty()) {
        res = PQexec(conn, sqlQuery.c_str());
    } else {
        std::vector<const char*> paramValues;
        for (size_t i = 0; i < params.size(); ++i) {
            paramValues.push_back(params[i].c_str());
        }
        res = PQexecParams(
            conn,
            sqlQuery.c_str(),
            static_cast<int>(params.size()),
            nullptr,
            paramValues.data(),
            nullptr,
            nullptr,
            0
        );
    }

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "[PostgresAPI::executeSelect Query Error] " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
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
    return results;
}

bool PostgresAPI::getConnectionStatus() const {
    return connected;
}

std::string PostgresAPI::getConnString() const {
    return connString;
}