#-------------------------------------------------
#
# Project created by QtCreator 2017-02-11T12:57:15
#
#-------------------------------------------------

QT       += core gui
QT       += charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = UBCRM_UnitTest
TEMPLATE = app


SOURCES += main.cpp\
        ubcrmwindow.cpp \
    ubcrm_fcns.cpp \
    ubcrm_engine.cpp

HEADERS  += ubcrmwindow.h \
    ubcrm_fcns.h \
    ubcrm_engine.h

FORMS    += ubcrmwindow.ui
