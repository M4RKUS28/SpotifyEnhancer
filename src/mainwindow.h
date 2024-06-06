#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "spotifymanager.h"
#include "dialogueber.h"
#include "stylehandler.h"
#include "ui_mainwindow.h"
#include "switch.h"

#include <QMainWindow>
#include <windows.h>

#include <QMessageBox>
#include <QTime>
#include <QString>
#include <QMouseEvent>
#include <QEvent>
#include <QStyleHints>



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

    QString version = "1.10.2";

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void closeEvent(QCloseEvent* event) override;

    SpotifyManager * spotmngr;
    DialogUeber * dialogUeber;

    Switch
        *autoStartBtn,
        *appbackgroundBtn,
        *status_btn,
        * transition_btn;


private slots:
    void on_actionSpotify_Pfad_setzen_triggered();
    void on_actionAutostart_triggered(bool checked);
    void colorSchemeChanged(Qt::ColorScheme colorScheme);


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


    const int TEST_INTERVALL_HOCH = 100;
    const int TEST_INTERVALL_MITTEL = 500;
    const int TEST_INTERVALL_GERING = 1000;

    const int DELAY_INTERVALL_AUS = 10;
    const int DELAY_INTERVALL_GERING = 200;
    const int DELAY_INTERVALL_MITTEL = 500;
    const int DELAY_INTERVALL_HOCH = 1000;

    const int DELAY_TRANISITION = 5000;

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

    void toggledAutostartBtn(bool checked);
    void toggledBackgroundBtn(bool checked);
    void toggledstatus_btn(bool checked);
    void toggled_transition_btn(bool checked);


    void on_pushButton_2_clicked();


    void on_actionGesammtzahl_Werbungen_triggered();
    void on_action_ber_triggered();

    void on_radioButtonStstaus_clicked(bool status);
    void on_pushButtonExit_clicked();
    void on_radioButtonStstaus_toggled(bool checked);

    // QObject interface
    void on_actionForceForeground_triggered();

    void styleChanged();
//protected:
//     void timerEvent(QTimerEvent *event) override;

    void on_actionColored_Background_triggered(bool checked);

    void on_pushButtonChangePath_clicked();
    void on_comboBoxOSTheme_currentIndexChanged(int index);
    void on_actionAuf_Updates_pr_fen_triggered();
    void on_comboBoxTestRate_currentIndexChanged(int index);
    void on_comboBox_delayTime_currentIndexChanged(int index);
    void on_pushButtonremovewatermark_clicked();
};






#endif // MAINWINDOW_H
