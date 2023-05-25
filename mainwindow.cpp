#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QStandardPaths>
#include <QSettings>
#include <iostream>
#include <QTimerEvent>
#include <QMenu>
#include <QIcon>
#include <QPixmap>
#include <QApplication>
#include <QDebug>
#include <QCloseEvent>
#include <QPushButton>




MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon(":/spotify-32.ico"));

    // Create SpotifyManager instance
    spotmngr = new SpotifyManager();
    this->ui->spinBox_ms_checkrate->setValue( spotmngr->load_and_getMs_checkrate() );

    if( spotmngr->alreadyRunningServiceAvaible() ) {
        qDebug() << "Already running service";
        exit(0);
    }

    // Initialize the system tray icon and menu
    trayMenu = new QMenu(this);
    trayMenu->setTitle("Spotify Enhancer");
//    trayMenu->setIcon(QIcon(":/spotify-32.ico"));

    actionOption1 = trayMenu->addAction("Fenster anzeigen");
    actionOption1->setIcon(QIcon(":/maximize_icon.png"));

    trayMenu->addSeparator();
    acstatus = trayMenu->addAction("Werbung entfernen");
    acstatus->setCheckable(true);
    trayMenu->addSeparator();

    startSpot = trayMenu->addAction("Spotify starten");
    startSpot->setIcon(QIcon(":/spotify_original_green.png"));
    trayMenu->addSeparator();

    stopSpot = trayMenu->addAction("Spotify beenden");
    stopSpot->setIcon(QIcon(":/close_icon.png"));
    trayMenu->addSeparator();


    actionOption2 = trayMenu->addAction("Programm beenden");
    actionOption2->setIcon(QIcon(":/beenden.png"));

    connect(actionOption1, &QAction::triggered, this, &MainWindow::onOption1Clicked);
    connect(actionOption2, &QAction::triggered, this, &MainWindow::onOption2Clicked);
    connect(acstatus, &QAction::triggered, this, &MainWindow::acstatusclicked);
    connect(startSpot, &QAction::triggered, this, &MainWindow::startSpotSlot);
    connect(stopSpot, &QAction::triggered, this, &MainWindow::stopSpotSlot);


    // Set up the NOTIFYICONDATA structure
    NOTIFYICONDATA nid;
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = (HWND)winId();
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIconW(NULL, IDI_APPLICATION);
    nid.hIcon = (HICON) LoadImageA( // returns a HANDLE so we have to cast to HICON
        NULL,             // hInstance must be NULL when loading from a file
        "spotify-32.ico",   // the icon file name
        IMAGE_ICON,       // specifies that the file is an icon
        32,                // width of the image (we'll specify default later on)
        32,                // height of the image
        LR_LOADFROMFILE|  // we want to load a file (as opposed to a resource)
            LR_DEFAULTSIZE|   // default metrics based on the type (IMAGE_ICON, 32x32)
            LR_SHARED         // let the system release the handle when it's no longer used
        );

    // Set the tooltip text
    wcscpy_s(nid.szTip, ARRAYSIZE(nid.szTip), L"Spotify Enhancer");

//    // Set the title (info balloon) text
//    wcscpy_s(nid.szInfo, ARRAYSIZE(nid.szInfo), L"Spotify Enhancer");
//    nid.uFlags |= NIF_INFO;
//    nid.dwInfoFlags = NIIF_INFO;
//    nid.uTimeout = 5000; // Display the balloon for 5 seconds

    if (nid.hIcon == NULL) {
        // Error handling: Unable to load the icon
        DWORD error = GetLastError();
        // Add code to log or display the error message
        qDebug() << "LoadIconW failed with error: " << error;
    }

    // Additional checks
    if (nid.hWnd == NULL) {
        // Error handling: Invalid window handle
        // Add code to log or display an error message
        qDebug() << "Invalid window handle obtained from winId()";
    }

    if (!Shell_NotifyIcon(NIM_ADD, &nid)) {
        // Error handling: Unable to add the icon to the system tray
        DWORD error = GetLastError();
        // Add code to log or display the error message
        qDebug() << "Shell_NotifyIcon failed with error: " << error;
    }


    connect(spotmngr, SIGNAL(updateReq()), this, SLOT(updateCounts()));
    connect(spotmngr, SIGNAL(stopedSpot()), this, SLOT(showThis()));


