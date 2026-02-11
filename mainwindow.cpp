#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "src/databasemanager.h"
#include "src/configdialog.h"
#include "src/connectionstore.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QMessageBox>
#include <QSettings>
#include <QScrollArea>
#include <QMenuBar>
#include <QMenu>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_queryModel(nullptr)
    , m_connectionForm(nullptr)
{
    ui->setupUi(this);

    // Initialize query model
    m_queryModel = new QSqlQueryModel(this);

    // Setup status bar label
    m_statusLabel = new QLabel("Disconnected", this);
    ui->statusbar->addPermanentWidget(m_statusLabel);

    // Load saved configuration
    QSettings settings("DBUI", "DatabaseManager");
    m_config.load(settings);

    iconMenu = new QMenu(this);
    iconMenu->setIcon(QIcon(":/logo.svg"));
    ui->menubar->setCornerWidget(new QLabel(), Qt::TopLeftCorner);
    ui->menubar->insertMenu(ui->menubar->actions().isEmpty() ? nullptr : ui->menubar->actions().first(), iconMenu);

    // Load connection store
    ConnectionStore::instance().load();

    // Setup the landing page with embedded connection form
    setupLandingPage();

    // Initial UI state
    updateConnectionStatus();

    // Initialize table content buttons as disabled
    setTableContentButtonsEnabled(false);
}

MainWindow::~MainWindow()
{
    // Save configuration
    QSettings settings("DBUI", "DatabaseManager");
    m_config.save(settings);

    delete ui;
}

void MainWindow::updateConnectionStatus()
{
    bool connected = DatabaseManager::instance().isConnected();

    if (connected) {
        ui->appStack->setCurrentIndex(1); // Show connected app
        ui->connectionStatus->setText("Connected");
        ui->connectionStatus->setStyleSheet("color: #a6e3a1;"); // Green
        m_statusLabel->setText("Connected: " + m_config.getTypeName());
        ui->btnTables->setChecked(true);
        ui->btnQuery->setChecked(false);
        ui->stackedWidget->setCurrentIndex(0);
        ui->pageTitle->setText("Tables");
        refreshTableList();
    } else {
        ui->appStack->setCurrentIndex(0); // Show landing page
        ui->connectionStatus->setText("Disconnected");
        ui->connectionStatus->setStyleSheet("color: #f38ba8;"); // Red
        m_statusLabel->setText("Disconnected");
        refreshConnectionLists();
        // Clear table list when disconnected
        ui->tableListWidget->clear();
        ui->dataTableWidget->clear();
        ui->dataTableWidget->setRowCount(0);
        ui->dataTableWidget->setColumnCount(0);
        ui->selectedTableLabel->setText("Select a table to view its contents");
        ui->recordsInfoLabel->setText("No table selected");
        setTableContentButtonsEnabled(false);
        m_currentTable.clear();
    }
}

void MainWindow::refreshTableList()
{
    if (!DatabaseManager::instance().isConnected()) {
        return;
    }

    QStringList tables = DatabaseManager::instance().getTables();

    // Update table list widget
    ui->tableListWidget->clear();

    for (const QString &table : tables) {
        ui->tableListWidget->addItem(table);
    }
}

void MainWindow::setTableContentButtonsEnabled(bool enabled)
{
    ui->btnEditRecord->setEnabled(enabled);
    ui->btnDeleteRecord->setEnabled(enabled);
    ui->btnRefreshTable->setEnabled(enabled);
    ui->btnExportTable->setEnabled(enabled);
}

void MainWindow::executeQuery(const QString &sql)
{
    if (!DatabaseManager::instance().isConnected()) {
        QMessageBox::warning(this, "Not Connected",
            "Please connect to a database first.");
        return;
    }

    if (sql.isEmpty()) {
        QMessageBox::warning(this, "Empty Query",
            "Please enter a query to execute.");
        return;
    }

    QSqlDatabase db = DatabaseManager::instance().getDatabase();
    m_queryModel->setQuery(sql, db);

    if (m_queryModel->lastError().isValid()) {
        QMessageBox::critical(this, "Query Error",
            m_queryModel->lastError().text());
    } else {
        // Display results in the query result table
        int columnCount = m_queryModel->columnCount();
        int rowCount = m_queryModel->rowCount();

        ui->queryResultTable->clear();
        ui->queryResultTable->setColumnCount(columnCount);
        ui->queryResultTable->setRowCount(rowCount);

        // Set headers
        QStringList headers;
        for (int col = 0; col < columnCount; ++col) {
            headers << m_queryModel->headerData(col, Qt::Horizontal).toString();
        }
        ui->queryResultTable->setHorizontalHeaderLabels(headers);

        // Fill data
        for (int row = 0; row < rowCount; ++row) {
            for (int col = 0; col < columnCount; ++col) {
                QTableWidgetItem *item = new QTableWidgetItem(
                    m_queryModel->data(m_queryModel->index(row, col)).toString());
                ui->queryResultTable->setItem(row, col, item);
            }
        }

        ui->queryResultTable->resizeColumnsToContents();
    }
}

