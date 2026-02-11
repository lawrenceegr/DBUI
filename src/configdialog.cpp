#include "configdialog.h"
#include "databasemanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QApplication>

ConfigDialog::ConfigDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    setWindowTitle("Database Configuration");
    resize(480, 420);
}

ConfigDialog::~ConfigDialog() {
}

void ConfigDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(20, 20, 20, 20);

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
    m_sslExpandButton->setText("▶");
    m_sslExpandButton->setCheckable(true);
    m_sslExpandButton->setChecked(false);

    QLabel* sslLabel = new QLabel("Enable SSL", this);
    sslLabel->setObjectName("configSslLabel");

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
    m_passwordEdit->setPlaceholderText("••••••••");
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

    // Connection Name 
    QVBoxLayout* connNameLayout = new QVBoxLayout();
    connNameLayout->setSpacing(4);
    QLabel* connNameLabel = new QLabel("Connection Name", this);
    m_connectionNameEdit = new QLineEdit(this);
    m_connectionNameEdit->setText("default_connection");
    m_connectionNameEdit->setPlaceholderText("Connection name");
    connNameLayout->addWidget(connNameLabel);
    connNameLayout->addWidget(m_connectionNameEdit);
    mainLayout->addLayout(connNameLayout);

    // Status Label 
    m_statusLabel = new QLabel(this);
    m_statusLabel->setObjectName("configStatusLabel");
    m_statusLabel->setWordWrap(true);
    mainLayout->addWidget(m_statusLabel);

    mainLayout->addStretch();

    // Buttons 
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);

    m_testButton = new QPushButton("Test", this);
    m_connectButton = new QPushButton("Connect", this);
    m_connectButton->setObjectName("connectButton");
    m_cancelButton = new QPushButton("Cancel", this);

    buttonLayout->addStretch();
    buttonLayout->addWidget(m_testButton);
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_connectButton);
    mainLayout->addLayout(buttonLayout);

    // Connect Signals 
    connect(m_dbTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ConfigDialog::onDatabaseTypeChanged);
    connect(m_browseButton, &QPushButton::clicked,
            this, &ConfigDialog::onBrowseSQLiteFile);
    connect(m_testButton, &QPushButton::clicked,
            this, &ConfigDialog::onTestConnection);
    connect(m_connectButton, &QPushButton::clicked,
            this, &ConfigDialog::onAccepted);
    connect(m_cancelButton, &QPushButton::clicked,
            this, &QDialog::reject);
    connect(m_sslExpandButton, &QToolButton::toggled,
            this, &ConfigDialog::onSslExpandToggled);

    // Initial setup
    onDatabaseTypeChanged(0);
}

void ConfigDialog::setConfig(const DatabaseConfig& config) {
    m_config = config;
    populateFromConfig(config);
}

DatabaseConfig ConfigDialog::getConfig() const {
    return m_config;
}

void ConfigDialog::onDatabaseTypeChanged(int index) {
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

void ConfigDialog::onSslExpandToggled(bool checked) {
    m_sslOptionsWidget->setVisible(checked);
    m_sslExpandButton->setText(checked ? "▼" : "▶");
}

void ConfigDialog::onTestConnection() {
    DatabaseConfig config;
    config.type = static_cast<DatabaseConfig::DatabaseType>(
        m_dbTypeCombo->currentData().toInt());
    config.name = m_nameEdit->text();
    config.host = m_hostEdit->text();
    config.port = m_portSpinBox->value();
    config.username = m_usernameEdit->text();
    config.password = m_passwordEdit->text();
    config.connectionName = "test_connection";
    config.charset = m_charsetEdit->text();
    config.useSSL = m_sslCheckBox->isChecked();
    config.connectionTimeout = m_timeoutSpinBox->value();

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
}

void ConfigDialog::onBrowseSQLiteFile() {
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

void ConfigDialog::onAccepted() {
    m_config.type = static_cast<DatabaseConfig::DatabaseType>(
        m_dbTypeCombo->currentData().toInt());
    m_config.name = m_nameEdit->text();
    m_config.host = m_hostEdit->text();
    m_config.port = m_portSpinBox->value();
    m_config.username = m_usernameEdit->text();
    m_config.password = m_passwordEdit->text();
    m_config.connectionName = m_connectionNameEdit->text();
    m_config.charset = m_charsetEdit->text();
    m_config.useSSL = m_sslCheckBox->isChecked();
    m_config.connectionTimeout = m_timeoutSpinBox->value();

    if (!m_config.isValid()) {
        QMessageBox::warning(this, "Invalid Configuration",
            "Please fill in all required fields.");
        return;
    }

    emit configChanged(m_config);
    accept();
}

void ConfigDialog::updateFieldsVisibility() {
    onDatabaseTypeChanged(m_dbTypeCombo->currentIndex());
}

void ConfigDialog::populateFromConfig(const DatabaseConfig& config) {
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
