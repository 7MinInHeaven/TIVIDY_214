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

TEST(AdapterStrategyTest, LiveNeonPostgresInsertWhenEnvConfigured) {
    // 1. Save workflow tasks locally into SQLite (tividy_local.db)
    SQLiteAdapter* localSq = new SQLiteAdapter(new SQLiteAPI("tividy_local.db"), "workflow_audit_log", true);
    DatabaseObserver localObserver(nullptr, localSq);

    WorkUnit localTaskA("Verify Policy");
    WorkUnit localTaskB("Approve Payout");
    localTaskA.setData("task_id", "777");
    localTaskB.setData("task_id", "778");
    localTaskA.attach(&localObserver);
    localTaskB.attach(&localObserver);

    localTaskA.setState(new InProgressState()); // Saves "Verify Policy | In Progress" to SQLite
    localTaskB.setState(new CompletedState());  // Saves "Approve Payout | Completed" to SQLite

    // 2. If DATABASE_URL is configured (.env), test Cloud Postgres AND bidirectional sync!
    const char* envUrl = std::getenv("DATABASE_URL");
    if (envUrl == nullptr || std::string(envUrl).empty()) {
        SUCCEED();
        return;
    }

    PostgresAdapter* cloudPg = new PostgresAdapter(new PostgresAPI(envUrl), "workflow_audit_log", true);
    WorkUnit liveTask("Live Neon Cloud Test");
    liveTask.setData("task_id", "777");
    liveTask.setData("category", "AccidentalClaim");
    liveTask.setData("amount", "R300000");

    DatabaseObserver liveObserver(nullptr, cloudPg);
    liveTask.attach(&liveObserver);

    liveTask.setState(new InProgressState()); // Saves "In Progress" directly to Neon
    EXPECT_EQ(liveObserver.getLastSyncedState(), "In Progress");

    liveTask.setState(new CompletedState());  // Saves "Completed" directly to Neon
    EXPECT_EQ(liveObserver.getLastSyncedState(), "Completed");

   
    //    - Upload missing local SQLite rows -> Neon Cloud
    //    - Download missing Neon Cloud rows -> local SQLite
    liveObserver.syncDatabases(localSq, cloudPg);
    liveObserver.syncDatabases(cloudPg, localSq);

    EXPECT_GE(localSq->fetchAllRecords().size(), 4u);
    EXPECT_GE(cloudPg->fetchAllRecords().size(), 4u);
}

TEST(AdapterStrategyTest, SingleObserverWatchesMultipleWorkComponentsAutomatically) {
    const std::string testFile = "test_multi_subject.db";
    std::remove(testFile.c_str());

    // 1. Create ONE DatabaseObserver (starts with subject = nullptr)
    SQLiteAdapter* localSq = new SQLiteAdapter(new SQLiteAPI(testFile), "workflow_audit_log", true);
    DatabaseObserver singleObserver(nullptr, localSq);

    // 2. Create two separate WorkUnits and attach the SAME singleObserver to both!
    WorkUnit taskA("Verify Policy");
    WorkUnit taskB("Approve Payout");
    taskA.attach(&singleObserver);
    taskB.attach(&singleObserver);

    // 3. Change state on taskA and taskB (we never manually call setSubject or update!)
    taskA.setState(new InProgressState());
    taskB.setState(new CompletedState());

    // 4. Both tasks were automatically saved to the database by that ONE observer!
    EXPECT_EQ(localSq->getRecordState("Verify Policy"), "In Progress");
    EXPECT_EQ(localSq->getRecordState("Approve Payout"), "Completed");

    std::remove(testFile.c_str());
}