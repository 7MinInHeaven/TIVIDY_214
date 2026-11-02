/**
 * @file DatabaseObserver.h
 * @brief Declares the ConcreteObserver and Strategy Context for database persistence
 */
#ifndef DATABASEOBSERVER_H
#define DATABASEOBSERVER_H

#include "WorkflowObserver.h"
#include "ExternalDatabase.h"
#include "Composite.h"
#include <string>

/**
 * @class DatabaseObserver
 * @brief ConcreteObserver in the Observer pattern and Context in the Strategy pattern
 *
 * Observes WorkComponent state/data transitions and persists audit records using an
 * interchangeable ExternalDatabase strategy (PostgresAdapter for cloud storage
 * or SQLiteAdapter for local fallback storage). Also supports database
 * synchronization between two ExternalDatabase strategies.
 *
 * @see WorkflowObserver
 * @see ExternalDatabase
 * @see PostgresAdapter
 * @see SQLiteAdapter
 */

class DatabaseObserver : public WorkflowObserver {
private:
    /**< Currently active WorkComponent subject being observed. */
    WorkComponent* subject;
     /**< Main database strategy (composition relationship). */
    ExternalDatabase* database;
    /**< Optional fallback database strategy when main is offline (Composition relationship). */
    ExternalDatabase* fallbackDatabase;
    /**< Saves the most recently updated workflow state name. */
    std::string lastSyncedState;

public:
     /**
     * @brief Constructs a DatabaseObserver with a subject, main strategy, and optional fallback strategy
     *
     * @param[in] subj Pointer to the initial WorkComponent subject (may be nullptr when observing multiple subjects)
     * @param[in] db Primary ExternalDatabase strategy
     * @param[in] fallbackDb Optional fallback ExternalDatabase strategy used if primary is disconnected
     */
    DatabaseObserver(WorkComponent* subj, ExternalDatabase* db, ExternalDatabase* fallbackDb = nullptr);

    /**
     * @brief Destructor that cleans up owned primary and fallback ExternalDatabase strategies
     */
    virtual ~DatabaseObserver();

    /**
     * @brief Pulls the current state and payload from the active WorkComponent subject and saves it via the active strategy
     */
    virtual void update() override;

    /**
     * @brief Returns the state string recorded during the most recent successful update()
     *
     * @return Last synced state name, or an empty string if no state has been synced yet.
     */
    std::string getLastSyncedState() const;

     /**
     * @brief Sets the active WorkComponent subject to pull state from on the next update()
     *
     * @param[in] s Pointer to the WorkComponent subject.
     */
    void setSubject(WorkComponent* s);

    /**
     * @brief Replaces the primary ExternalDatabase strategy at runtime, deleting the previous strategy
     *
     * @param[in] db New primary ExternalDatabase strategy (ownership transferred).
     */
    void setDatabaseStrategy(ExternalDatabase* db);

    /**
     * @brief Selects the best available database strategy based on live connectivity.
     *
     * @return Pointer to the primary database if connected, otherwise the fallback database if connected, or primary as default
     */
    ExternalDatabase* selectBestStrategy() const;

    /**
     * @brief Synchronizes missing records from a source database strategy into a target database strategy
     *
     * Deduplicates against existing records in the target database so repeated bidirectional syncs do not duplicate rows.
     *
     * @param[in] fromDb Source ExternalDatabase strategy to read records from.
     * @param[in] toDb Target ExternalDatabase strategy to insert missing records into.
     * @return Number of new records transferred from @p fromDb to @p toDb.
     */
    int syncDatabases(ExternalDatabase* fromDb, ExternalDatabase* toDb);
};

#endif