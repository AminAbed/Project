#-------------------------------------------------
#
# Project created by QtCreator 2014-05-30T15:31:51
#
#-------------------------------------------------

QT       += core gui webkit

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VQmGrapher-Lite
TEMPLATE = app


SOURCES += main.cpp\
    CustomLineEdit.cpp \
    PatientInfo.cpp \
    MainWindow.cpp \
    QCustomPlot.cpp \
    SettingsPage.cpp \
    CommentWindow.cpp

HEADERS  += \
    CustomLineEdit.h \
    PatientInfo.h \
    MainWindow.h \
    QCustomPlot.h \
    SettingsPage.h \
    CommentWindow.h \
    CommentTracer.h

FORMS    += mainwindow.ui \
    SettingsPage.ui \
    CommentWindow.ui

RESOURCES += \
    images.qrc \
    resources/data.qrc

OTHER_FILES += \
    scripts/gen_version_header.sh \
    scripts/gitrev.sh \
    resources/export-template.html
