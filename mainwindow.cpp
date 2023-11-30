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

#include "stylehandler.h"





MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon(":/spotify-32.ico"));

    // Create SpotifyManager instance
    spotmngr = new SpotifyManager();
    this->ui->spinBox_ms_checkrate->setValue( spotmngr->load_and_getMs_checkrate() );

//    std::cout << "this: " << this->winId() << std::endl;



    SpotifyManager::WindowHandle wh;
    if( (wh = spotmngr->alreadyRunningServiceAvaible()) ) {
        qDebug() << "Already running service -> try to make it visible...";
        qDebug() << "Showing hidden service... " <<  (spotmngr->showHiddenService(wh) ? "Success" : "Failed");
        exit(0);
    }

    // Initialize the system tray icon and menu
    trayMenu = new QMenu(this);
    trayMenu->setTitle("Spotify Enhancer");
//    trayMenu->setIcon(QIcon(":/spotify-32.ico"));

    actionOption1 = trayMenu->addAction("Fenster anzeigen");
    actionOption1->setIcon(QIcon(":/maximize_icon.png"));

    trayMenu->addSeparator();

    acstatus = trayMenu->addAction("Werbung entfernen           ");
    acstatus->setCheckable(true);
    acstatus->setChecked(true);
    trayMenu->addSeparator();

    startSpot = trayMenu->addAction("Spotify starten            ");
    startSpot->setIcon(QIcon(":/spotify_original_green.png"));
    trayMenu->addSeparator();

    stopSpot = trayMenu->addAction("Spotify beenden             ");
    stopSpot->setIcon(QIcon(":/close_icon.png"));
    trayMenu->addSeparator();

    actionOption2 = trayMenu->addAction("Programm beenden           ");
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
//    nid.hIcon = LoadIconW(NULL, IDI_APPLICATION);
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

//    connect(spotmngr, SIGNAL(updateReq()), this, SLOT(updateCounts()));
    connect(spotmngr, SIGNAL(stopedSpot()), this, SLOT(showThis()));

//    // Handle the tray icon context menu
//    connect(this, &MainWindow::customContextMenuRequested, this, &MainWindow::showContextMenu);
//    connect(this, &MainWindow::mousePressEvent, this, &MainWindow::onTrayIconClicked);

    on_radioButtonStstaus_clicked(ui->radioButtonStstaus->isChecked());


    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    ui->actionAutostart->setChecked( settings.contains("SpotifyEnhancer") );
    //update exe path if changed!
    on_actionAutostart_triggered(ui->actionAutostart->isChecked());
    ui->label_exepath->setText( spotmngr->getExePath() );

    dialogUeber = new DialogUeber( ( (false) ? QApplication::applicationDirPath() : "C:\\Program Files\\SpotifyEnhancer\\bin" ) + "/../SpotifyEnhancerMaintenanceTool.exe",
                                  "M4RKUS", "SpotifyEnhancer", this->version, QColor::fromRgb(0, 171, 255), this, true);
    dialogUeber->setPixmap(QPixmap::fromImage(QImage("://spot_ico_blue_x512.ico")).scaled(96, 96));
    dialogUeber->setDescription("https://code.obermui.de/markus/SpotifyEnhancer", QFile(":/description.txt"), "code.obermui.de/markus/SpotifyEnhancer");
    dialogUeber->setLicence(QFile(":/LICENSE.txt"));
    dialogUeber->setIssueWebsite("https://code.obermui.de/markus/SpotifyEnhancer/issues");



    ui->actionAutomatisch_nach_Updates_suchen->setChecked(dialogUeber->updater()->getAutoSearchForUpdateStatus());

    styleHandler = new StyleHandler("M4RKUS", "SpotifyEnhancer");
    ui->designl->addWidget(styleHandler->getCombobox());

//    qApp->setStyleSheet("QWidget#widget{ background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1,"
//                        " stop:0 rgba(12, 169, 239, 100), stop:1 rgba(255, 255, 255, 0))\n}");

//    // Create a linear gradient
//    QLinearGradient gradient(0, 0, 300, 300);
//    gradient.setColorAt(0, QColor(12, 169, 239, 100));
//    gradient.setColorAt(1, QColor(255, 255, 255, 0));

//    // Create a brush with the gradient
//    QBrush brush(gradient);

//    // Create a palette
//    QPalette palette = ui->widget->palette();

