#include "mainwindow.h"
#include <QApplication>
#include <QVector>
#include <QString>
#include "helper.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    QVector<QString> tmp;
    Helper::getHostInfo(tmp);

    return a.exec();
}
