#ifndef CONNECTIONFORMWIDGET_H
#define CONNECTIONFORMWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QToolButton>
#include "databaseconfig.h"

class ConnectionFormWidget : public QWidget {
    Q_OBJECT

public:
    explicit ConnectionFormWidget(QWidget *parent = nullptr);

    void setConfig(const DatabaseConfig& config);
    DatabaseConfig getConfig() const;
    void setConnectionName(const QString& name);

signals:
    void connectRequested(const DatabaseConfig& config);
    void testRequested(const DatabaseConfig& config);
    void saveRequested(const DatabaseConfig& config);

private slots:
    void onDatabaseTypeChanged(int index);
    void onTestConnection();
    void onBrowseSQLiteFile();
    void onConnectClicked();
    void onSaveClicked();
    void onSslExpandToggled(bool checked);

private:
    void setupUI();
    void updateFieldsVisibility();
    DatabaseConfig buildConfig() const;

    // Form fields
    QComboBox* m_dbTypeCombo;
    QLineEdit* m_nameEdit;
    QLineEdit* m_hostEdit;
    QSpinBox* m_portSpinBox;
    QLineEdit* m_usernameEdit;
    QLineEdit* m_passwordEdit;
    QLineEdit* m_connectionNameEdit;
    QLineEdit* m_charsetEdit;
    QCheckBox* m_sslCheckBox;
    QSpinBox* m_timeoutSpinBox;
    QPushButton* m_browseButton;
    QPushButton* m_testButton;
    QPushButton* m_connectButton;
    QPushButton* m_saveButton;

    // SSL collapsible
    QToolButton* m_sslExpandButton;
    QWidget* m_sslOptionsWidget;

    // Layout containers
    QWidget* m_serverFieldsWidget;
    QLabel* m_nameLabel;
    QLabel* m_statusLabel;
};

#endif // CONNECTIONFORMWIDGET_H
