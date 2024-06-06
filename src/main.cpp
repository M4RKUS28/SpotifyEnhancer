#include "mainwindow.h"

#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;

    if(argc > 1 && strcmp(argv[1], "show") == 0)
        w.show();
    return a.exec();
}




