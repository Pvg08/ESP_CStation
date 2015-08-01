#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtTranslator);

    QTranslator myappTranslator;
    myappTranslator.load(QCoreApplication::applicationDirPath()+"/translations/CStation_" + QLocale::system().name());
    app.installTranslator(&myappTranslator);

    MainWindow w;
    w.show();

    return app.exec();
}
