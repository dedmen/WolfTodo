#include "mysql.h"
#include <QApplication>
#include <QtSql>
#include "dblogin.h"
#include <QHash>
#include <QDebug>
#include <QMessageBox>
QSqlDatabase db;
Mysql::Mysql(QObject *parent) :
    QObject(parent)
{
}

void Mysql::init(){
    //qDebug() << QSqlDatabase::drivers();
    db = QSqlDatabase::addDatabase("QMYSQL3","WolfTodoMysqlMod");
    db.setHostName(DBSERVER);
    db.setDatabaseName(DBNAME);
    db.setUserName(DBUSER);
    db.setPassword(DBPW);
    bool ok = db.open(); // 248 ms
    if (!ok) {
        qCritical() << "DB1 Failed" << db.lastError().text();
        db.setHostName(DBSERVER2);
        db.setUserName(DBUSER);
        db.setPassword(DBPW);
        ok = db.open();
    }
    if (!ok){
        QMessageBox::critical(0," Modpack Updater","Konnte keine Verbindung zur Datenbank herstellen.\nÜberprüfen sie ihre Internetverbindung.\n"+ db.lastError().text());
        qFatal("NOSQL");//qFatal exits
    }
    db.setConnectOptions("MYSQL_OPT_RECONNECT=1;"); //QMYSQLDriver::open: Unknown connect option 'MYSQL_OPT_COMPRESS'

}
void Mysql::error(QString identifier,QString query,QSqlError error){
    QMessageBox::warning(nullptr,"WolfTodo mysqlError",error.text());
    /*
    MU->GlobalVariablesO->log("MysqlError "+identifier+" "+query+" | "+error.text()+"|"+QString::number(error.number()));
    if (error.number() == -1){
        MU->log(QString("mysql::error isOpen:%1 isOpenError:%2 isValid:%3 open:%4").arg(db.isOpen())
                .arg(db.isOpenError())
                .arg(db.isValid())
                .arg(db.open())

                );
    }*/
}

QVector<TodoEntry> Mysql::MysqlGetTodoList() {
    QVector<TodoEntry> todos;
    QSqlQuery *query = new QSqlQuery(db);
    if(query->exec("SELECT todoEntries.`id`, todoEntries.`title`, todoEntries.`author`, todoEntries.`description`, todoEntries.`added`, todoEntries.`state`, todoEntries.`category` FROM todoEntries")) {
        while (query->next()) {
            TodoEntry entry;
            entry.id = query->value(0).toInt();
            entry.title = query->value(1).toString();
            entry.author = query->value(2).toString();
            entry.description = query->value(3).toString();
            entry.added = query->value(4).toDateTime();
            entry.state = query->value(5).toString();
            entry.category = query->value(6).toString();
            todos.push_back(entry);
        }
        delete query;
        return todos;
    } else {
        error("MysqlGetModpacks",query->lastQuery(),query->lastError().text());
    }
    delete query;
    return todos;
}

QVector<TodoFile> Mysql::MysqlGetTodoFiles()
{
    QVector<TodoFile> files;
    QSqlQuery *query = new QSqlQuery(db);

    if(query->exec("SELECT todoFiles.`id`, todoFiles.`todoEntry`, todoFiles.`fileName`, todoFiles.`url`, todoFiles.`date` FROM todoFiles")) {
        while (query->next()) {
            TodoFile file;
            file.id = query->value(0).toInt();
            file.entryID = query->value(1).toInt();
            file.filename = query->value(2).toString();
            file.url = query->value(3).toString();
            file.date = query->value(4).toDateTime();
            files.push_back(file);
        }
        delete query;
        return files;
    } else {
        error("MysqlGetModpacks",query->lastQuery(),query->lastError().text());
    }
    delete query;
    return files;
}

bool Mysql::updateEntry(TodoEntry &entry)
{

    QSqlQuery *query = new QSqlQuery(db);
    QString Squery;
    Squery = "UPDATE `todoEntries` SET "
             "`title`='" + entry.title + "' ,"
             "`description`='" + entry.description + "' ,"
             "`state`='" + entry.state + "' ,"
             "`category`='" + entry.category + "' "
             " WHERE (`id`=" + QString::number(entry.id) + ");";

    if(query->exec(Squery)) {
        return true;
    } else {
        error("MysqlWriteConfig",query->lastQuery(),query->lastError().text());
        delete query;
        return false;
    }
    return true;

}

bool Mysql::addEntry(TodoEntry &entry)
{
    QSqlQuery *query = new QSqlQuery(db);
    QString Squery;

    Squery = "INSERT INTO `todoEntries` (title, author,description,added,state,category) VALUES ("
             "'"+ entry.title + "',"
             "'"+ entry.author + "',"
             "'" + entry.description + "',"
             "from_unixtime(" + QString::number(entry.added.toTime_t()) + "),"
             "'" + entry.state + "',"
             "'" +entry.category + "');";

    if(query->exec(Squery)) {
        entry.id = query->lastInsertId().toInt();
        return true;
    } else {
        error("addEntry",query->lastQuery(),query->lastError().text());
        delete query;
        return false;
    }
    return false;
}

bool Mysql::addFile(TodoFile &file,TodoEntry& entry)
{
    QSqlQuery *query = new QSqlQuery(db);
    QString Squery;

    Squery = "INSERT INTO `todoFiles` (todoEntry,fileName,url,date) VALUES ("
             ""+ QString::number(entry.id) + ","
             "'"+ file.filename + "',"
             "'" + file.url + "',"
             "from_unixtime(" + QString::number(file.date.toTime_t()) + "));";
    if(query->exec(Squery)) {
        return true;
    } else {
        error("addFile",query->lastQuery(),query->lastError().text());
        delete query;
        return false;
    }
    return false;
}


void Mysql::executeStatement(QString statement)
{
    QSqlQuery *query = new QSqlQuery(db);
    if(query->exec(statement)) {
    } else {
        error("mysql::executeStatement",query->lastQuery(),query->lastError().text());
        delete query;
        return;
    }
    delete query;
    return;
}
