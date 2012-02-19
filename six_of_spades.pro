#-------------------------------------------------
#
# Project created by QtCreator 2012-02-08T12:13:38
#
#-------------------------------------------------

QT       += core gui\
            sql

TARGET = six_of_spades
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    ticketdialog.cpp \
    userdialog.cpp

HEADERS  += mainwindow.h \
    ticketdialog.h \
    userdialog.h

FORMS    += mainwindow.ui \
    ticketdialog.ui \
    userdialog.ui

OTHER_FILES += \
    database_config.txt
