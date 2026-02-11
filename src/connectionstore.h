#ifndef CONNECTIONSTORE_H
#define CONNECTIONSTORE_H

#include <QObject>
#include <QList>
#include <QDateTime>
#include <QSettings>
#include "databaseconfig.h"

struct SavedConnection {
    DatabaseConfig config;
    QDateTime lastUsed;

    void save(QSettings& settings, int index) const {
        QString prefix = QString("saved_connections/%1/").arg(index);
        settings.setValue(prefix + "type", static_cast<int>(config.type));
        settings.setValue(prefix + "name", config.name);
        settings.setValue(prefix + "host", config.host);
        settings.setValue(prefix + "port", config.port);
        settings.setValue(prefix + "username", config.username);
        settings.setValue(prefix + "password", config.password);
        settings.setValue(prefix + "connectionName", config.connectionName);
        settings.setValue(prefix + "charset", config.charset);
        settings.setValue(prefix + "useSSL", config.useSSL);
        settings.setValue(prefix + "connectionTimeout", config.connectionTimeout);
        settings.setValue(prefix + "lastUsed", lastUsed);
    }

    void load(QSettings& settings, int index) {
        QString prefix = QString("saved_connections/%1/").arg(index);
        config.type = static_cast<DatabaseConfig::DatabaseType>(
            settings.value(prefix + "type", DatabaseConfig::SQLite).toInt());
        config.name = settings.value(prefix + "name").toString();
        config.host = settings.value(prefix + "host", "localhost").toString();
        config.port = settings.value(prefix + "port", DatabaseConfig::getDefaultPort(config.type)).toInt();
        config.username = settings.value(prefix + "username").toString();
        config.password = settings.value(prefix + "password").toString();
        config.connectionName = settings.value(prefix + "connectionName", "default_connection").toString();
        config.charset = settings.value(prefix + "charset", "utf8").toString();
        config.useSSL = settings.value(prefix + "useSSL", false).toBool();
        config.connectionTimeout = settings.value(prefix + "connectionTimeout", 30).toInt();
        lastUsed = settings.value(prefix + "lastUsed").toDateTime();
    }
};

struct RecentConnection {
    DatabaseConfig config;
    QDateTime connectedAt;

    void save(QSettings& settings, int index) const {
        QString prefix = QString("recent_connections/%1/").arg(index);
        settings.setValue(prefix + "type", static_cast<int>(config.type));
        settings.setValue(prefix + "name", config.name);
        settings.setValue(prefix + "host", config.host);
        settings.setValue(prefix + "port", config.port);
        settings.setValue(prefix + "username", config.username);
        settings.setValue(prefix + "password", config.password);
        settings.setValue(prefix + "connectionName", config.connectionName);
        settings.setValue(prefix + "charset", config.charset);
        settings.setValue(prefix + "useSSL", config.useSSL);
        settings.setValue(prefix + "connectionTimeout", config.connectionTimeout);
        settings.setValue(prefix + "connectedAt", connectedAt);
    }

    void load(QSettings& settings, int index) {
        QString prefix = QString("recent_connections/%1/").arg(index);
        config.type = static_cast<DatabaseConfig::DatabaseType>(
            settings.value(prefix + "type", DatabaseConfig::SQLite).toInt());
        config.name = settings.value(prefix + "name").toString();
        config.host = settings.value(prefix + "host", "localhost").toString();
        config.port = settings.value(prefix + "port", DatabaseConfig::getDefaultPort(config.type)).toInt();
        config.username = settings.value(prefix + "username").toString();
        config.password = settings.value(prefix + "password").toString();
        config.connectionName = settings.value(prefix + "connectionName", "default_connection").toString();
        config.charset = settings.value(prefix + "charset", "utf8").toString();
        config.useSSL = settings.value(prefix + "useSSL", false).toBool();
        config.connectionTimeout = settings.value(prefix + "connectionTimeout", 30).toInt();
        connectedAt = settings.value(prefix + "connectedAt").toDateTime();
    }

    QString timeAgo() const {
        if (!connectedAt.isValid()) return "Unknown";
        qint64 secs = connectedAt.secsTo(QDateTime::currentDateTime());
        if (secs < 60) return "Just now";
        if (secs < 3600) return QString("%1 minutes ago").arg(secs / 60);
        if (secs < 86400) return QString("%1 hours ago").arg(secs / 3600);
        return QString("%1 days ago").arg(secs / 86400);
    }
};

class ConnectionStore : public QObject {
    Q_OBJECT

public:
    static ConnectionStore& instance() {
        static ConnectionStore s;
        return s;
    }

    void load() {
        QSettings settings("DBUI", "DatabaseManager");
        int savedCount = settings.value("saved_connections/count", 0).toInt();
        m_saved.clear();
        for (int i = 0; i < savedCount; ++i) {
            SavedConnection sc;
            sc.load(settings, i);
            m_saved.append(sc);
        }

        int recentCount = settings.value("recent_connections/count", 0).toInt();
        m_recent.clear();
        for (int i = 0; i < recentCount; ++i) {
            RecentConnection rc;
            rc.load(settings, i);
            m_recent.append(rc);
        }
    }

    void save() {
        QSettings settings("DBUI", "DatabaseManager");
        // Clear old entries
        settings.remove("saved_connections");
        settings.remove("recent_connections");

        settings.setValue("saved_connections/count", m_saved.size());
        for (int i = 0; i < m_saved.size(); ++i) {
            m_saved[i].save(settings, i);
        }

        settings.setValue("recent_connections/count", m_recent.size());
        for (int i = 0; i < m_recent.size(); ++i) {
            m_recent[i].save(settings, i);
        }
    }

    const QList<SavedConnection>& savedConnections() const { return m_saved; }
    const QList<RecentConnection>& recentConnections() const { return m_recent; }

    void addSavedConnection(const DatabaseConfig& config) {
        // Update existing or add new
        for (int i = 0; i < m_saved.size(); ++i) {
            if (m_saved[i].config.connectionName == config.connectionName) {
                m_saved[i].config = config;
                m_saved[i].lastUsed = QDateTime::currentDateTime();
                save();
                emit connectionsChanged();
                return;
            }
        }
        SavedConnection sc;
        sc.config = config;
        sc.lastUsed = QDateTime::currentDateTime();
        m_saved.append(sc);
        save();
        emit connectionsChanged();
    }

    void removeSavedConnection(int index) {
        if (index >= 0 && index < m_saved.size()) {
            m_saved.removeAt(index);
            save();
            emit connectionsChanged();
        }
    }

    void addRecentConnection(const DatabaseConfig& config) {
        // Remove duplicate if exists
        for (int i = m_recent.size() - 1; i >= 0; --i) {
            if (m_recent[i].config.connectionName == config.connectionName &&
                m_recent[i].config.host == config.host &&
                m_recent[i].config.name == config.name) {
                m_recent.removeAt(i);
            }
        }
        RecentConnection rc;
        rc.config = config;
        rc.connectedAt = QDateTime::currentDateTime();
        m_recent.prepend(rc);

        // Keep max 10 recent
        while (m_recent.size() > 10) {
            m_recent.removeLast();
        }
        save();
        emit connectionsChanged();
    }

signals:
    void connectionsChanged();

private:
    ConnectionStore() {}
    QList<SavedConnection> m_saved;
    QList<RecentConnection> m_recent;
};

#endif // CONNECTIONSTORE_H
