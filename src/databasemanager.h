#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QString>
#include <QMutex>
#include "databaseconfig.h"

class DatabaseManager {
public:
    static DatabaseManager& instance();

    // Delete copy constructor and assignment operator
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    bool initialize(const DatabaseConfig& config);
    bool testConnection(const DatabaseConfig& config, QString& errorMessage);
    void closeConnection();

    QSqlDatabase getDatabase() const;

    bool isConnected() const;

    DatabaseConfig getCurrentConfig() const;

    QString getLastError() const;

    bool createSampleTable();
    
    QString getDatabaseInfo() const;
    QStringList getTables() const;

private:
    DatabaseManager();
    ~DatabaseManager();

    QSqlDatabase createConnection(const DatabaseConfig& config, const QString& connectionName);
    void setupConnection(QSqlDatabase& db, const DatabaseConfig& config);

    DatabaseConfig m_currentConfig;
    QString m_lastError;
    mutable QMutex m_mutex;
};

#endif // DATABASEMANAGER_H
