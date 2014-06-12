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
    CustomLineEdit.cpp \
    PatientInfo.cpp \
    MainWindow.cpp \
    QCustomPlot.cpp

HEADERS  += \
    CustomLineEdit.h \
    PatientInfo.h \
    MainWindow.h \
    QCustomPlot.h

FORMS    += mainwindow.ui

RESOURCES += \
    images.qrc
