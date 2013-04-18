#-------------------------------------------------
#
# Project created by QtCreator 2013-04-13T09:36:20
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = E133App
TEMPLATE = app

CONFIG    += link_pkgconfig
PKGCONFIG += "libola libolae133slp libolae133controller libolae133common libolaacn" 

SOURCES += main.cpp\
        mainwindow.cpp \
    olaworker.cpp \
    devicetracker.cpp \
    e133devicetablemodel.cpp \
    e133devicelistmodel.cpp \
    pidlistmodel.cpp

HEADERS  += mainwindow.h \
    olaworker.h \
    devicetracker.h \
    e133devicetablemodel.h \
    e133devicelistmodel.h \
    pidlistmodel.h

FORMS    += mainwindow.ui
