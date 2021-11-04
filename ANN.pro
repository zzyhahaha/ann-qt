QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    adjacency_list.cpp \
    hidden_layer.cpp \
    inputneure.cpp \
    main.cpp \
    mainwindow.cpp \
    neure.cpp \
    outputneure.cpp \
    transport.cpp

HEADERS += \
    adjacency_list.h \
    hidden_layer.h \
    inputneure.h \
    mainwindow.h \
    neure.h \
    outputneure.h \
    transport.h

FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    ANN_zh_CN.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
