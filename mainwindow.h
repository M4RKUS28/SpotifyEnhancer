#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "spotifymanager.h"
#include <QMainWindow>
#include <windows.h>

#include <QMessageBox>
#include <QTime>
#include <QString>



QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

void closeEvent(QCloseEvent* event) override;

    SpotifyManager * spotmngr;

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

    // Custom message identifier
#define WM_TRAYICON (WM_USER + 1)

    // Handle system tray events
    virtual bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result);

    // QObject interface
private slots:
    void updateCounts();
    void showThis();

    void onOption1Clicked();
    void onOption2Clicked();
    void acstatusclicked();

    void on_pushButton_2_clicked();

    void on_spinBox_ms_checkrate_valueChanged(int arg1);
    void on_actionGesammtzahl_Werbungen_triggered();
    void on_action_ber_triggered();
};


#endif // MAINWINDOW_H
