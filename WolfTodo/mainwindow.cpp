#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QInputDialog>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QFileDialog>
#include <QFile>
#include <QCryptographicHash>
#include <QMessageBox>
#include "dblogin.h"
#include <QPushButton>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    config(this),
    mysql(this)
{
    ui->setupUi(this);
    config.init();
    mysql.init();
    if (!config.ConfigGetConfigValue("username").isValid()){
        QString username = QInputDialog::getText (this, "NAME", "Enter NAME!");
        if (username.isEmpty()) exit(0);
        config.ConfigSetValue("username",username);
    }
    connect(&uploader,&HTTPdownloader::setProgress,[this](int prc){
        ui->fileProgress->setValue(prc);
    });

    refreshList();
    ui->entrySave->setGraphicsEffect(&buttonEffect);

    ui->treeList->header()->setStretchLastSection(false);
    ui->treeList->header()->setSectionResizeMode(0, QHeaderView::Stretch);



}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::refreshList()
{
    currentEntry = nullptr;

    ui->entryTitle->setReadOnly(true);
    ui->entryTitle->clear();
    ui->entryState->setEnabled(false);
    ui->entryState->clear();
    ui->entryState->addItems({"TODO","DONE"});
    ui->entrySave->setEnabled(false);
    ui->entryDescription->setReadOnly(true);
    ui->entryDescription->clear();
    ui->entryDate->setReadOnly(true);
    ui->entryDate->clear();
    ui->entryAddFile->setEnabled(false);

    ui->treeList->blockSignals(true);

    ui->treeList->clear();
    todoEntries = mysql.MysqlGetTodoList();
    todoFiles = mysql.MysqlGetTodoFiles();
    QStringList categories;
    for (TodoEntry& it : todoEntries){
        categories.push_back(it.category);
    }
    categories.removeDuplicates();
    ui->comboNewEntryCategory->clear();
    ui->comboNewEntryCategory->addItems(categories);
    QMap<QString,QTreeWidgetItem*> categoryItems;
    for (auto& category : categories){
        auto item = new QTreeWidgetItem();
        item->setText(0,category);
        ui->treeList->addTopLevelItem(item);
        categoryItems.insert(category,item);
    }
    for (TodoEntry& it : todoEntries){
        auto item = new QTreeWidgetItem();
        item->setText(0,it.title);
        item->setText(1,it.author);
        item->setText(2,it.state);
        item->setData(0,Qt::UserRole,it.id);
        it.item = item;
        (*categoryItems.find(it.category))->addChild(item);
    }
    ui->treeList->resizeColumnToContents(0);

    ui->treeList->blockSignals(false);


    ui->entryFiles->clearContents();
    ui->entryFiles->setRowCount(0);
    refreshListColors();
}

void MainWindow::addEntryFile(TodoFile & it)
{
    ui->entryFiles->setRowCount(ui->entryFiles->rowCount()+1);
    int currentRow = ui->entryFiles->rowCount()-1;


    QLabel* name = new QLabel();
    name->setText(it.filename);
    ui->entryFiles->setCellWidget(currentRow,0,name);

    QLabel* date = new QLabel();
    date->setText(it.date.toString());
    ui->entryFiles->setCellWidget(currentRow,2,date);



    QPushButton* saveButton = new QPushButton("Save",this);
    ui->entryFiles->setCellWidget(currentRow,1,saveButton);
    connect(saveButton,&QPushButton::clicked,[it,this](){
        qDebug() << it.url;
        QString file = QFileDialog::getExistingDirectory(0,
                                                         "Datei auswählen",
                                                         config.ConfigGetConfigValue("InputDirectory").isValid() ? config.ConfigGetConfigValue("InputDirectory").toString() : QString()
                                                                                                                   /*,
                                                                                                                                                                                                                                                                                                                                                                                     it.filename,0,
                                                                                                                                                                                                                                                                                                                                                                                     QFileDialog::DontResolveSymlinks*/);
        uploader.getFile(it.url,file+"/"+it.filename);

    });
}

void MainWindow::refreshListColors()
{

    for (int var = 0; var <  ui->treeList->topLevelItemCount(); ++var) {
        auto TLitem = ui->treeList->topLevelItem(var);

        bool categoryDone = true;


        for (int subVar = 0; subVar <  TLitem->childCount(); ++subVar) {
            auto item = TLitem->child(subVar);
            bool isDone = item->text(2) == "DONE";
            if (!isDone) categoryDone = false;
            item->setBackgroundColor(2,isDone ? QColor(0,255,0) : QColor(255,0,0));
        };

        TLitem->setBackgroundColor(2,categoryDone ? QColor(0,255,0) : QColor(255,0,0));
    }

}

void MainWindow::on_treeList_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *)
{
    if (current->text(1).isEmpty()) return;

    auto itemID = current->data(0,Qt::UserRole).toInt();
    currentEntry = nullptr;
    for (TodoEntry& it : todoEntries){
        if (it.id == itemID){
            currentEntry = &it;
            break;
        }
    }
    if (currentEntry==nullptr) return;

    ui->entryTitle->blockSignals(true);
    ui->entryState->blockSignals(true);
    ui->entryDescription->blockSignals(true);
    ui->entryDate->blockSignals(true);
    ui->entrySave->setEnabled(true);
    ui->entryAddFile->setEnabled(true);

    ui->entryTitle->setReadOnly(false);
    ui->entryState->setEnabled(true);
    ui->entryDescription->setReadOnly(false);
    ui->entryDate->setReadOnly(false);


    ui->entryTitle->setText(currentEntry->title);
    ui->entryState->setCurrentText(currentEntry->state);
    ui->entryDescription->setText(currentEntry->description);
    ui->entryDate->setDateTime(currentEntry->added);
    buttonEffect.setColor(QColor::fromRgb(250,0,0,0));

    ui->entryTitle->blockSignals(false);
    ui->entryState->blockSignals(false);
    ui->entryDescription->blockSignals(false);
    ui->entryDate->blockSignals(false);

    ui->entryFiles->clearContents();
    ui->entryFiles->setRowCount(0);

    for (TodoFile& it : todoFiles){
        if (it.entryID != itemID) continue;
        addEntryFile(it);
    }
















}