void MainWindow::loadTableData(const QString &tableName)
{
    if (!DatabaseManager::instance().isConnected()) {
        QMessageBox::warning(this, "Not Connected",
            "Please connect to a database first.");
        return;
    }

    QString sql = QString("SELECT * FROM %1").arg(tableName);

    QSqlDatabase db = DatabaseManager::instance().getDatabase();
    m_queryModel->setQuery(sql, db);

    if (m_queryModel->lastError().isValid()) {
        QMessageBox::critical(this, "Query Error",
            "Could not load table: " + m_queryModel->lastError().text());
        return;
    }

    // Store current table name
    m_currentTable = tableName;

    // Update labels
    ui->pageTitle->setText("Tables - " + tableName);
    ui->selectedTableLabel->setText("Table: " + tableName);

    // Display results in the data table widget
    int columnCount = m_queryModel->columnCount();
    int rowCount = m_queryModel->rowCount();

    ui->dataTableWidget->clear();
    ui->dataTableWidget->setColumnCount(columnCount);
    ui->dataTableWidget->setRowCount(rowCount);

    // Set headers
    QStringList headers;
    for (int col = 0; col < columnCount; ++col) {
        headers << m_queryModel->headerData(col, Qt::Horizontal).toString();
    }
    ui->dataTableWidget->setHorizontalHeaderLabels(headers);

    // Fill data
    for (int row = 0; row < rowCount; ++row) {
        for (int col = 0; col < columnCount; ++col) {
            QTableWidgetItem *item = new QTableWidgetItem(
                m_queryModel->data(m_queryModel->index(row, col)).toString());
            ui->dataTableWidget->setItem(row, col, item);
        }
    }

    ui->dataTableWidget->resizeColumnsToContents();

    // Update records info
    ui->recordsInfoLabel->setText(QString("Showing %1 records").arg(rowCount));

    // Enable table content buttons
    setTableContentButtonsEnabled(true);
}

// Menu Actions - File 

void MainWindow::on_actionNew_triggered()
{
    // Create new SQLite database (OneDB pattern)
    QString path = QFileDialog::getSaveFileName(
        this,
        "Create New Database",
        "",
        "SQLite Database (*.db *.sqlite);;All Files (*)"
    );

    if (!path.isEmpty()) {
        m_config.type = DatabaseConfig::SQLite;
        m_config.name = path;
        m_statusLabel->setText("Configured: " + QFileInfo(path).fileName());
    }
}

void MainWindow::on_actionOpen_triggered()
{
    // Open existing database (OneDB pattern)
    QString path = QFileDialog::getOpenFileName(
        this,
        "Open Existing Database",
        "",
        "SQLite Database (*.db *.sqlite);;All Files (*)"
    );

    if (!path.isEmpty()) {
        m_config.type = DatabaseConfig::SQLite;
        m_config.name = path;
        m_statusLabel->setText("Configured: " + QFileInfo(path).fileName());
    }
}

void MainWindow::on_actionExit_triggered()
{
    close();
}

// Menu Actions - Database 

void MainWindow::on_actionConnect_Database_triggered()
{
    // Show config dialog (QWekaDB pattern)
    ConfigDialog dialog(this);
    dialog.setConfig(m_config);

    if (dialog.exec() == QDialog::Accepted) {
        m_config = dialog.getConfig();

        // Try to connect
        if (DatabaseManager::instance().initialize(m_config)) {
            ConnectionStore::instance().addRecentConnection(m_config);
            updateConnectionStatus();
            QMessageBox::information(this, "Connected",
                "Successfully connected to database.");
        } else {
            QMessageBox::critical(this, "Connection Failed",
                DatabaseManager::instance().getLastError());
        }
    }
}

void MainWindow::on_actionDisconnect_triggered()
{
    if (DatabaseManager::instance().isConnected()) {
        DatabaseManager::instance().closeConnection();
        updateConnectionStatus();
    } else {
        QMessageBox::information(this, "Not Connected",
            "No active database connection.");
    }
}

