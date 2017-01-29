#ifndef CONFIG_H
#define CONFIG_H
#include <QVariant>
#include <QSettings>
class Config: public QObject
{
    Q_OBJECT
public:
    Config(QObject *parent);
    void init();
    QVariant ConfigGetConfigValue(QString value);
    bool ConfigSetValue(QString value, QVariant newvalue);
    QSettings* MainConfig;
private:
    bool tsCompatSet;
    QVariant ConfigGetDefaultValue(QString value);
    signals:
    void errorMessage(QString title, QString message);
};
#endif // CONFIG_H
