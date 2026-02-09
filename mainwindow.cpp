#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "src/databasemanager.h"
#include "src/configdialog.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QMessageBox>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_queryModel(nullptr)
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

    // Initial UI state
    updateConnectionStatus();

    // Set initial sidebar button state
    ui->btnDashboard->setChecked(true);

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
        ui->connectionStatus->setText("Connected");
        ui->connectionStatus->setStyleSheet("color: #a6e3a1;"); // Green
        m_statusLabel->setText("Connected: " + m_config.getTypeName());
        refreshTableList();
    } else {
        ui->connectionStatus->setText("Disconnected");
        ui->connectionStatus->setStyleSheet("color: #f38ba8;"); // Red
        m_statusLabel->setText("Disconnected");
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

    // Update dashboard stats
    ui->statValue2->setText(QString::number(tables.count()));
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

// ========== Menu Actions - File ==========

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

// ========== Menu Actions - Database ==========

void MainWindow::on_actionConnect_Database_triggered()
{
    // Show config dialog (QWekaDB pattern)
    ConfigDialog dialog(this);
    dialog.setConfig(m_config);

    if (dialog.exec() == QDialog::Accepted) {
        m_config = dialog.getConfig();

        // Try to connect
        if (DatabaseManager::instance().initialize(m_config)) {
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

// ========== Menu Actions - View ==========

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

// ========== Query Tab ==========

void MainWindow::on_btnExecuteQuery_clicked()
{
    QString sql = ui->queryEditor->toPlainText();
    executeQuery(sql);
}

// ========== Data Tab Buttons ==========

void MainWindow::on_btnRefresh_clicked()
{
    // Re-execute current query if any
    if (m_queryModel && !m_queryModel->query().lastQuery().isEmpty()) {
        executeQuery(m_queryModel->query().lastQuery());
    }
}

// ========== Sidebar Navigation ==========

void MainWindow::on_btnDashboard_clicked()
{
    // Update button states
    ui->btnDashboard->setChecked(true);
    ui->btnTables->setChecked(false);
    ui->btnQuery->setChecked(false);

    // Switch to dashboard page
    ui->stackedWidget->setCurrentIndex(0);
    ui->pageTitle->setText("Dashboard");
}

void MainWindow::on_btnTables_clicked()
{
    // Update button states
    ui->btnDashboard->setChecked(false);
    ui->btnTables->setChecked(true);
    ui->btnQuery->setChecked(false);

    // Switch to tables page
    ui->stackedWidget->setCurrentIndex(1);
    ui->pageTitle->setText("Database Tables");
}

void MainWindow::on_btnQuery_clicked()
{
    // Update button states
    ui->btnDashboard->setChecked(false);
    ui->btnTables->setChecked(false);
    ui->btnQuery->setChecked(true);

    // Switch to query editor page
    ui->stackedWidget->setCurrentIndex(2);
    ui->pageTitle->setText("Query Editor");
}

void MainWindow::on_btnSettings_clicked()
{
    // Update button states
    ui->btnDashboard->setChecked(false);
    ui->btnTables->setChecked(false);
    ui->btnQuery->setChecked(false);

    // Switch to settings page
    ui->stackedWidget->setCurrentIndex(3);
    ui->pageTitle->setText("Settings");
}

void MainWindow::on_btnConfigureDatabase_clicked()
{
    on_actionConnect_Database_triggered();
}

// ========== Table List Actions ==========

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

// ========== Table Content Actions ==========

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
