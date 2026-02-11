#include "connectionformwidget.h"
#include "databasemanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QStandardPaths>
#include <QApplication>
#include <QScrollArea>

ConnectionFormWidget::ConnectionFormWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void ConnectionFormWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(24, 24, 24, 24);

    // Connection Type 
    QLabel* typeLabel = new QLabel("Connection Type", this);
    mainLayout->addWidget(typeLabel);

    m_dbTypeCombo = new QComboBox(this);
    m_dbTypeCombo->addItem("SQLite", DatabaseConfig::SQLite);
    m_dbTypeCombo->addItem("MySQL", DatabaseConfig::MySQL);
    m_dbTypeCombo->addItem("PostgreSQL", DatabaseConfig::PostgreSQL);
    m_dbTypeCombo->addItem("Oracle", DatabaseConfig::Oracle);
    mainLayout->addWidget(m_dbTypeCombo);

    // Server Fields Container (hidden for SQLite)
    m_serverFieldsWidget = new QWidget(this);
    QVBoxLayout* serverLayout = new QVBoxLayout(m_serverFieldsWidget);
    serverLayout->setContentsMargins(0, 0, 0, 0);
    serverLayout->setSpacing(12);

    // Host and Port Row
    QHBoxLayout* hostPortLayout = new QHBoxLayout();
    hostPortLayout->setSpacing(12);

    QVBoxLayout* hostVLayout = new QVBoxLayout();
    hostVLayout->setSpacing(4);
    QLabel* hostLabel = new QLabel("Host", this);
    m_hostEdit = new QLineEdit(this);
    m_hostEdit->setText("localhost");
    m_hostEdit->setPlaceholderText("localhost");
    hostVLayout->addWidget(hostLabel);
    hostVLayout->addWidget(m_hostEdit);

    QVBoxLayout* portVLayout = new QVBoxLayout();
    portVLayout->setSpacing(4);
    QLabel* portLabel = new QLabel("Port", this);
    m_portSpinBox = new QSpinBox(this);
    m_portSpinBox->setRange(0, 65535);
    m_portSpinBox->setValue(5432);
    m_portSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    portVLayout->addWidget(portLabel);
    portVLayout->addWidget(m_portSpinBox);

    hostPortLayout->addLayout(hostVLayout, 3);
    hostPortLayout->addLayout(portVLayout, 1);
    serverLayout->addLayout(hostPortLayout);

    // SSL Section (Collapsible)
    QHBoxLayout* sslHeaderLayout = new QHBoxLayout();
    sslHeaderLayout->setSpacing(6);

    m_sslExpandButton = new QToolButton(this);
    m_sslExpandButton->setText(QString::fromUtf8("\u25B6"));
    m_sslExpandButton->setCheckable(true);
    m_sslExpandButton->setChecked(false);

    QLabel* sslLabel = new QLabel("Enable SSL", this);
    sslLabel->setObjectName("sslLabel");

    m_sslCheckBox = new QCheckBox(this);

    sslHeaderLayout->addWidget(m_sslExpandButton);
    sslHeaderLayout->addWidget(sslLabel);
    sslHeaderLayout->addStretch();
    sslHeaderLayout->addWidget(m_sslCheckBox);
    serverLayout->addLayout(sslHeaderLayout);

    // SSL Options (hidden by default)
    m_sslOptionsWidget = new QWidget(this);
    QVBoxLayout* sslOptionsLayout = new QVBoxLayout(m_sslOptionsWidget);
    sslOptionsLayout->setContentsMargins(20, 8, 0, 0);
    sslOptionsLayout->setSpacing(8);

    QLabel* charsetLabel = new QLabel("Charset", this);
    m_charsetEdit = new QLineEdit(this);
    m_charsetEdit->setText("utf8");
    sslOptionsLayout->addWidget(charsetLabel);
    sslOptionsLayout->addWidget(m_charsetEdit);

    QHBoxLayout* timeoutLayout = new QHBoxLayout();
    QLabel* timeoutLabel = new QLabel("Timeout", this);
    m_timeoutSpinBox = new QSpinBox(this);
    m_timeoutSpinBox->setRange(5, 300);
    m_timeoutSpinBox->setValue(30);
    m_timeoutSpinBox->setSuffix(" sec");
    m_timeoutSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    timeoutLayout->addWidget(timeoutLabel);
    timeoutLayout->addWidget(m_timeoutSpinBox);
    timeoutLayout->addStretch();
    sslOptionsLayout->addLayout(timeoutLayout);

    m_sslOptionsWidget->setVisible(false);
    serverLayout->addWidget(m_sslOptionsWidget);

    // User and Password Row
    QHBoxLayout* userPassLayout = new QHBoxLayout();
    userPassLayout->setSpacing(12);

    QVBoxLayout* userVLayout = new QVBoxLayout();
    userVLayout->setSpacing(4);
    QLabel* userLabel = new QLabel("User", this);
    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setPlaceholderText("username");
    userVLayout->addWidget(userLabel);
    userVLayout->addWidget(m_usernameEdit);

    QVBoxLayout* passVLayout = new QVBoxLayout();
    passVLayout->setSpacing(4);
    QLabel* passLabel = new QLabel("Password", this);
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText(QString::fromUtf8("\u2022\u2022\u2022\u2022\u2022\u2022\u2022\u2022"));
    passVLayout->addWidget(passLabel);
    passVLayout->addWidget(m_passwordEdit);

    userPassLayout->addLayout(userVLayout, 1);
    userPassLayout->addLayout(passVLayout, 1);
    serverLayout->addLayout(userPassLayout);

    mainLayout->addWidget(m_serverFieldsWidget);

    // Database Name / File 
    QVBoxLayout* dbLayout = new QVBoxLayout();
    dbLayout->setSpacing(4);

    m_nameLabel = new QLabel("Default Database", this);
    dbLayout->addWidget(m_nameLabel);

    QHBoxLayout* dbInputLayout = new QHBoxLayout();
    dbInputLayout->setSpacing(8);
    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText("database name");
    m_browseButton = new QPushButton("Browse...", this);
    m_browseButton->setVisible(false);
    dbInputLayout->addWidget(m_nameEdit);
    dbInputLayout->addWidget(m_browseButton);
    dbLayout->addLayout(dbInputLayout);

    mainLayout->addLayout(dbLayout);

    // Status Label 
    m_statusLabel = new QLabel(this);
    m_statusLabel->setObjectName("connectionFormStatus");
    m_statusLabel->setWordWrap(true);
    mainLayout->addWidget(m_statusLabel);

    mainLayout->addStretch();

    // Test / Connect Buttons 
    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(10);
    actionLayout->addStretch();

    m_testButton = new QPushButton("Test", this);
    m_testButton->setObjectName("formTestButton");
    m_connectButton = new QPushButton("Connect", this);
    m_connectButton->setObjectName("formConnectButton");

    actionLayout->addWidget(m_testButton);
    actionLayout->addWidget(m_connectButton);
    mainLayout->addLayout(actionLayout);

    // Save Connection Section 
    QLabel* saveTitle = new QLabel("Save Connection", this);
    saveTitle->setObjectName("saveConnectionTitle");
    mainLayout->addWidget(saveTitle);

    QVBoxLayout* saveLayout = new QVBoxLayout();
    saveLayout->setSpacing(4);

    m_connectionNameEdit = new QLineEdit(this);
    m_connectionNameEdit->setText("default_connection");
    m_connectionNameEdit->setPlaceholderText("Connection name");
    saveLayout->addWidget(m_connectionNameEdit);

    QHBoxLayout* saveButtonLayout = new QHBoxLayout();
    saveButtonLayout->addStretch();
    m_saveButton = new QPushButton("Save", this);
    m_saveButton->setObjectName("formSaveButton");
    saveButtonLayout->addWidget(m_saveButton);
    saveLayout->addLayout(saveButtonLayout);

    mainLayout->addLayout(saveLayout);

    // Connect Signals 
    connect(m_dbTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ConnectionFormWidget::onDatabaseTypeChanged);
    connect(m_browseButton, &QPushButton::clicked,
            this, &ConnectionFormWidget::onBrowseSQLiteFile);
    connect(m_testButton, &QPushButton::clicked,
            this, &ConnectionFormWidget::onTestConnection);
    connect(m_connectButton, &QPushButton::clicked,
            this, &ConnectionFormWidget::onConnectClicked);
    connect(m_saveButton, &QPushButton::clicked,
            this, &ConnectionFormWidget::onSaveClicked);
    connect(m_sslExpandButton, &QToolButton::toggled,
            this, &ConnectionFormWidget::onSslExpandToggled);

    // Initial setup
    onDatabaseTypeChanged(0);
}

