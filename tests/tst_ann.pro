QT += testlib widgets
CONFIG += c++17 console testcase
CONFIG -= app_bundle
TARGET = tst_ann

SRC = $$PWD/..
INCLUDEPATH += $$SRC

SOURCES += tst_ann.cpp \
    $$SRC/network.cpp $$SRC/networkcanvas.cpp $$SRC/mainwindow.cpp \
    $$SRC/dialog.cpp $$SRC/dialog2.cpp $$SRC/dialog3.cpp
HEADERS += \
    $$SRC/network.h $$SRC/networkcanvas.h $$SRC/mainwindow.h $$SRC/numberedit.h \
    $$SRC/dialog.h $$SRC/dialog2.h $$SRC/dialog3.h
FORMS += $$SRC/mainwindow.ui $$SRC/dialog.ui $$SRC/dialog2.ui $$SRC/dialog3.ui
