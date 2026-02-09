#include "databasemanager.h"
#include <QSqlQuery>
#include <QMutexLocker>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager() {
}

DatabaseManager::~DatabaseManager() {
    closeConnection();
}

bool DatabaseManager::initialize(const DatabaseConfig& config) {
    QMutexLocker locker(&m_mutex);

    if (!config.isValid()) {
        m_lastError = "Invalid database configuration";
        return false;
    }

    // Close existing connection if any
    if (QSqlDatabase::contains(config.connectionName)) {
        QSqlDatabase::removeDatabase(config.connectionName);
    }

    QSqlDatabase db = createConnection(config, config.connectionName);
    if (!db.isValid()) {
        m_lastError = "Failed to create database connection";
        return false;
    }

    setupConnection(db, config);

    if (!db.open()) {
        m_lastError = db.lastError().text();
        QSqlDatabase::removeDatabase(config.connectionName);
        return false;
    }

    m_currentConfig = config;
    m_lastError.clear();
    return true;
}

bool DatabaseManager::testConnection(const DatabaseConfig& config, QString& errorMessage) {
    if (!config.isValid()) {
        errorMessage = "Invalid database configuration";
        return false;
    }

    QString testConnectionName = "test_connection_" + QString::number(QDateTime::currentMSecsSinceEpoch());

    QSqlDatabase testDb = createConnection(config, testConnectionName);
    if (!testDb.isValid()) {
        errorMessage = "Failed to create test database connection";
        return false;
    }

    setupConnection(testDb, config);

    bool success = testDb.open();
    if (!success) {
        errorMessage = testDb.lastError().text();
    } else {
        errorMessage = "Connection successful!";
    }

    testDb.close();
    QSqlDatabase::removeDatabase(testConnectionName);

    return success;
}

void DatabaseManager::closeConnection() {
    QMutexLocker locker(&m_mutex);

    if (QSqlDatabase::contains(m_currentConfig.connectionName)) {
        QSqlDatabase db = QSqlDatabase::database(m_currentConfig.connectionName);
        if (db.isOpen()) {
            db.close();
        }
        QSqlDatabase::removeDatabase(m_currentConfig.connectionName);
    }
}

QSqlDatabase DatabaseManager::getDatabase() const {
    if (QSqlDatabase::contains(m_currentConfig.connectionName)) {
        return QSqlDatabase::database(m_currentConfig.connectionName);
    }
    return QSqlDatabase();
}

bool DatabaseManager::isConnected() const {
    if (QSqlDatabase::contains(m_currentConfig.connectionName)) {
        QSqlDatabase db = QSqlDatabase::database(m_currentConfig.connectionName);
        return db.isValid() && db.isOpen();
    }
    return false;
}

DatabaseConfig DatabaseManager::getCurrentConfig() const {
    return m_currentConfig;
}

QString DatabaseManager::getLastError() const {
    return m_lastError;
}

bool DatabaseManager::createSampleTable() {
    if (!isConnected()) {
        m_lastError = "Not connected to database";
        return false;
    }

    QSqlDatabase db = getDatabase();
    QSqlQuery query(db);

    QString createTableQuery;
    switch (m_currentConfig.type) {
        case DatabaseConfig::SQLite:
            createTableQuery =
                "CREATE TABLE IF NOT EXISTS sample_data ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                "name TEXT NOT NULL, "
                "value TEXT, "
                "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
                ")";
            break;
        case DatabaseConfig::MySQL:
            createTableQuery =
                "CREATE TABLE IF NOT EXISTS sample_data ("
                "id INT AUTO_INCREMENT PRIMARY KEY, "
                "name VARCHAR(255) NOT NULL, "
                "value TEXT, "
                "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
                ")";
            break;
        case DatabaseConfig::PostgreSQL:
            createTableQuery =
                "CREATE TABLE IF NOT EXISTS sample_data ("
                "id SERIAL PRIMARY KEY, "
                "name VARCHAR(255) NOT NULL, "
                "value TEXT, "
                "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
                ")";
            break;
        default:
            createTableQuery =
                "CREATE TABLE sample_data ("
                "id INT PRIMARY KEY, "
                "name VARCHAR(255) NOT NULL, "
                "value VARCHAR(MAX), "
                "created_at DATETIME DEFAULT GETDATE()"
                ")";
    }

    if (!query.exec(createTableQuery)) {
        m_lastError = query.lastError().text();
        return false;
    }

    return true;
}

QString DatabaseManager::getDatabaseInfo() const {
    if (!isConnected()) {
        return "Not connected";
    }

    QSqlDatabase db = getDatabase();
    QString info;
    info += "Database Type: " + m_currentConfig.getTypeName() + "\n";
    info += "Driver: " + db.driverName() + "\n";
    info += "Database Name: " + db.databaseName() + "\n";

    if (m_currentConfig.type != DatabaseConfig::SQLite) {
        info += "Host: " + db.hostName() + "\n";
        info += "Port: " + QString::number(db.port()) + "\n";
        info += "Username: " + db.userName() + "\n";
    }

    info += "Connection Name: " + db.connectionName() + "\n";
    info += "Status: " + QString(db.isOpen() ? "Connected" : "Disconnected");

    return info;
}

QSqlDatabase DatabaseManager::createConnection(const DatabaseConfig& config, const QString& connectionName) {
    QSqlDatabase db = QSqlDatabase::addDatabase(config.getDriverName(), connectionName);
    return db;
}

QStringList DatabaseManager::getTables() const {
    QStringList tables;
    if (!isConnected()) {
        return tables;
    }

    QSqlDatabase db = getDatabase();
    tables = db.tables(QSql::Tables);
    return tables;
}

void DatabaseManager::setupConnection(QSqlDatabase& db, const DatabaseConfig& config) {
    if (config.type == DatabaseConfig::SQLite) {
        // For SQLite, ensure directory exists
        QFileInfo fileInfo(config.name);
        QDir dir = fileInfo.dir();
        if (!dir.exists()) {
            dir.mkpath(".");
        }
        db.setDatabaseName(config.name);
    } else {
        db.setHostName(config.host);
        db.setPort(config.port);
        db.setDatabaseName(config.name);
        db.setUserName(config.username);
        db.setPassword(config.password);

        // Set connection options (format varies by database driver)
        QStringList options;

        if (config.type == DatabaseConfig::PostgreSQL) {
            // PostgreSQL uses lowercase option names
            if (config.connectionTimeout > 0) {
                options << QString("connect_timeout=%1").arg(config.connectionTimeout);
            }
            if (config.useSSL) {
                options << "sslmode=require";
            }
        } else {
            // MySQL and others
            if (!config.charset.isEmpty()) {
                options << QString("CHARSET=%1").arg(config.charset);
            }
            if (config.useSSL) {
                options << "SSL_ENABLE=1";
            }
            if (config.connectionTimeout > 0) {
                options << QString("CONNECT_TIMEOUT=%1").arg(config.connectionTimeout);
            }
        }

        if (!options.isEmpty()) {
            db.setConnectOptions(options.join(";"));
        }
    }
}