DatabaseConfig ConnectionFormWidget::buildConfig() const {
    DatabaseConfig config;
    config.type = static_cast<DatabaseConfig::DatabaseType>(
        m_dbTypeCombo->currentData().toInt());
    config.name = m_nameEdit->text();
    config.host = m_hostEdit->text();
    config.port = m_portSpinBox->value();
    config.username = m_usernameEdit->text();
    config.password = m_passwordEdit->text();
    config.connectionName = m_connectionNameEdit->text();
    config.charset = m_charsetEdit->text();
    config.useSSL = m_sslCheckBox->isChecked();
    config.connectionTimeout = m_timeoutSpinBox->value();
    return config;
}

void ConnectionFormWidget::setConfig(const DatabaseConfig& config) {
    for (int i = 0; i < m_dbTypeCombo->count(); ++i) {
        if (m_dbTypeCombo->itemData(i).toInt() == static_cast<int>(config.type)) {
            m_dbTypeCombo->setCurrentIndex(i);
            break;
        }
    }
    m_nameEdit->setText(config.name);
    m_hostEdit->setText(config.host);
    m_portSpinBox->setValue(config.port);
    m_usernameEdit->setText(config.username);
    m_passwordEdit->setText(config.password);
    m_connectionNameEdit->setText(config.connectionName);
    m_charsetEdit->setText(config.charset);
    m_sslCheckBox->setChecked(config.useSSL);
    m_timeoutSpinBox->setValue(config.connectionTimeout);
    updateFieldsVisibility();
}

