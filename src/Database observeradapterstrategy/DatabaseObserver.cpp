#include "DatabaseObserver.h"
#include <sstream>
#include <cstdlib>

DatabaseObserver::DatabaseObserver(WorkComponent* subj, ExternalDatabase* db, ExternalDatabase* fallbackDb): subject(subj), database(db), fallbackDatabase(fallbackDb), lastSyncedState("") {}

DatabaseObserver::~DatabaseObserver() {
    if (fallbackDatabase != nullptr && fallbackDatabase != database) {
        delete fallbackDatabase;
        fallbackDatabase = nullptr;
    }
    delete database;
    database = nullptr;
}

void DatabaseObserver::setSubject(WorkComponent* s) {
    subject = s;
}

void DatabaseObserver::setDatabaseStrategy(ExternalDatabase* db) {
     if (database != nullptr && database != db) {
        if (database->isConnected() && db != nullptr && db->isConnected()) {
            syncDatabases(database, db);
            syncDatabases(db, database);
        }
        if (database != fallbackDatabase) {
            delete database;
        }
    }
    database = db;
}

std::string DatabaseObserver::getLastSyncedState() const {
    return lastSyncedState;
}

ExternalDatabase* DatabaseObserver::selectBestStrategy() const {
    if (database != nullptr && database->isConnected()) {
        return database;
    }
    if (fallbackDatabase != nullptr && fallbackDatabase->isConnected()) {
        return fallbackDatabase;
    }
    return database;
}

void DatabaseObserver::update() {
    if (subject == nullptr) {
        return;
    }

    ExternalDatabase* activeDb = selectBestStrategy();
    if (activeDb == nullptr) {
        return;
    }

    std::string taskName = subject->getName();

    std::string stateStr = "Pending";
    if (subject->getState() != nullptr) {
        stateStr = subject->getState()->getStatus();
    }

    int taskID = 1;
    std::string payloadStr = "";
    WorkItem* item = subject->getItem();
    if (item != nullptr) {
        if (item->hasData("task_id")) {
            taskID = std::atoi(item->getData("task_id").c_str());
        }
        const std::map<std::string, std::string>& dataMap = item->getPayloadData();
        for (std::map<std::string, std::string>::const_iterator it = dataMap.begin(); it != dataMap.end(); ++it) {
            if (!payloadStr.empty()) {
                payloadStr += ";";
            }
            payloadStr += it->first + "=" + it->second;
        }
    }

    if (activeDb->saveRecord(taskID, taskName, stateStr, payloadStr)) {
        lastSyncedState = stateStr;
    }
}

int DatabaseObserver::syncDatabases(ExternalDatabase* fromDb, ExternalDatabase* toDb) {
    if (fromDb == nullptr || toDb == nullptr || !toDb->isConnected()) {
        return 0;
    }

    std::vector<std::string> sourceRecords = fromDb->fetchAllRecords();
    std::vector<std::string> existingTargetRecords = toDb->fetchAllRecords();
    int syncedCount = 0;

    for (size_t i = 0; i < sourceRecords.size(); ++i) {
        const std::string& row = sourceRecords[i];

        // skip if this exact record already exists in the target database
        bool alreadyExists = false;
        for (size_t j = 0; j < existingTargetRecords.size(); ++j) {
            if (existingTargetRecords[j] == row) {
                alreadyExists = true;
                break;
            }
        }
        if (alreadyExists) continue;

        size_t p1 = row.find(" | ");
        if (p1 == std::string::npos) continue;
        size_t p2 = row.find(" | ", p1 + 3);
        if (p2 == std::string::npos) continue;
        size_t p3 = row.find(" | ", p2 + 3);
        if (p3 == std::string::npos) continue;

        int taskID = std::atoi(row.substr(0, p1).c_str());
        std::string taskName = row.substr(p1 + 3, p2 - (p1 + 3));
        std::string state = row.substr(p2 + 3, p3 - (p2 + 3));
        std::string payload = row.substr(p3 + 3);

        if (toDb->saveRecord(taskID, taskName, state, payload)) {
            existingTargetRecords.push_back(row);
            syncedCount++;
        }
    }

    return syncedCount;
}