//    // Set the brush to the desired role
//    palette.setBrush(QPalette::Window, brush);
//    palette.setBrush(QPalette::Base, brush);
//    palette.setBrush(QPalette::AlternateBase, brush);

//    // Apply the modified palette to the widget
//    ui->widget->setPalette(palette);



    QSettings s("M4RKUS", "SpotifyEnhancer");
    bool val = s.value("special_methode", false).toBool();
    spotmngr->setUse_special_methode(val);
    ui->actionForceForeground->setChecked(val);

    val = s.value("colored_background", true).toBool();
    setColoredBackground(val);
    ui->actionColored_Background->setChecked(val);

    int i_val = s.value("playdelay", 500).toInt();
    spotmngr->setPlayDelay(i_val);
    ui->spinBoxPlayDelay->setValue(i_val);

//    startTimer(3000);
}



void MainWindow::on_actionForceForeground_triggered()
{
    QSettings s("M4RKUS", "SpotifyEnhancer");
    s.setValue("special_methode", ui->actionForceForeground->isChecked());
    spotmngr->setUse_special_methode(ui->actionForceForeground->isChecked());
}

void MainWindow::on_actionColored_Background_triggered(bool checked)
{
    setColoredBackground(checked);
    QSettings s("M4RKUS", "SpotifyEnhancer");
    s.setValue("colored_background", checked);
}



void MainWindow::on_spinBoxPlayDelay_valueChanged(int arg1)
{
    QSettings s("M4RKUS", "SpotifyEnhancer");
    s.setValue("playdelay", arg1);
    spotmngr->setPlayDelay(arg1);
}



MainWindow::~MainWindow()
{
    delete spotmngr;
    delete dialogUeber;
    delete trayMenu;
    delete ui;
}


//void MainWindow::timerEvent(QTimerEvent *event)
//{
//    if( spotmngr->startSpotify() ) {
//        v_window v(nullptr, "Spotify.exe", "", "");
//        if( spotmngr->searchSpotifyWindow(&v, 5) ) {
//            spotmngr->sendPlaySignal(v);
//        }  else {
//            qDebug() << "sendPlaySignal failed";
//        }
//    } else {
//        qDebug() << "startSpotify failed";
//    }
//}


void MainWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox msgBox(QMessageBox::Question, "Bestätigung", "Was möchten Sie tun?", QMessageBox::NoButton, this);
    QPushButton* aboardButton = msgBox.addButton("Abbrechen", QMessageBox::ActionRole);
    QPushButton* hideButton = msgBox.addButton("Fenster verstecken", QMessageBox::ActionRole);
    QPushButton* exitButton = msgBox.addButton("Programm beenden", QMessageBox::RejectRole);

    msgBox.setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    msgBox.exec();


    if (msgBox.clickedButton() == hideButton)
    {
        QApplication::setQuitOnLastWindowClosed(true);

        event->ignore();  // Das Schließen des Fensters ignorieren
        hide();          // Das Fenster verstecken
        qDebug() << "a1";
        return;
    }
    else if (msgBox.clickedButton() == exitButton)
    {
        event->accept();  // Das Programm beenden

        QApplication::quit();

        qDebug() << "a2";

    } else if(msgBox.clickedButton() == aboardButton || msgBox.clickedButton() == nullptr)
    {
        event->ignore();  // Das Schließen des Fensters ignorieren
        qDebug() << "a3";

        return;
    }
}







