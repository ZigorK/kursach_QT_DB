#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QMessageBox>
#include <QSqlQueryModel>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    dbManager(new DatabaseManager(this))
{
    ui->setupUi(this); 
    
    connect(ui->btnConnect, &QPushButton::clicked, this, &MainWindow::onConnectToDatabase);
    connect(ui->btnDisconnect, &QPushButton::clicked, this, &MainWindow::onDisconnectFromDatabase);
    connect(ui->btnExecute, &QPushButton::clicked, this, &MainWindow::onExecuteQuery);
    connect(ui->cbConnections, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onConnectionSelected);
    connect(ui->cbTables, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onTableSelected);
    connect(ui->btnBuildQuery, &QPushButton::clicked, this, [this]() {
        QString tableName = ui->cbTables->currentText();
        if (!tableName.isEmpty()) {
            ui->pteQuery->setPlainText("SELECT * FROM " + tableName + " LIMIT 100");
        }
    });
    connect(ui->btnExport, &QPushButton::clicked, this, [this]() {
        QAbstractItemModel *model = ui->tvResults->model();
        if (!model) return;
        
        QString fileName = QFileDialog::getSaveFileName(this, "Export to CSV", "", "CSV Files (*.csv)");
        if (fileName.isEmpty()) return;
        
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            // Заголовки
            for (int col = 0; col < model->columnCount(); ++col) {
                if (col > 0) out << ",";
                out << "\"" << model->headerData(col, Qt::Horizontal).toString() << "\"";
            }
            out << "\n";
            // Данные
            for (int row = 0; row < model->rowCount(); ++row) {
                for (int col = 0; col < model->columnCount(); ++col) {
                    if (col > 0) out << ",";
                    out << "\"" << model->data(model->index(row, col)).toString() << "\"";
                }
                out << "\n";
            }
            file.close();
        }
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onConnectToDatabase()
{
    DatabaseManager::DatabaseType dbType = ui->rbSQLite->isChecked() ? 
                                         DatabaseManager::SQLite : 
                                         DatabaseManager::PostgreSQL;
    
    QString connectionName = ui->leConnectionName->text();
    if (connectionName.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter connection name");
        return;
    }
    
    bool success = false;
    if (dbType == DatabaseManager::SQLite) {
        QString dbPath = ui->leSQLitePath->text();
        success = dbManager->connectToDatabase(dbType, connectionName, dbPath);
    } else {
        QString dbName = ui->lePGDatabase->text();
        QString host = ui->lePGHost->text();
        QString user = ui->lePGUser->text();
        QString password = ui->lePGPassword->text();
        int port = ui->sbPGPort->value();
        
        success = dbManager->connectToDatabase(dbType, connectionName, dbName, 
                                             host, user, password, port);
    }
    
    if (!success) {
        QMessageBox::critical(this, "Connection error", dbManager->lastError());
    } else {
        updateConnectionsList();
        QMessageBox::information(this, "Success", "Connected successfully");
    }
}

void MainWindow::onDisconnectFromDatabase()
{
    QString connectionName = ui->cbConnections->currentText();
    if (connectionName.isEmpty()) return;
    
    dbManager->disconnectFromDatabase(connectionName);
    updateConnectionsList();
}

void MainWindow::onExecuteQuery()
{
    QString connectionName = ui->cbConnections->currentText();
    if (connectionName.isEmpty()) {
        QMessageBox::warning(this, "Error", "No connection selected");
        return;
    }
    
    QString queryText = ui->pteQuery->toPlainText();
    if (queryText.isEmpty()) {
        QMessageBox::warning(this, "Error", "Query is empty");
        return;
    }
    
    QSqlQuery result = dbManager->executeQuery(queryText, connectionName);
    updateQueryResults(std::move(result));
}

void MainWindow::onConnectionSelected(int index)
{
    if (index < 0) return;
    QString connectionName = ui->cbConnections->currentText();
    updateTablesList(connectionName);
}

void MainWindow::onTableSelected(int index)
{
    if (index < 0) return;
    
    QString connectionName = ui->cbConnections->currentText();
    QString tableName = ui->cbTables->currentText();
    
    if (!connectionName.isEmpty() && !tableName.isEmpty()) {
        QString query = QString("SELECT * FROM %1 LIMIT 100").arg(tableName);
        QSqlQuery result = dbManager->executeQuery(query, connectionName);
        updateQueryResults(std::move(result));
    }
}

void MainWindow::updateConnectionsList()
{
    ui->cbConnections->clear();
    // В реальном проекте нужно получить список активных подключений из DatabaseManager
}

void MainWindow::updateTablesList(const QString &connectionName)
{
    ui->cbTables->clear();
    QStringList tables = dbManager->getTables(connectionName);
    ui->cbTables->addItems(tables);
}

void MainWindow::updateQueryResults(QSqlQuery query)
{
    QSqlQueryModel *model = new QSqlQueryModel(this);
    model->setQuery(std::move(query));
    ui->tvResults->setModel(model);
    
    if (model->lastError().isValid()) {
        QMessageBox::warning(this, "Query error", model->lastError().text());
    }
}