//    // Handle the tray icon context menu
//    connect(this, &MainWindow::customContextMenuRequested, this, &MainWindow::showContextMenu);
//    connect(this, &MainWindow::mousePressEvent, this, &MainWindow::onTrayIconClicked);

    on_pushButton_clicked();

    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    ui->actionAutostart->setChecked( settings.contains("SpotifyEnhancer") );
    ui->label_exepath->setText( spotmngr->getExePath() );

}

MainWindow::~MainWindow()
{
    delete spotmngr;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
    {
        QMessageBox msgBox(QMessageBox::Question, "Bestätigung", "Was möchten Sie tun?", QMessageBox::NoButton, this);
        QPushButton* hideButton = msgBox.addButton("Fenster verstecken", QMessageBox::ActionRole);
        QPushButton* exitButton = msgBox.addButton("Programm beenden", QMessageBox::RejectRole);
        msgBox.exec();

        if (msgBox.clickedButton() == hideButton)
        {
            event->ignore();  // Das Schließen des Fensters ignorieren
            hide();          // Das Fenster verstecken
        }
        else if (msgBox.clickedButton() == exitButton)
        {
            event->accept();  // Das Programm beenden
        }
    }

bool MainWindow::nativeEvent(const QByteArray &, void *message, qintptr *)
{
    MSG* msg = static_cast<MSG*>(message);
    if (msg->message == WM_TRAYICON)
    {
        if (msg->lParam == WM_RBUTTONUP || msg->lParam == WM_LBUTTONUP )
        {
            POINT cursorPos;
            GetCursorPos(&cursorPos);
            SetForegroundWindow((HWND)winId());
            trayMenu->exec(QPoint(cursorPos.x, cursorPos.y));
        }
    }
    return false;
}

//void MainWindow::showContextMenu(const QPoint &pos)
//{
//    QPoint globalPos = mapToGlobal(pos);
//    trayMenu->exec(globalPos);
//}

//void MainWindow::onTrayIconClicked(QMouseEvent* event)
//{
//    if (event->button() == Qt::LeftButton)
//    {
//        qDebug() << "CLICK";
//        // Handle left-click event here
//        // Show or hide the window, for example
//        if (isHidden())
//            show();
//        else
//            hide();
//    }
//    else if (event->button() == Qt::RightButton)
//    {
//        // Handle right-click event here
//        // Show the context menu
//        QPoint pos = mapFromGlobal(event->globalPos());
//        emit customContextMenuRequested(pos);
//    }
//}

void MainWindow::onOption1Clicked()
{
    // Handle Option 1 clicked
    this->show();
}

void MainWindow::onOption2Clicked()
{
    // Handle Option 2 clicked
    QApplication::exit(0);
}

void MainWindow::acstatusclicked()
{
    on_pushButton_clicked();
}

void MainWindow::startSpotSlot()
{
    if( spotmngr->startSpotify() ) {
        v_window v(nullptr, "Spotify.exe", "", "");
        if( spotmngr->searchSpotifyWindow(&v, 5) ) {
            spotmngr->sendPlaySignal(v);
        }
    }

}




void MainWindow::stopSpotSlot()
{
    v_window v(nullptr, "Spotify.exe", "", "");
    if( spotmngr->searchSpotifyWindow(&v, 1) ) {
        spotmngr->stopSpotify(v);
    }
}



void MainWindow::on_actionSpotify_Pfad_setzen_triggered()
{
    QString appDataRoamingPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    appDataRoamingPath.remove(appDataRoamingPath.lastIndexOf("/"), appDataRoamingPath.length() - appDataRoamingPath.lastIndexOf("/"));
    appDataRoamingPath += "/Spotify/" ;
    QUrl file;
    if( ( file = QFileDialog::getOpenFileUrl(this, "Executable Pfad Spotify", QUrl::fromLocalFile(appDataRoamingPath) , "*.exe") ).isLocalFile() ) {
        spotmngr->setExePath(file.toLocalFile());
    }

    ui->label_exepath->setText( spotmngr->getExePath() );
}


void MainWindow::on_pushButton_clicked()
{
    this->ui->pushButton->setDisabled(true);
    if(this->spotmngr->isRunning()) {
        this->spotmngr->stopThread();
        this->ui->pushButton->setText("Start");
        this->acstatus->setChecked(false);

    } else {
        this->spotmngr->startThread();
        this->ui->pushButton->setText("Stop");
        std::cout << "STARTED" << std::endl;
        this->acstatus->setChecked(true);

    }

    this->ui->pushButton->setDisabled(false);
}


void MainWindow::on_actionAutostart_triggered(bool checked)
{
    QString programName = "SpotifyEnhancer";
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);

    if(checked) {
        QString programPath = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
        if( ! settings.isWritable()) {
            std::cout << "NICHT SCHREIBBAR!!!" << std::endl;
        } else
            settings.setValue(programName, programPath);
        if( ! settings.contains(programName)) {
            std::cout << "autostart setzten fehlgeschlagen!" << std::endl;
        }
    } else {
        if( ! settings.isWritable()) {
            std::cout << "NICHT SCHREIBBAR!!!" << std::endl;
        } else
            settings.remove(programName);
        if( settings.contains(programName)) {
            std::cout << "autostart entfernen fehlgeschlagen!" << std::endl;
        }

    }
}

