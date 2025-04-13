#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "DatabaseManager.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onConnectToDatabase();
    void onDisconnectFromDatabase();
    void onExecuteQuery();
    void onConnectionSelected(int index);
    void onTableSelected(int index);

private:
    Ui::MainWindow *ui;
    DatabaseManager *dbManager;
    
    void updateConnectionsList();
    void updateTablesList(const QString &connectionName);
    void updateQueryResults(QSqlQuery query);
};

#endif // MAINWINDOW_H