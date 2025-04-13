#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QSqlRecord>  // Добавлен этот заголовок

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    enum DatabaseType {
        SQLite,
        PostgreSQL
    };

    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    bool connectToDatabase(DatabaseType type, const QString &connectionName, 
                        const QString &databaseName, 
                        const QString &host = "", 
                        const QString &user = "", 
                        const QString &password = "",
                        int port = 0);
    
    void disconnectFromDatabase(const QString &connectionName);
    
    QSqlQuery executeQuery(const QString &query, const QString &connectionName);
    QStringList getTables(const QString &connectionName);
    QStringList getTableColumns(const QString &tableName, const QString &connectionName);
    
    QString lastError() const;

private:
    QMap<QString, QSqlDatabase> m_connections;
    QString m_lastError;
};

#endif // DATABASEMANAGER_H