// Menu Actions - View 

void MainWindow::on_actionRefresh_triggered()
{
    on_btnRefresh_clicked();
}

void MainWindow::on_actionShowSidebar_triggered(bool checked)
{
    ui->sidebarFrame->setVisible(checked);
}

void MainWindow::on_actionFullScreen_triggered()
{
    if (isFullScreen()) {
        showNormal();
    } else {
        showFullScreen();
    }
}

// Query Tab 

void MainWindow::on_btnExecuteQuery_clicked()
{
    QString sql = ui->queryEditor->toPlainText();
    executeQuery(sql);
}

// Data Tab Buttons 

void MainWindow::on_btnRefresh_clicked()
{
    // Re-execute current query if any
    if (m_queryModel && !m_queryModel->query().lastQuery().isEmpty()) {
        executeQuery(m_queryModel->query().lastQuery());
    }
}

// Sidebar Navigation 

void MainWindow::on_btnTables_clicked()
{
    ui->btnTables->setChecked(true);
    ui->btnQuery->setChecked(false);

    ui->stackedWidget->setCurrentIndex(0);
    ui->pageTitle->setText("Tables");
}

void MainWindow::on_btnQuery_clicked()
{
    ui->btnTables->setChecked(false);
    ui->btnQuery->setChecked(true);

    ui->stackedWidget->setCurrentIndex(1);
    ui->pageTitle->setText("Query Editor");
}

void MainWindow::on_btnSettings_clicked()
{
    ui->btnTables->setChecked(false);
    ui->btnQuery->setChecked(false);

    ui->stackedWidget->setCurrentIndex(2);
    ui->pageTitle->setText("Settings");
}

void MainWindow::on_btnDisconnect_clicked()
{
    on_actionDisconnect_triggered();
}

void MainWindow::on_btnConfigureDatabase_clicked()
{
    on_actionConnect_Database_triggered();
}

// Table List Actions 

void MainWindow::on_btnAddTable_clicked()
{
    // Switch to query editor with CREATE TABLE template
    on_btnQuery_clicked();
    ui->queryEditor->setPlainText("CREATE TABLE table_name (\n    id INTEGER PRIMARY KEY,\n    column1 TEXT,\n    column2 INTEGER\n);");
}

void MainWindow::on_btnRefreshTables_clicked()
{
    refreshTableList();
}

void MainWindow::on_tableListWidget_itemClicked(QListWidgetItem *item)
{
    if (item) {
        QString tableName = item->text();
        loadTableData(tableName);
    }
}

// Table Content Actions 

void MainWindow::on_btnRefreshTable_clicked()
{
    if (!m_currentTable.isEmpty()) {
        loadTableData(m_currentTable);
    }
}

void MainWindow::on_btnExportTable_clicked()
{
    if (m_currentTable.isEmpty()) {
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this,
        "Export Table Data",
        m_currentTable + ".csv",
        "CSV Files (*.csv);;All Files (*)");

    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Export Error", "Could not open file for writing.");
        return;
    }

    QTextStream out(&file);

    // Write headers
    QStringList headers;
    for (int col = 0; col < ui->dataTableWidget->columnCount(); ++col) {
        headers << ui->dataTableWidget->horizontalHeaderItem(col)->text();
    }
    out << headers.join(",") << "\n";

    // Write data
    for (int row = 0; row < ui->dataTableWidget->rowCount(); ++row) {
        QStringList rowData;
        for (int col = 0; col < ui->dataTableWidget->columnCount(); ++col) {
            QTableWidgetItem *item = ui->dataTableWidget->item(row, col);
            QString cellData = item ? item->text() : "";
            // Escape quotes and wrap in quotes if contains comma
            if (cellData.contains(',') || cellData.contains('"')) {
                cellData.replace("\"", "\"\"");
                cellData = "\"" + cellData + "\"";
            }
            rowData << cellData;
        }
        out << rowData.join(",") << "\n";
    }

    file.close();
    QMessageBox::information(this, "Export Complete",
        QString("Table data exported to %1").arg(fileName));
}

void MainWindow::on_btnEditRecord_clicked()
{
    QList<QTableWidgetItem*> selected = ui->dataTableWidget->selectedItems();
    if (selected.isEmpty()) {
        QMessageBox::information(this, "No Selection", "Please select a record to edit.");
        return;
    }

    // For now, just show a message - full edit functionality would require a dialog
    QMessageBox::information(this, "Edit Record",
        "Edit functionality: Use the Query Editor to modify records with UPDATE statements.");
}