void MainWindow::updateCounts()
{
    this->ui->spinBox->setValue( spotmngr->getCounts() );
}

void MainWindow::showThis()
{
    if( ! this->spotmngr->isRunning()) {
        this->ui->pushButton->setText("Start");
        std::cout << "STOPED" << std::endl;

    } else {
        this->ui->pushButton->setText("Stop");

    }
}


void MainWindow::on_pushButton_2_clicked()
{
    this->hide();
}



void MainWindow::on_spinBox_ms_checkrate_valueChanged(int arg1)
{
    this->spotmngr->setMs_checkrate(arg1);
}


QString formatiereSekunden(int sekunden)
{
    QTime zeit(0, 0, 0);
    zeit = zeit.addSecs(sekunden);

    int stunden = zeit.hour();
    int minuten = zeit.minute();
    int restlicheSekunden = zeit.second();

    QString formatierteZeit;
    if (stunden > 0)
    {
        formatierteZeit += QString::number(stunden) + " Stunde" + (stunden > 1 ? "n " : " ");
    }
    if (minuten > 0)
    {
        formatierteZeit += QString::number(minuten) + " Minute" + (minuten > 1 ? "n " : " ");
    }
    if (restlicheSekunden > 0 || formatierteZeit.isEmpty())
    {
        formatierteZeit += QString::number(restlicheSekunden) + " Sekunde" + (restlicheSekunden > 1 ? "n" : "");
    }

    return formatierteZeit.trimmed();
}



void MainWindow::on_actionGesammtzahl_Werbungen_triggered()
{
    int ges = spotmngr->getGesammtAnzahl();
    QMessageBox::information(this, "Gesamtanzahl übersprungener Werbungen", "Insgesamt hast du bereits " + QString::number(ges) + " Werbungen übersprungen!\nDamit hast mehr als du dir mehr als " + formatiereSekunden(ges * 30) + " Werbung erspart!" );
}


void MainWindow::on_action_ber_triggered()
{
    QMessageBox::information(this, "Über", "M4RKUS!\nVersion: " + this->version );
}

