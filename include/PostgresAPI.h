#ifndef POSTGRESAPI_H
#define POSTGRESAPI_H

#include <string>
#include <vector>

class PostgresAPI {
private:
    std::string connString;
    bool connected;

public:
    PostgresAPI(std::string connStr = "");
    virtual ~PostgresAPI();

    virtual bool connect();
    virtual void disconnect();
<<<<<<< HEAD
    virtual int executeTransaction(std::string sqlQuery, const std::vector<std::string>& params = std::vector<std::string>());
    virtual std::vector<std::string> executeSelect(std::string sqlQuery, const std::vector<std::string>& params = std::vector<std::string>());
=======
    virtual int executeTransaction(std::string sqlQuery);
    virtual std::vector<std::string> executeSelect(std::string sqlQuery);
>>>>>>> aa798df (completed PostgresAPI implentation and header)
    virtual bool getConnectionStatus() const;
    std::string getConnString() const;
};

#endif