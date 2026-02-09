#include "mainwindow.h"

#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setDesktopFileName("DBUI");

    QFile styleFile(":/style.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        a.setStyleSheet(styleFile.readAll());
        styleFile.close();
    }

    a.setWindowIcon(QIcon(":/icon.png"));

    MainWindow w;
    w.show();
    return a.exec();
}
