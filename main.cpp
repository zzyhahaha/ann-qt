#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <hidden_layer.h>
#include <inputneure.h>
#include <outputnerue.h>
#include <transport.h>
#include <QMouseEvent>
int mark;   //负责给各个神经元标号


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "ANN_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    MainWindow w;
    w.show();
    return a.exec();
}
