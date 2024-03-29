#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "spotifymanager.h"
#include "dialogueber.h"
#include "stylehandler.h"
#include "ui_mainwindow.h"

#include <QMainWindow>
#include <windows.h>

#include <QMessageBox>
#include <QTime>
#include <QString>
#include <QMouseEvent>
#include <QEvent>

//struct v_window {
//    v_window() { }
//    v_window( HWND window, std::string exe, std::string titel, std::string path,  bool checkInv = false,  std::string checkTitel = "")
//        : window(window), titel(titel) , path(path), exe(exe), checkInv(checkInv), checkTitel(checkTitel), count(0)
//    { }
//    HWND window;
//    std::string titel;
//    std::string path;
//    std::string exe;
//    bool checkInv;
//    std::string checkTitel;
//    int count;
//};

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    QString version = "1.7.6";

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void closeEvent(QCloseEvent* event) override;

    SpotifyManager * spotmngr;
    DialogUeber * dialogUeber;
    StyleHandler * styleHandler;

private slots:
    void on_actionSpotify_Pfad_setzen_triggered();
    void on_actionAutostart_triggered(bool checked);

private:
    Ui::MainWindow *ui;

    void setColoredBackground(bool status);

    // System Tray
    QMenu* trayMenu;
    QAction* actionOption1;
    QAction* actionOption2;
    QAction* acstatus;
    QAction* startSpot;
    QAction* stopSpot;


    // Custom message identifier
#define WM_TRAYICON (WM_USER + 1)

    // Handle system tray events
    virtual bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;

//    void showContextMenu(const QPoint& pos);
//    void onTrayIconClicked(QMouseEvent* event);


    // QObject interface
private slots:
    void showThis();

    void onOption1Clicked();
    void onOption2Clicked();
    void acstatusclicked();
    void startSpotSlot();
    void stopSpotSlot();


    void on_pushButton_2_clicked();

    void on_spinBox_ms_checkrate_valueChanged(int arg1);
    void on_actionGesammtzahl_Werbungen_triggered();
    void on_action_ber_triggered();

    void on_radioButtonStstaus_clicked(bool status);
    void on_pushButtonExit_clicked();
    void on_radioButtonStstaus_toggled(bool checked);

    // QObject interface
    void on_actionForceForeground_triggered();
//protected:
//     void timerEvent(QTimerEvent *event) override;

    void on_actionColored_Background_triggered(bool checked);
    void on_spinBoxPlayDelay_valueChanged(int arg1);
};




#endif // MAINWINDOW_H
