QT       += core gui widgets

# 用到了 QMouseEvent::position()、QStringConverter 等 Qt 6 接口
lessThan(QT_MAJOR_VERSION, 6): error("需要 Qt 6（原项目用的是 Qt 6.1.3 MinGW 64-bit）")

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dialog.cpp \
    dialog2.cpp \
    dialog3.cpp \
    main.cpp \
    mainwindow.cpp \
    network.cpp \
    networkcanvas.cpp

HEADERS += \
    dialog.h \
    dialog2.h \
    dialog3.h \
    mainwindow.h \
    network.h \
    networkcanvas.h \
    numberedit.h

FORMS += \
    dialog.ui \
    dialog2.ui \
    dialog3.ui \
    mainwindow.ui

TRANSLATIONS += \
    ANN_zh_CN.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
