#include "DatabaseManager.h"

DatabaseManager::DatabaseManager(QObject *parent) : QObject(parent)
{
}

DatabaseManager::~DatabaseManager()
{
    for (auto &connection : m_connections) {
        connection.close();
    }
    m_connections.clear();
}

bool DatabaseManager::connectToDatabase(DatabaseType type, const QString &connectionName, 
                                    const QString &databaseName, 
                                    const QString &host, 
                                    const QString &user, 
                                    const QString &password,
                                    int port)
{
    if (m_connections.contains(connectionName)) {
        m_lastError = "Connection with this name already exists";
        return false;
    }

    QSqlDatabase db;
    if (type == SQLite) {
        db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        db.setDatabaseName(databaseName);
    } 
    else if (type == PostgreSQL) {
        db = QSqlDatabase::addDatabase("QPSQL", connectionName);
        db.setDatabaseName(databaseName);
        db.setHostName(host);
        db.setUserName(user);
        db.setPassword(password);
        if (port > 0) {
            db.setPort(port);
        }
    }

    if (!db.open()) {
        m_lastError = db.lastError().text();
        return false;
    }

    m_connections.insert(connectionName, db);
    return true;
}

void DatabaseManager::disconnectFromDatabase(const QString &connectionName)
{
    if (m_connections.contains(connectionName)) {
        m_connections[connectionName].close();
        m_connections.remove(connectionName);
        QSqlDatabase::removeDatabase(connectionName);
    }
}

QSqlQuery DatabaseManager::executeQuery(const QString &query, const QString &connectionName)
{
    if (!m_connections.contains(connectionName)) {
        m_lastError = "Connection not found";
        return QSqlQuery();
    }

    QSqlQuery qry(m_connections[connectionName]);
    if (!qry.exec(query)) {
        m_lastError = qry.lastError().text();
    }
    return qry;
}

QStringList DatabaseManager::getTables(const QString &connectionName)
{
    QStringList tables;
    if (m_connections.contains(connectionName)) {
        tables = m_connections[connectionName].tables(QSql::Tables);
    }
    return tables;
}

QStringList DatabaseManager::getTableColumns(const QString &tableName, const QString &connectionName)
{
    QStringList columns;
    if (m_connections.contains(connectionName)) {
        QSqlRecord record = m_connections[connectionName].record(tableName);
        for (int i = 0; i < record.count(); ++i) {
            columns << record.fieldName(i);
        }
    }
    return columns;
}

QString DatabaseManager::lastError() const
{
    return m_lastError;
}