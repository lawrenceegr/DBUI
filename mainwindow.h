#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QStringListModel>
#include <QLabel>
#include <QListWidgetItem>
#include "src/databaseconfig.h"
#include "src/connectionformwidget.h"
#include <QMenu>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    // Database state
    DatabaseConfig m_config;
    QSqlQueryModel *m_queryModel;
    QLabel *m_statusLabel;

    QMenu *iconMenu;

    // Helper functions
    void updateConnectionStatus();
    void executeQuery(const QString &sql);
    void loadTableData(const QString &tableName);
    void refreshTableList();
    void setTableContentButtonsEnabled(bool enabled);
    void setupLandingPage();
    void refreshConnectionLists();

    // Landing page
    ConnectionFormWidget* m_connectionForm;

    // Current selected table
    QString m_currentTable;

private slots:
    // Menu actions - File
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionExit_triggered();

    // Menu actions - Database
    void on_actionConnect_Database_triggered();
    void on_actionDisconnect_triggered();

    // Menu actions - View
    void on_actionRefresh_triggered();
    void on_actionShowSidebar_triggered(bool checked);
    void on_actionFullScreen_triggered();

    // Query tab
    void on_btnExecuteQuery_clicked();

    // Data tab buttons
    void on_btnRefresh_clicked();

    // Sidebar navigation
    void on_btnTables_clicked();
    void on_btnQuery_clicked();
    void on_btnSettings_clicked();
    void on_btnDisconnect_clicked();

    // Settings page
    void on_btnConfigureDatabase_clicked();

    // Table list actions
    void on_btnAddTable_clicked();
    void on_btnRefreshTables_clicked();
    void on_tableListWidget_itemClicked(QListWidgetItem *item);

    // Table content actions
    void on_btnRefreshTable_clicked();
    void on_btnExportTable_clicked();
    void on_btnEditRecord_clicked();
    void on_btnDeleteRecord_clicked();

    // Landing page actions
    void onFormConnectRequested(const DatabaseConfig& config);
    void onFormSaveRequested(const DatabaseConfig& config);
    void on_savedConnectionsList_itemClicked(QListWidgetItem *item);
    void on_recentConnectionsList_itemClicked(QListWidgetItem *item);

};

#endif // MAINWINDOW_H