bool MainWindow::nativeEvent(const QByteArray &, void *message, qintptr * result)
{
    MSG* msg = static_cast<MSG*>(message);
    if (msg->message == WM_TRAYICON)
    {
        if (msg->lParam == WM_RBUTTONUP || msg->lParam == WM_LBUTTONUP )
        {
//            POINT cursorPos;
//            if (!GetCursorPos(&cursorPos))
//            {
//                // Failed to get cursor position
//                DWORD error = GetLastError();
//                qDebug() << "Failed to get cursor position. Error code:" << error;
//            }

            HWND windowHandle = (HWND)winId();
            if (!SetForegroundWindow(windowHandle))
            {
                // Failed to set foreground window
                DWORD error = GetLastError();
                qDebug() << "Failed to set foreground window. Error code:" << error;
            }
            trayMenu->exec(/*QPoint(cursorPos.x, cursorPos.y)*/QCursor::pos());
        }
    }


    if (msg->message == WM_USER + 2) {
        qDebug() << "Custom message received!";

        if (msg->lParam == 187)
        {
            qDebug() << "Recieved Signal 'SHOW'";
            *result = 187; // Set the result
            this->show();
            return true; // Indicate that the message was handled
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
    qDebug() << this->acstatus->isChecked();
    on_radioButtonStstaus_clicked(this->acstatus->isChecked());
}

void MainWindow::startSpotSlot()
{
    if( spotmngr->startSpotify() ) {
        v_window v(nullptr, "Spotify.exe", "", "");
        if( spotmngr->searchSpotifyWindow(&v, 5) ) {
            spotmngr->sendPlaySignal(v);
        }  else {
            qDebug() << "sendPlaySignal failed";
        }
    } else {
        qDebug() << "startSpotify failed";
    }

}




void MainWindow::stopSpotSlot()
{
    v_window v(nullptr, "Spotify.exe", "", "");
    if( spotmngr->searchSpotifyWindow(&v, 1) ) {
        if( ! spotmngr->stopSpotify(v) ) {
            qDebug() << "stopSpotify failed";

        }
    } else {
        qDebug() << "searchSpotifyWindow failed";
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
        std::cout << "autostart updated to "<< programPath.toStdString() << std::endl;
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

void MainWindow::setColoredBackground(bool status)
{
    if(status) {
        this->setStyleSheet("QWidget#MainWindow { \nbackground-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(0, 114, 255, 78), stop:1 rgba(96, 207, 212, 113));\n}");
        this->ui->label->setStyleSheet("color: rgb(255, 255, 255);");
        this->ui->label_2->setStyleSheet("color: rgb(255, 255, 255);");
        this->ui->label_3->setStyleSheet("color: rgb(255, 255, 255);");
        this->ui->label_4->setStyleSheet("color: rgb(255, 255, 255);");
        this->ui->label_5->setStyleSheet("color: rgb(255, 255, 255);");
        this->ui->radioButtonStstaus->setStyleSheet("QRadioButton{color: rgb(255, 255, 255);}\n QRadioButton::indicator {\n    width: 20px;\n    height: 20px;\n}\n\nQRadioButton::indicator::unchecked {\n    image: url(:/redc.png);\n}\n\nQRadioButton::indicator::checked {\n    image: url(:/greenc.png);\n}");


    } else {
        this->setStyleSheet("");
        this->ui->label->setStyleSheet("");
        this->ui->label_2->setStyleSheet("");
        this->ui->label_3->setStyleSheet("");
        this->ui->label_4->setStyleSheet("");
        this->ui->label_5->setStyleSheet("");
        this->ui->radioButtonStstaus->setStyleSheet("QRadioButton::indicator {\n    width: 20px;\n    height: 20px;\n}\n\nQRadioButton::indicator::unchecked {\n    image: url(:/redc.png);\n}\n\nQRadioButton::indicator::checked {\n    image: url(:/greenc.png);\n}");

    }
}





void MainWindow::showThis()
{
    if( ! this->spotmngr->isRunning()) {
        this->ui->radioButtonStstaus->setChecked(false);
        std::cout << "STOPED" << std::endl;

    } else {
        this->ui->radioButtonStstaus->setChecked(true);

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
    dialogUeber->exec();
}



void MainWindow::on_radioButtonStstaus_clicked(bool status)
{
    this->ui->radioButtonStstaus->setDisabled(true);
    if(!status) {
        if( this->spotmngr->isRunning() ) {
            this->spotmngr->stopThread();
            std::cout << "stoped" << std::endl;
        }
    } else {
        if(! this->spotmngr->isRunning() ) {
            this->spotmngr->startThread();
            std::cout << "STARTED" << std::endl;
        }
    }
    ui->radioButtonStstaus->setChecked(status);
    this->acstatus->setChecked(status);

    this->ui->radioButtonStstaus->setDisabled(false);
}

#include <QApplication>
void MainWindow::on_pushButtonExit_clicked()
{
//    QCloseEvent *closeEvent = new QCloseEvent();
//    //closeEvent->accept(); // You can set the accepted state as needed
//    QApplication::sendEvent(this, closeEvent);
    this->close();
}



void MainWindow::on_radioButtonStstaus_toggled(bool checked)
{
    ui->radioButtonStstaus->setText(checked ? " Aktiv " : "Inaktiv");
}



