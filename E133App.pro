#-------------------------------------------------
#
# Project created by QtCreator 2013-04-13T09:36:20
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = E133App
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    olaworker.cpp \
    devicetracker.cpp \
    e133devicetablemodel.cpp \
    e133devicelistmodel.cpp

HEADERS  += mainwindow.h \
    olaworker.h \
    devicetracker.h \
    e133devicetablemodel.h \
    e133devicelistmodel.h

FORMS    += mainwindow.ui