DatabaseConfig ConnectionFormWidget::getConfig() const {
    return buildConfig();
}

void ConnectionFormWidget::setConnectionName(const QString& name) {
    m_connectionNameEdit->setText(name);
}

void ConnectionFormWidget::onDatabaseTypeChanged(int index) {
    Q_UNUSED(index);
    DatabaseConfig::DatabaseType type =
        static_cast<DatabaseConfig::DatabaseType>(m_dbTypeCombo->currentData().toInt());

    bool isSQLite = (type == DatabaseConfig::SQLite);

    m_serverFieldsWidget->setVisible(!isSQLite);
    m_browseButton->setVisible(isSQLite);

    if (isSQLite) {
        m_nameLabel->setText("Database File");
        m_nameEdit->setPlaceholderText("Path to SQLite database file");
    } else {
        m_nameLabel->setText("Default Database");
        m_nameEdit->setPlaceholderText("database name");
    }

    m_portSpinBox->setValue(DatabaseConfig::getDefaultPort(type));
    m_statusLabel->clear();
}

void ConnectionFormWidget::onSslExpandToggled(bool checked) {
    m_sslOptionsWidget->setVisible(checked);
    m_sslExpandButton->setText(checked ? QString::fromUtf8("\u25BC") : QString::fromUtf8("\u25B6"));
}

void ConnectionFormWidget::onTestConnection() {
    DatabaseConfig config = buildConfig();

    if (!config.isValid()) {
        m_statusLabel->setText("Configuration is incomplete or invalid");
        m_statusLabel->setStyleSheet("QLabel { color: #f38ba8; padding: 6px; background-color: rgba(243, 139, 168, 0.1); border-radius: 4px; }");
        return;
    }

    m_testButton->setEnabled(false);
    m_statusLabel->setText("Testing connection...");
    m_statusLabel->setStyleSheet("QLabel { color: #89b4fa; padding: 6px; }");

    QApplication::processEvents();

    QString errorMessage;
    bool success = DatabaseManager::instance().testConnection(config, errorMessage);

    m_testButton->setEnabled(true);

    if (success) {
        m_statusLabel->setText("Connection successful!");
        m_statusLabel->setStyleSheet("QLabel { color: #a6e3a1; padding: 6px; background-color: rgba(166, 227, 161, 0.1); border-radius: 4px; }");
    } else {
        m_statusLabel->setText("Connection failed: " + errorMessage);
        m_statusLabel->setStyleSheet("QLabel { color: #f38ba8; padding: 6px; background-color: rgba(243, 139, 168, 0.1); border-radius: 4px; }");
    }

    emit testRequested(config);
}

void ConnectionFormWidget::onBrowseSQLiteFile() {
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Select SQLite Database File",
        documentsPath + "/database.db",
        "SQLite Database (*.db *.sqlite *.sqlite3);;All Files (*)"
    );

    if (!fileName.isEmpty()) {
        m_nameEdit->setText(fileName);
    }
}

void ConnectionFormWidget::onConnectClicked() {
    DatabaseConfig config = buildConfig();
    if (!config.isValid()) {
        m_statusLabel->setText("Configuration is incomplete or invalid");
        m_statusLabel->setStyleSheet("QLabel { color: #f38ba8; padding: 6px; background-color: rgba(243, 139, 168, 0.1); border-radius: 4px; }");
        return;
    }
    emit connectRequested(config);
}

void ConnectionFormWidget::onSaveClicked() {
    DatabaseConfig config = buildConfig();
    if (m_connectionNameEdit->text().isEmpty()) {
        m_statusLabel->setText("Please enter a connection name");
        m_statusLabel->setStyleSheet("QLabel { color: #f38ba8; padding: 6px; background-color: rgba(243, 139, 168, 0.1); border-radius: 4px; }");
        return;
    }
    emit saveRequested(config);
}

void ConnectionFormWidget::updateFieldsVisibility() {
    onDatabaseTypeChanged(m_dbTypeCombo->currentIndex());
}
