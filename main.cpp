#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false); // Fix bug when shown updater window

//    a.setStyle("fusion"); // looks good with dark color scheme
//    //Customize color palette for dark mode
//    QPalette darkPalette;
//    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
//    darkPalette.setColor(QPalette::WindowText, Qt::white);
//    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
//    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
//    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
//    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
//    darkPalette.setColor(QPalette::Text, Qt::white);
//    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
//    darkPalette.setColor(QPalette::ButtonText, Qt::white);
//    darkPalette.setColor(QPalette::BrightText, Qt::red);
//    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));

//    // Setzen Sie die Dark Palette für Ihre Anwendung
//    QApplication::setPalette(darkPalette);

    MainWindow w;
//    w.show();
    return a.exec();
}
