#include <gtest/gtest.h>
#include <cstdio>
#include <cstdlib>
#include "ExternalDatabase.h"
#include "SQLiteAPI.h"
#include "SQLiteAdapter.h"
#include "PostgresAPI.h"
#include "PostgresAdapter.h"
#include "DatabaseObserver.h"
#include "Composite.h"


TEST(AdapterStrategyTest, SQLiteAdapterSavesAndFetchesRecords) {
    const std::string testDbFile = "test_tividy_unit.db";
    std::remove(testDbFile.c_str());

    ExternalDatabase* db = new SQLiteAdapter(new SQLiteAPI(testDbFile), "workflow_audit_log", true);
    EXPECT_TRUE(db->isConnected());

    // save initial state
    EXPECT_TRUE(db->saveRecord(101, "Review Policy", "Pending", "Category=DeathBenefit"));
    EXPECT_EQ(db->getRecordState("Review Policy"), "Pending");

    // save updated state for the same task
    EXPECT_TRUE(db->saveRecord(101, "Review Policy", "Completed", "Approved payout R150000"));
    EXPECT_EQ(db->getRecordState("Review Policy"), "Completed");

    std::vector<std::string> allRows = db->fetchAllRecords();
    EXPECT_EQ(allRows.size(), 2u);

    delete db;
    std::remove(testDbFile.c_str());
}

TEST(AdapterStrategyTest, SQLitePreparedStatementsPreventSQLInjection) {
    const std::string testDbFile = "test_tividy_sqli.db";
    std::remove(testDbFile.c_str());

    SQLiteAdapter db(new SQLiteAPI(testDbFile), "workflow_audit_log", true);

    std::string maliciousPayload = "Bob's claim'); DROP TABLE workflow_audit_log; --";
    EXPECT_TRUE(db.saveRecord(202, "Verify Claim", "InProgress", maliciousPayload));

    // table must remain intact and return the state
    EXPECT_EQ(db.getRecordState("Verify Claim"), "InProgress");
    EXPECT_EQ(db.fetchAllRecords().size(), 1u);

    std::remove(testDbFile.c_str());
}

//postgres adapter tests 

class MockPostgresAPI : public PostgresAPI {
public:
    bool online = true;
    std::string lastQuery;
    std::vector<std::string> lastParams;
    std::vector<std::string> storedRows;

    MockPostgresAPI(bool isOnline = true) : PostgresAPI(""), online(isOnline) {}

    bool connect() override { return online; }
    bool getConnectionStatus() const override { return online; }

    int executeTransaction(std::string sqlQuery, const std::vector<std::string>& params) override {
        if (!online) return -1;
        lastQuery = sqlQuery;
        lastParams = params;
        if (params.size() == 4u) {
            storedRows.push_back(params[0] + " | " + params[1] + " | " + params[2] + " | " + params[3]);
        }
        return 0;
    }

    std::vector<std::string> executeSelect(std::string sqlQuery, const std::vector<std::string>& params) override {
        lastQuery = sqlQuery;
        lastParams = params;
        if (!online) return std::vector<std::string>();

        if (!params.empty()) {
            std::vector<std::string> stateResult;
            for (int i = static_cast<int>(storedRows.size()) - 1; i >= 0; --i) {
                if (storedRows[i].find(" | " + params[0] + " | ") != std::string::npos) {

                    size_t p1 = storedRows[i].find(" | ");
                    size_t p2 = storedRows[i].find(" | ", p1 + 3);
                    size_t p3 = storedRows[i].find(" | ", p2 + 3);
                    stateResult.push_back(storedRows[i].substr(p2 + 3, p3 - (p2 + 3)));
                    break;
                }
            }
            return stateResult;
        }
        return storedRows;
    }
};

TEST(AdapterStrategyTest, PostgresAdapterTranslatesToParameterizedSQL) {
    MockPostgresAPI* mockApi = new MockPostgresAPI(true);
    ExternalDatabase* db = new PostgresAdapter(mockApi, "workflow_audit_log", true);

    EXPECT_TRUE(db->isConnected());
    EXPECT_TRUE(db->saveRecord(303, "Approve Payout", "Completed", "Amount=R300000"));

    EXPECT_NE(mockApi->lastQuery.find("VALUES ($1, $2, $3, $4)"), std::string::npos);
    ASSERT_EQ(mockApi->lastParams.size(), 4u);
    EXPECT_EQ(mockApi->lastParams[0], "303");
    EXPECT_EQ(mockApi->lastParams[1], "Approve Payout");
    EXPECT_EQ(mockApi->lastParams[2], "Completed");
    EXPECT_EQ(mockApi->lastParams[3], "Amount=R300000");

    EXPECT_EQ(db->getRecordState("Approve Payout"), "Completed");

    delete db;
}


//database observer and strategy context and fallback and sync tests

TEST(AdapterStrategyTest, DatabaseObserverAutomaticFallbackAndSync) {
    const std::string fallbackFile = "test_fallback.db";
    std::remove(fallbackFile.c_str());

    // Cloud Postgres is OFFLINE (false), Fallback SQLite is ONLINE
    MockPostgresAPI* cloudApi = new MockPostgresAPI(false);
    ExternalDatabase* cloudPg = new PostgresAdapter(cloudApi, "workflow_audit_log", true);
    ExternalDatabase* localSq = new SQLiteAdapter(new SQLiteAPI(fallbackFile), "workflow_audit_log", true);

    WorkUnit task("Verify Medical Report");
    task.setData("task_id", "404");
    task.setData("category", "CriticalIllness");

    DatabaseObserver observer(&task, cloudPg, localSq);

    // trigger update() while cloud is offline, should automatically save to localSql
    EXPECT_EQ(observer.selectBestStrategy(), localSq);
    observer.update();

    // record was saved into local SQLite fallback database
    EXPECT_EQ(observer.getLastSyncedState(), "Pending");
    EXPECT_EQ(localSq->getRecordState("Verify Medical Report"), "Pending");
    EXPECT_TRUE(cloudPg->fetchAllRecords().empty());

    // Cloud comes back ONLINE, syncDatabases() migrates local SQLite records to Cloud Postgres
    cloudApi->online = true;
    EXPECT_EQ(observer.selectBestStrategy(), cloudPg);

    int synced = observer.syncDatabases(localSq, cloudPg);
    EXPECT_EQ(synced, 1);
    EXPECT_EQ(cloudPg->getRecordState("Verify Medical Report"), "Pending");

    std::remove(fallbackFile.c_str());
}

TEST(AdapterStrategyTest, HandlesNullPointersSafely) {
    PostgresAdapter nullPg(nullptr, "workflow_audit_log", false);
    SQLiteAdapter nullSq(nullptr, "workflow_audit_log", false);

    EXPECT_FALSE(nullPg.isConnected());
    EXPECT_FALSE(nullPg.saveRecord(1, "Task", "Pending", "Data"));
    EXPECT_EQ(nullPg.getRecordState("Task"), "");
    EXPECT_TRUE(nullPg.fetchAllRecords().empty());

    EXPECT_FALSE(nullSq.isConnected());
    EXPECT_FALSE(nullSq.saveRecord(1, "Task", "Pending", "Data"));
    EXPECT_EQ(nullSq.getRecordState("Task"), "");
    EXPECT_TRUE(nullSq.fetchAllRecords().empty());
}