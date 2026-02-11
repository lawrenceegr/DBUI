#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include "databaseconfig.h"

class ConfigDialog : public QDialog {
    Q_OBJECT

public:
    explicit ConfigDialog(QWidget *parent = nullptr);
    ~ConfigDialog();

    void setConfig(const DatabaseConfig& config);
    DatabaseConfig getConfig() const;

signals:
    void configChanged(const DatabaseConfig& config);

private slots:
    void onDatabaseTypeChanged(int index);
    void onTestConnection();
    void onBrowseSQLiteFile();
    void onAccepted();
    void onSslExpandToggled(bool checked);

private:
    void setupUI();
    void updateFieldsVisibility();
    void populateFromConfig(const DatabaseConfig& config);

    // UI Components
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
    QPushButton* m_cancelButton;

    // SSL collapsible section
    QToolButton* m_sslExpandButton;
    QWidget* m_sslOptionsWidget;

    // Layout containers
    QWidget* m_serverFieldsWidget;
    QLabel* m_nameLabel;
    QLabel* m_statusLabel;

    DatabaseConfig m_config;
};

#endif // CONFIGDIALOG_H