void MainWindow::on_btnDeleteRecord_clicked()
{
    QList<QTableWidgetItem*> selected = ui->dataTableWidget->selectedItems();
    if (selected.isEmpty()) {
        QMessageBox::information(this, "No Selection", "Please select a record to delete.");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirm Delete",
        "Delete selected records? Use Query Editor for DELETE statements.",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QMessageBox::information(this, "Delete Record",
            "Use the Query Editor to delete records with DELETE statements.\n\n"
            "Example: DELETE FROM " + m_currentTable + " WHERE id = ?");
    }
}

// Landing Page 

void MainWindow::setupLandingPage()
{
    // Create connection form and embed it in the right panel via a scroll area
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    m_connectionForm = new ConnectionFormWidget(this);
    m_connectionForm->setConfig(m_config);
    scrollArea->setWidget(m_connectionForm);

    ui->connectionFormPanelLayout->addWidget(scrollArea);

    // Connect form signals
    connect(m_connectionForm, &ConnectionFormWidget::connectRequested,
            this, &MainWindow::onFormConnectRequested);
    connect(m_connectionForm, &ConnectionFormWidget::saveRequested,
            this, &MainWindow::onFormSaveRequested);

    // Connect connection store changes
    connect(&ConnectionStore::instance(), &ConnectionStore::connectionsChanged,
            this, &MainWindow::refreshConnectionLists);

    // Initial population
    refreshConnectionLists();
}

void MainWindow::refreshConnectionLists()
{
    const auto& store = ConnectionStore::instance();

    // Populate saved connections
    ui->savedConnectionsList->clear();
    for (int i = 0; i < store.savedConnections().size(); ++i) {
        const SavedConnection& sc = store.savedConnections()[i];
        QString displayText = sc.config.connectionName;
        QString subText;
        if (sc.config.type == DatabaseConfig::SQLite) {
            subText = sc.config.name;
        } else {
            subText = sc.config.host + ":" + QString::number(sc.config.port) + "/" + sc.config.name;
        }

        QListWidgetItem* item = new QListWidgetItem();
        item->setText(displayText + "\n" + subText);
        item->setData(Qt::UserRole, i);
        item->setData(Qt::UserRole + 1, sc.config.getTypeName().toLower());
        ui->savedConnectionsList->addItem(item);
    }

    // Update saved count label
    ui->savedConnectionsLabel->setText(
        QString("SAVED CONNECTIONS  %1").arg(store.savedConnections().size()));

    // Populate recent connections
    ui->recentConnectionsList->clear();
    for (int i = 0; i < store.recentConnections().size(); ++i) {
        const RecentConnection& rc = store.recentConnections()[i];
        QString displayText = rc.config.connectionName;
        QString subText = rc.timeAgo();

        QListWidgetItem* item = new QListWidgetItem();
        item->setText(displayText + "\n" + subText);
        item->setData(Qt::UserRole, i);
        item->setData(Qt::UserRole + 1, rc.config.getTypeName().toLower());
        ui->recentConnectionsList->addItem(item);
    }

    // Update recent count label
    ui->recentConnectionsLabel->setText(
        QString("RECENT CONNECTIONS  %1").arg(store.recentConnections().size()));
}

void MainWindow::onFormConnectRequested(const DatabaseConfig& config)
{
    m_config = config;

    if (DatabaseManager::instance().initialize(m_config)) {
        // Add to recent connections
        ConnectionStore::instance().addRecentConnection(m_config);
        updateConnectionStatus();
    } else {
        QMessageBox::critical(this, "Connection Failed",
            DatabaseManager::instance().getLastError());
    }
}

void MainWindow::onFormSaveRequested(const DatabaseConfig& config)
{
    ConnectionStore::instance().addSavedConnection(config);
}

void MainWindow::on_savedConnectionsList_itemClicked(QListWidgetItem *item)
{
    if (!item) return;
    int index = item->data(Qt::UserRole).toInt();
    const auto& saved = ConnectionStore::instance().savedConnections();
    if (index >= 0 && index < saved.size()) {
        m_connectionForm->setConfig(saved[index].config);
    }
}

void MainWindow::on_recentConnectionsList_itemClicked(QListWidgetItem *item)
{
    if (!item) return;
    int index = item->data(Qt::UserRole).toInt();
    const auto& recent = ConnectionStore::instance().recentConnections();
    if (index >= 0 && index < recent.size()) {
        m_connectionForm->setConfig(recent[index].config);
    }
}
