# 复现程序：直接链接原版源码（git 4ffc9e2，不改一行），逐条复现 bug，并和新版 network.cpp 做基准对比。
# 先在仓库根目录把原版导出到 docs/repro/orig（命令的解释见 docs/修改记录.md 第 6 节）：
#   git archive -o docs/repro/orig.tar 4ffc9e2
#   mkdir docs/repro/orig
#   tar -xf docs/repro/orig.tar -C docs/repro/orig
QT += core gui widgets
CONFIG += c++17 console
CONFIG -= app_bundle
TARGET = repro_original

ORIG = $$PWD/orig
INCLUDEPATH += $$ORIG

SOURCES += repro_original.cpp \
    $$ORIG/adjacency_list.cpp $$ORIG/dialog.cpp $$ORIG/dialog2.cpp $$ORIG/dialog3.cpp \
    $$ORIG/hidden_layer.cpp $$ORIG/inputneure.cpp $$ORIG/mainwindow.cpp $$ORIG/neure.cpp \
    $$ORIG/outputneure.cpp $$ORIG/transport.cpp
HEADERS += $$ORIG/mainwindow.h $$ORIG/dialog.h $$ORIG/dialog2.h $$ORIG/dialog3.h
FORMS += $$ORIG/dialog.ui $$ORIG/dialog2.ui $$ORIG/dialog3.ui $$ORIG/mainwindow.ui

# 基准对比：同时编译新版的 network.cpp；关掉原版 TopSort 里的 qDebug，只比算法本身
DEFINES += QT_NO_DEBUG_OUTPUT
NEWSRC = $$PWD/../..
INCLUDEPATH += $$NEWSRC
SOURCES += $$NEWSRC/network.cpp
