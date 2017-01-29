#ifndef TODOENTRY_H
#define TODOENTRY_H
#include <stdint.h>
#include <QDateTime>

class TodoEntry
{
public:
    TodoEntry();

    uint32_t id;
    QString title;
    QString author;
    QString description;
    QDateTime added;
    QString state;
    QString category;
    bool newEntry = false;
};

class TodoFile
{
public:
    uint32_t id;
    uint32_t entryID;
    QString filename;
    QString url;
    QDateTime date;
};

#endif // TODOENTRY_H
