#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "spotifymanager.h"
#include "updater.h"
#include "ui_mainwindow.h"

#include <QMainWindow>
#include <windows.h>

#include <QMessageBox>
#include <QTime>
#include <QString>
#include <QMouseEvent>

struct v_window {
    v_window() { }
    v_window( HWND window, std::string exe, std::string titel, std::string path,  bool checkInv = false,  std::string checkTitel = "")
        : window(window), titel(titel) , path(path), exe(exe), checkInv(checkInv), checkTitel(checkTitel), count(0)
    { }
    HWND window;
    std::string titel;
    std::string path;
    std::string exe;
    bool checkInv;
    std::string checkTitel;
    int count;
};

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    QString version = "1.4.0";

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

void closeEvent(QCloseEvent* event) override;

    SpotifyManager * spotmngr;
    Updater * updater;

private slots:
    void on_actionSpotify_Pfad_setzen_triggered();

    void on_pushButton_clicked();

    void on_actionAutostart_triggered(bool checked);

private:
    Ui::MainWindow *ui;


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
    virtual bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result);

//    void showContextMenu(const QPoint& pos);
//    void onTrayIconClicked(QMouseEvent* event);


    // QObject interface
private slots:
    void updateCounts();
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
    void on_actionAutomatisch_nach_Updates_suchen_triggered();
};


#endif // MAINWINDOW_H
