#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "config.h"
#include "mysql.h"
#include <QTreeWidgetItem>
#include <QGraphicsColorizeEffect>
#include "httpdownloader.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    Config config;
    Mysql mysql;
    HTTPdownloader uploader;
    void refreshList();
    QVector<TodoEntry> todoEntries;
    QVector<TodoFile> todoFiles;
    void addEntryFile(TodoFile&);
    uint32_t newEntryID = 16000;
    TodoEntry* currentEntry = nullptr;
    QGraphicsColorizeEffect buttonEffect;
private slots:
    void on_treeList_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

    void on_entryTitle_textChanged(const QString &arg1);

    void on_entrySave_clicked();

    void on_entryDescription_textChanged();

    void on_entryState_currentIndexChanged(int index);

    void on_buttonRefresh_clicked();

    void on_buttonNewEntry_clicked();

    void on_treeList_expanded(const QModelIndex &index);

    void on_entryAddFile_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
