#ifndef DATABASECONFIG_H
#define DATABASECONFIG_H

#include <QString>
#include <QSettings>
#include <QSqlDatabase>

struct DatabaseConfig {
    enum DatabaseType {
        SQLite,
        MySQL,
        PostgreSQL,
        Oracle
    };

    DatabaseType type;
    QString name;           // Database name or file path for SQLite
    QString host;
    int port;
    QString username;
    QString password;
    QString connectionName;

    // Additional options
    QString charset;
    bool useSSL;
    int connectionTimeout;

    DatabaseConfig()
        : type(SQLite)
        , port(0)
        , useSSL(false)
        , connectionTimeout(30)
        , connectionName("default_connection")
    {}

    QString getDriverName() const {
        switch (type) {
            case SQLite:
                return "QSQLITE";
            case MySQL:
                // Try QMYSQL first, fallback to QMARIADB (compatible with MySQL)
                if (QSqlDatabase::isDriverAvailable("QMYSQL") &&
                    QSqlDatabase::drivers().contains("QMYSQL")) {
                    // Check if driver actually loads by testing availability
                    QSqlDatabase testDb = QSqlDatabase::addDatabase("QMYSQL", "driver_test");
                    bool works = testDb.isValid();
                    QSqlDatabase::removeDatabase("driver_test");
                    if (works) {
                        return "QMYSQL";
                    }
                }
                if (QSqlDatabase::isDriverAvailable("QMARIADB")) {
                    return "QMARIADB";
                }
                return "QMYSQL";  // Return default even if unavailable for error message
            case PostgreSQL:
                return "QPSQL";
            case Oracle:
                return "QOCI";
            default:
                return "QSQLITE";
        }
    }

    static int getDefaultPort(DatabaseType type) {
        switch (type) {
            case MySQL:
                return 3306;
            case PostgreSQL:
                return 5432;
            case Oracle:
                return 1521;
            default:
                return 0;
        }
    }

    QString getTypeName() const {
        switch (type) {
            case SQLite:
                return "SQLite";
            case MySQL:
                return "MySQL";
            case PostgreSQL:
                return "PostgreSQL";
            case Oracle:
                return "Oracle";
            default:
                return "Unknown";
        }
    }

    void save(QSettings& settings) const {
        settings.setValue("database/type", static_cast<int>(type));
        settings.setValue("database/name", name);
        settings.setValue("database/host", host);
        settings.setValue("database/port", port);
        settings.setValue("database/username", username);
        settings.setValue("database/password", password);
        settings.setValue("database/connectionName", connectionName);
        settings.setValue("database/charset", charset);
        settings.setValue("database/useSSL", useSSL);
        settings.setValue("database/connectionTimeout", connectionTimeout);
    }

    void load(QSettings& settings) {
        type = static_cast<DatabaseType>(settings.value("database/type", SQLite).toInt());
        name = settings.value("database/name").toString();
        host = settings.value("database/host", "localhost").toString();
        port = settings.value("database/port", getDefaultPort(type)).toInt();
        username = settings.value("database/username").toString();
        password = settings.value("database/password").toString();
        connectionName = settings.value("database/connectionName", "default_connection").toString();
        charset = settings.value("database/charset", "utf8").toString();
        useSSL = settings.value("database/useSSL", false).toBool();
        connectionTimeout = settings.value("database/connectionTimeout", 30).toInt();
    }

    bool isValid() const {
        if (type == SQLite) {
            return !name.isEmpty();
        } else {
            return !name.isEmpty() && !host.isEmpty() && port > 0;
        }
    }
};

#endif // DATABASECONFIG_H