void MainWindow::on_entryTitle_textChanged(const QString &)
{
    if (!currentEntry) return;
    currentEntry->title = ui->entryTitle->text();
    buttonEffect.setColor(QColor::fromRgb(250,0,0,255));
}

void MainWindow::on_entrySave_clicked()
{
    if (!currentEntry) return;
    if (currentEntry->title.isEmpty()){
        QMessageBox::warning(nullptr,"NO","Title cannot be empty");
        return;
    }

    if (currentEntry->newEntry){
        if (mysql.addEntry(*currentEntry)){
            buttonEffect.setColor(QColor::fromRgb(0,250,0,255));
            currentEntry->newEntry = false;
            currentEntry->item->setText(0,currentEntry->title);
            currentEntry->item->setData(0,Qt::UserRole,currentEntry->id);
        }
    } else {
        if (mysql.updateEntry(*currentEntry)){
            buttonEffect.setColor(QColor::fromRgb(0,250,0,255));
            currentEntry->item->setText(0,currentEntry->title);
        }

    }
    refreshListColors();
}

void MainWindow::on_entryDescription_textChanged()
{
    if (!currentEntry) return;
    currentEntry->description = ui->entryDescription->toPlainText();
    buttonEffect.setColor(QColor::fromRgb(250,0,0,255));
}

void MainWindow::on_entryState_currentIndexChanged(int)
{

    if (!currentEntry) return;
    currentEntry->state = ui->entryState->currentText();
    buttonEffect.setColor(QColor::fromRgb(250,0,0,255));
}

void MainWindow::on_buttonRefresh_clicked()
{
    refreshList();
}

void MainWindow::on_buttonNewEntry_clicked()
{
    if (ui->comboNewEntryCategory->currentText().isEmpty()) return;
    TodoEntry newEntry;
    newEntry.author = config.ConfigGetConfigValue("username").toString();
    newEntry.added = QDateTime::currentDateTime();
    newEntry.newEntry = true;
    newEntry.category = ui->comboNewEntryCategory->currentText();
    newEntry.state = "TODO";
    newEntry.id = newEntryID++;
    bool inserted=false;
    for (int var = 0; var <  ui->treeList->topLevelItemCount(); ++var) {
        auto TLitem = ui->treeList->topLevelItem(var);

        if (TLitem->text(0) == newEntry.category){

            auto item = new QTreeWidgetItem();
            item->setText(0,newEntry.title);
            item->setText(1,newEntry.author);
            item->setText(2,newEntry.state);
            item->setData(0,Qt::UserRole,newEntry.id);
            newEntry.item = item;
            TLitem->addChild(item);
            inserted = true;
            break;
        }

    }
    if (!inserted){
        auto item = new QTreeWidgetItem();
        item->setText(0,newEntry.title);
        item->setText(1,newEntry.author);
        item->setText(2,newEntry.state);
        item->setData(0,Qt::UserRole,newEntry.id);
        newEntry.item = item;
        auto TLItem = new QTreeWidgetItem();
        TLItem->setText(0,newEntry.category);
        TLItem->addChild(item);
        ui->treeList->addTopLevelItem(TLItem);
    }
    todoEntries.push_back(newEntry);

}

void MainWindow::on_treeList_expanded(const QModelIndex &)
{
    ui->treeList->resizeColumnToContents(1);
}

void MainWindow::on_entryAddFile_clicked()
{
    if (!currentEntry) return;
    if (currentEntry->newEntry){
        QMessageBox::warning(nullptr,"NO","You have to save the current entry first");
    }
    QString file = QFileDialog::getOpenFileName(0,
                                                "Datei auswählen",
                                                config.ConfigGetConfigValue("InputDirectory").isValid() ? config.ConfigGetConfigValue("InputDirectory").toString() : QString(),
                                                "*",0,
                                                QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly);

    if (file.length() < 5)
        return;
    config.ConfigSetValue("InputDirectory",QVariant(file.left(file.lastIndexOf("/"))));
    qDebug() << QVariant(file.left(file.lastIndexOf("/")));
    QStringRef filename = file.rightRef(file.length()-file.lastIndexOf("/")-1);
    QString fileHash;
    QFile f(file);
    if (f.open(QFile::ReadOnly)) {
        QCryptographicHash hash(QCryptographicHash::Algorithm::Sha256);
        if (hash.addData(&f)) {
            fileHash = QString(hash.result().toBase64());
        }
    }
    if (fileHash.isEmpty()){
        QMessageBox::warning(nullptr,"Upload error",f.errorString() + "\nCouldn't get hash");
        return;
    }

    fileHash = fileHash.replace("/","_");
    fileHash = fileHash.replace("+","-");

    if (!uploader.uploadfile(file,fileHash)){
        QMessageBox::warning(nullptr,"Upload Failed","Upload failed");
        return;
    }
    TodoFile newFile;
    newFile.entryID = currentEntry->id;
    newFile.filename = filename.toString();
    newFile.date = QDateTime::currentDateTime();
    newFile.url = FTPDWLURL + fileHash;
    mysql.addFile(newFile,*currentEntry);
    todoFiles.push_back(newFile);
    addEntryFile(newFile);
}
