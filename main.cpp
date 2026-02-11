#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QIcon>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setDesktopFileName("DBUI");

    QFile styleFile(":/style.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        a.setStyleSheet(styleFile.readAll());
        styleFile.close();
    }

    MainWindow w;
    w.setWindowIcon(QIcon(":/logo.png"));
    w.show();
    return a.exec();
}
