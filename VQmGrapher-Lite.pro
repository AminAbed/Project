#-------------------------------------------------
#
# Project created by QtCreator 2014-05-30T15:31:51
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VQmGrapher-Lite
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    FileSystemDialog.cpp

HEADERS  += mainwindow.h \
    FileSystemDialog.h

FORMS    += mainwindow.ui \
    FileSystemDialog.ui

RESOURCES += \
    images.qrc
