#-------------------------------------------------
#
# Project created by QtCreator 2015-10-19T21:56:04
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = tfjik_kr_gui
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    lexical_analyzer.cpp \
    syntax_analyzer.cpp

HEADERS  += mainwindow.hpp \
    lexical_analyzer.h \
    syntax_analyzer.h

QMAKE_CXXFLAGS += -std=c++11
