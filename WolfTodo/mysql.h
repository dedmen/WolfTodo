#ifndef MYSQL_H
#define MYSQL_H
#include <QtSql>
#include <QString>
#include <QHash>
#include "todoentry.h"
extern QSqlDatabase db;
class Mysql: public QObject
{
    Q_OBJECT
public:
    Mysql(QObject *parent);
    void init();
    QVector<TodoEntry> MysqlGetTodoList();
    QVector<TodoFile> MysqlGetTodoFiles();
    bool updateEntry(TodoEntry& entry);
    bool addEntry(TodoEntry& entry);
    bool addFile(TodoFile& file, TodoEntry &entry);
    void executeStatement(QString statement);
    QSqlDatabase db;
    QObject *parent;
private:
    void error(QString identifier,QString query,QSqlError error);
};

#endif // MYSQL_H

