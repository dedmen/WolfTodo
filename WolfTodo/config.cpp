#include "config.h"
#include <QVariant>
#include <QSettings>
#include <QDebug>
#include <QStringList>
#include <QMessageBox>
#include <QStandardPaths>
#include <QInputDialog>
#include <QStringBuilder>
#include <QFileDialog>
#include <windows.h>
#include <QCoreApplication>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
QHash<QString,QVariant> DefaultValues;
QSettings *MainConfig;

Config::Config(QObject *parent) :
    QObject(parent)
{
    DefaultValues.insert(QStringLiteral("forcedArmaPath"),QVariant(QStringLiteral("")));
}

void Config::init() {
    QSettings ArmaRegistry(QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Bohemia Interactive\\ArmA 3"),QSettings::NativeFormat);
    QString ArmaPath = ArmaRegistry.value(QStringLiteral("main")).toString();
    QString directory = "";
    if (ArmaPath.length() < 5){

        do {
            if (!directory.isEmpty())
                QMessageBox::critical(0,
                                      "Wolf Todo","Dieser Pfad ist kein zulässiger Arma 3 Pfad."
                                      );
            directory = QFileDialog::getExistingDirectory(0,
                                                          "Arma Pfad auswählen",
                                                          directory,
                                                          QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly);

        }
        while (!QDir(directory).entryList().contains(QStringLiteral("arma3.exe"),Qt::CaseInsensitive) && !directory.isEmpty());

        ArmaPath = directory;
    }

    MainConfig = new QSettings(ArmaPath+"\\WolfTodoConfig.ini",QSettings::IniFormat );
}

QVariant Config::ConfigGetDefaultValue(QString value) {
    return DefaultValues.value(value);
}

QVariant Config::ConfigGetConfigValue(QString value) {
    //if(MainConfig == nullptr)//crashed because Intel was initialized before config
    //	return QVariant();
    QVariant result = MainConfig->value(value);
    if (result.isValid()) {
        return result;
    } else {
        QVariant result = ConfigGetDefaultValue(value);
        if (result.isValid()) {
            return result;
        } else {
            return QVariant(); //doesn't exist
            //qCritical() << "Falsche Config Abfrage" << value;
            // QMessageBox::critical(0,"Falsche Config Abfrage",value);
        }
    }
    return QVariant();
}

bool Config::ConfigSetValue(QString value,QVariant newvalue){
    MainConfig->setValue(value,QVariant(newvalue));
    return true;
}
