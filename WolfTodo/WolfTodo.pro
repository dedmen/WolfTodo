#-------------------------------------------------
#
# Project created by QtCreator 2017-01-29T15:20:26
#
#-------------------------------------------------

QT       += core gui widgets sql network

TARGET = WolfTodo
TEMPLATE = app
win32:CONFIG(debug, debug|release): LIBS += -lqwindowsd -lqtfreetyped -lQt5PlatformSupportd

win32:RC_ICONS += ToDoList.ico


SOURCES += main.cpp\
        mainwindow.cpp \
    config.cpp \
    mysql.cpp \
    todoentry.cpp \
    httpdownloader.cpp

HEADERS  += mainwindow.h \
    config.h \
    mysql.h \
    dblogin.h \
    todoentry.h \
    httpdownloader.h

FORMS    += mainwindow.ui
