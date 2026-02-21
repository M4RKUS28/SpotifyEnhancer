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
#include <QProcess>

#include "stylehandler.h"

#include "menue.h"



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon(":/spotify-32.ico"));

    // Create SpotifyManager instance
    spotmngr = new SpotifyManager();
    int test_rate = spotmngr->load_and_getMs_checkrate();
    if(test_rate <= TEST_INTERVALL_HOCH)
        ui->comboBoxTestRate->setCurrentIndex(0);
    else if(test_rate <= TEST_INTERVALL_MITTEL)
        ui->comboBoxTestRate->setCurrentIndex(1);
    else
        ui->comboBoxTestRate->setCurrentIndex(2);

//    std::cout << "this: " << this->winId() << std::endl;

    QSettings s("M4RKUS", "SpotifyEnhancer");

    SpotifyManager::WindowHandle wh;
    if( (wh = spotmngr->alreadyRunningServiceAvaible()) ) {
        qDebug() << "Already running service -> try to make it visible...";
        qDebug() << "Showing hidden service... " <<  (spotmngr->showHiddenService(wh) ? "Success" : "Failed");
        exit(0);
    }


    // Initialize the system tray icon and menu
    trayMenu = new Menu(this);

    trayMenu->setTitle("Spotify Enhancer");
    // trayMenu->setIcon(QIcon(":/icons/spotify-32.ico"));

    actionOption1 = trayMenu->addAction("Fenster anzeigen");
    actionOption1->setIcon(QIcon(":/icons/maximize_icon.png"));

    acstatus = trayMenu->addAction("Werbung entfernen           ");
    acstatus->setCheckable(true);
    acstatus->setChecked(true);

    trayMenu->addSeparator();

    startSpot = trayMenu->addAction("Spotify starten / pausieren");
    startSpot->setIcon(QIcon(":/icons/spotify_original_green.png"));

    stopSpot = trayMenu->addAction("Spotify beenden             ");
    stopSpot->setIcon(QIcon(":/icons/close_icon.png"));
    trayMenu->addSeparator();

    actionOption2 = trayMenu->addAction("Programm beenden           ");
    actionOption2->setIcon(QIcon(":/icons/beenden.png"));

    connect(actionOption1, &QAction::triggered, this, &MainWindow::onOption1Clicked);
    connect(actionOption2, &QAction::triggered, this, &MainWindow::onOption2Clicked);
    connect(acstatus, &QAction::triggered, this, &MainWindow::acstatusclicked);
    connect(startSpot, &QAction::triggered, this, &MainWindow::startSpotSlot);
    connect(stopSpot, &QAction::triggered, this, &MainWindow::stopSpotSlot);


    //setup switch button
    // switchbtn = new Switch(this);
    // ui->horizontalLayout_swizchbuton->addWidget(switchbtn);
    // connect(switchbtn, SIGNAL(toggled(bool)), this, SLOT(toggledWinWaterMarkSwitchBtn(bool)));
    // switchbtn->setChecked( s.value("toggledWinWaterMarkSwitchBtn", false).toBool() );
    // removeWaterMark(switchbtn->isChecked()
    //                     ? REMOVE_TO_DO_WATER_MARK::AUTO_ON_START_UP : REMOVE_TO_DO_WATER_MARK::NONE);

    appbackgroundBtn = new Switch(this);
    ui->horizontalLayout_4_app_backgroundBtn->addWidget(appbackgroundBtn);
    connect(appbackgroundBtn, &QAbstractButton::toggled, this, &MainWindow::toggledBackgroundBtn);


    autoStartBtn = new Switch(this);
    ui->horizontalLayout_autostartBtn->addWidget(autoStartBtn);
    connect(autoStartBtn, &QAbstractButton::toggled, this, &MainWindow::toggledAutostartBtn);

    status_btn = new Switch(this);
    ui->horizontalLayout_7statusbtn->addWidget(status_btn);
    connect(status_btn, &QAbstractButton::toggled, this, &MainWindow::toggledstatus_btn);


    transition_btn = new Switch(this);
    ui->horizontalLayout_tranisition->addWidget(transition_btn);
    connect(transition_btn, &QAbstractButton::toggled, this, &MainWindow::toggled_transition_btn);
    transition_btn->setChecked( s.value("transition_btn", false).toBool() );
    ui->transisition_status_label->setText(transition_btn->isChecked() ? "Ein" : "Aus");
    this->spotmngr->setTransitionWaitingTime(transition_btn->isChecked() ? DELAY_TRANSITION : 0);

    //OS THEME THINGS

    //changeevent
    connect(QApplication::styleHints(), &QStyleHints::colorSchemeChanged, this, &MainWindow::colorSchemeChanged);

    //combobox default
    ui->comboBoxOSTheme->setCurrentIndex( (qApp->styleHints()->colorScheme() == Qt::ColorScheme::Light) ? 0 : 1 );

    // QGraphicsBlurEffect* p_blur = new QGraphicsBlurEffect;
    // p_blur->setBlurRadius(3);
    // p_blur->setBlurHints(QGraphicsBlurEffect::QualityHint);
    // ui->groupBox->setGraphicsEffect(p_blur);


    QFile::copy(":/icons/spotify-32.ico",QCoreApplication::applicationDirPath() + "/spotify-32.ico");


    // Set up the NOTIFYICONDATA structure
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
    connect(spotmngr, &SpotifyManager::stopedSpot, this, &MainWindow::showThis);

//    // Handle the tray icon context menu
//    connect(this, &MainWindow::customContextMenuRequested, this, &MainWindow::showContextMenu);
//    connect(this, &MainWindow::mousePressEvent, this, &MainWindow::onTrayIconClicked);

    on_radioButtonStstaus_clicked(ui->radioButtonStstaus->isChecked());

    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    ui->actionAutostart->setChecked( settings.contains("SpotifyEnhancer") );
    ui->autostartBtnMsg->setText(settings.contains("SpotifyEnhancer") ? "Ein " : "Aus ");
    //update exe path if changed!
    autoStartBtn->setChecked(settings.contains("SpotifyEnhancer"));
    on_actionAutostart_triggered(ui->actionAutostart->isChecked());
    ui->label_exepath_2->setText( spotmngr->getExePath() );

    dialogUeber = new DialogUeber( QCoreApplication::applicationDirPath() + "/../SpotifyEnhancerMaintenanceTool.exe",
                                  "M4RKUS", "SpotifyEnhancer", this->version, QColor::fromRgb(0, 171, 255), this, true);
    dialogUeber->setPixmap(QPixmap::fromImage(QImage(":/icons/spot_ico_blue_x512.ico")).scaled(96, 96));
    dialogUeber->setDescription("https://code.obermui.de/markus/SpotifyEnhancer", QFile(":/description.txt"), "code.obermui.de/markus/SpotifyEnhancer");
    dialogUeber->setLicence(QFile(":/LICENSE.txt"));
    dialogUeber->setIssueWebsite("https://code.obermui.de/markus/SpotifyEnhancer/issues");
    dialogUeber->setUpdaterFinishedMsgBoxFilePath("./news.txt");

    ui->actionAutomatisch_nach_Updates_suchen->setChecked(dialogUeber->updater()->getAutoSearchForUpdateStatus());

    dialogUeber->styleHandler()->setFilteredStyles(
        QStringList::fromList(QList<QString>{"Fusion", "Fusion_OWN_dark_gray", "Fusion_OWN_dark_blue2", "QDarkStyleDark", "QDarkStyleLight"}));
    auto w = dialogUeber->styleHandler()->getCombobox();
    w->setFont(ui->comboBoxOSTheme->font());
    ui->designl->addWidget(w);
    connect(dialogUeber->styleHandler(), SIGNAL(currentStyleChanged()), this, SLOT(styleChanged()));

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



    bool val = s.value("special_methode", false).toBool();
    spotmngr->setUse_special_methode(val);
    ui->actionForceForeground->setChecked(val);

    val = s.value("colored_background", true).toBool();
    setColoredBackground(val);
    ui->actionColored_Background->setChecked(val);
    ui->colorbackgroundmsg->setText(val ? "Ein " : "Aus ");
    appbackgroundBtn->setChecked(val);

    int i_val = s.value("playdelay", 500).toInt();
    spotmngr->setPlayDelay(i_val);

    if(i_val <= DELAY_INTERVALL_AUS)
        ui->comboBox_delayTime->setCurrentIndex(0);
    else if(i_val <= DELAY_INTERVALL_GERING)
        ui->comboBox_delayTime->setCurrentIndex(1);
    else if(i_val <= DELAY_INTERVALL_MITTEL)
        ui->comboBox_delayTime->setCurrentIndex(2);
    else
        ui->comboBox_delayTime->setCurrentIndex(3);


//    startTimer(3000);
}


void MainWindow::toggledAutostartBtn(bool checked)
{
    bool oldState = ui->actionAutostart->blockSignals(true);
    ui->actionAutostart->setChecked(checked);
    ui->autostartBtnMsg->setText(checked ? "Ein " : "Aus ");
    ui->actionAutostart->blockSignals(oldState);

    on_actionAutostart_triggered(checked);

}

void MainWindow::toggledBackgroundBtn(bool checked)
{
    bool oldState = ui->actionColored_Background->blockSignals(true);
    ui->actionColored_Background->setChecked(checked);
    ui->colorbackgroundmsg->setText(checked ? "Ein " : "Aus ");
    ui->actionColored_Background->blockSignals(oldState);
    setColoredBackground(checked);
    QSettings s("M4RKUS", "SpotifyEnhancer");
    s.setValue("colored_background", checked);
}


void MainWindow::toggledstatus_btn(bool checked)
{
    ui->radioButtonStstaus->setChecked(checked);
}

void MainWindow::toggled_transition_btn(bool checked)
{
    QSettings s("M4RKUS", "SpotifyEnhancer");
    s.setValue("transition_btn", checked);
    ui->transisition_status_label->setText(transition_btn->isChecked() ? "Ein" : "Aus");
    this->spotmngr->setTransitionWaitingTime(transition_btn->isChecked() ? DELAY_TRANSITION : 0);
}

void MainWindow::on_actionForceForeground_triggered()
{
    QSettings s("M4RKUS", "SpotifyEnhancer");
    s.setValue("special_methode", ui->actionForceForeground->isChecked());
    spotmngr->setUse_special_methode(ui->actionForceForeground->isChecked());
}

void MainWindow::styleChanged()
{
    setColoredBackground(appbackgroundBtn->isChecked());
}

void MainWindow::on_actionColored_Background_triggered(bool checked)
{
    setColoredBackground(checked);
    QSettings s("M4RKUS", "SpotifyEnhancer");
    s.setValue("colored_background", checked);
}




MainWindow::~MainWindow()
{
    Shell_NotifyIcon(NIM_DELETE, &nid);
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
    QPushButton* hideButton = msgBox.addButton(  QString((dialogUeber->styleHandler()->currentStyleUsesStyleSheets() ? QString("") : "Fenster ") + "verstecken"), QMessageBox::ActionRole);
    QPushButton* exitButton = msgBox.addButton( QString((dialogUeber->styleHandler()->currentStyleUsesStyleSheets() ? QString("") : "Programm ") + "beenden"), QMessageBox::RejectRole);

    msgBox.setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    msgBox.exec();

    if (msgBox.clickedButton() == hideButton)
    {
        QApplication::setQuitOnLastWindowClosed(true);
        event->ignore();
        hide();
        return;
    }
    else if (msgBox.clickedButton() == exitButton)
    {
        event->accept();
        QApplication::quit();
    } else if(msgBox.clickedButton() == aboardButton || msgBox.clickedButton() == nullptr)
    {
        event->ignore();
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

    ui->label_exepath_2->setText( spotmngr->getExePath() );

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

    autoStartBtn->setChecked(checked);

}

void MainWindow::colorSchemeChanged(Qt::ColorScheme)
{
    setColoredBackground(appbackgroundBtn->isChecked());
    ui->comboBoxOSTheme->setCurrentIndex(  (qApp->styleHints()->colorScheme() == Qt::ColorScheme::Light) ? 0 : 1 );
}

void MainWindow::setColoredBackground(bool status)
{
    bool donotuseStylesheets = (dialogUeber->styleHandler()->currentStyleUsesStyleSheets());
    appbackgroundBtn->setChecked(donotuseStylesheets ? false : status);

    if(status) {
        this->setStyleSheet( donotuseStylesheets ? "" : "QWidget#MainWindow,QScrollArea { \n"
                                                       "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1,"
                                                       " stop:0 rgba(0, 114, 255, 78),"
                                                       " stop:1 rgba(96, 207, 212, 113));"
                                                       "\n} "
                                                "QGroupBox {"
                                                       " background-color: rgba(0, 0, 0, 80);"
                                                       " border-radius: 5px;"
                                                       " margin-top: 32px;"
                                                       " padding-top: 0px;"
                                                       " padding-bottom: 0px;"
                                                       " margin-bottom: 12px;"
                                                " }");
        this->ui->groupBox->setStyleSheet(donotuseStylesheets ? "" : "QLabel,QGroupBox{color: rgb(255, 255, 255);}");
        this->ui->groupBox_2->setStyleSheet(donotuseStylesheets ? "" : "QLabel,QGroupBox{color: rgb(255, 255, 255);}");
        this->ui->groupBox_3->setStyleSheet(donotuseStylesheets ? "" : "QLabel,QGroupBox{color: rgb(255, 255, 255);}");
        this->ui->groupBox_4->setStyleSheet(donotuseStylesheets ? "" : "QLabel,QGroupBox{color: rgb(255, 255, 255);}");
        this->ui->groupBox_5->setStyleSheet(donotuseStylesheets ? "" : "QLabel,QGroupBox{color: rgb(255, 255, 255);}");
        this->ui->radioButtonStstaus->setStyleSheet(donotuseStylesheets ? "" : "QRadioButton{color: rgb(255, 255, 255);}\n QRadioButton::indicator {\n    width: 20px;\n    height: 20px;\n}\n\nQRadioButton::indicator::unchecked {\n    image: url(:/icons/redc.png);\n}\n\nQRadioButton::indicator::checked {\n    image: url(:/icons/greenc.png);\n}");
        this->setMinimumHeight(430);
        this->setMaximumHeight(800);
    } else {
        this->setStyleSheet(donotuseStylesheets ? "" : "QGroupBox{ "
                                                       "background-color: palette(Base);"
                                                       " border-radius: 5px;"
                                                       " margin-top: 32px;"
                                                       " padding-top: 0px;"
                                                       " padding-bottom: 0px;"
                                                       " margin-bottom: 12px;"
                                                       " }");
        this->ui->groupBox->setStyleSheet("");
        this->ui->groupBox_2->setStyleSheet("");
        this->ui->groupBox_3->setStyleSheet("");
        this->ui->groupBox_4->setStyleSheet("");
        this->ui->groupBox_5->setStyleSheet("");
        this->ui->radioButtonStstaus->setStyleSheet(donotuseStylesheets ? "" : "QRadioButton::indicator {\n    width: 20px;\n    height: 20px;\n}\n\nQRadioButton::indicator::unchecked {\n    image: url(:/icons/redc.png);\n}\n\nQRadioButton::indicator::checked {\n    image: url(:/icons/greenc.png);\n}");
        this->setMinimumHeight(470);
        this->setMaximumHeight(800);
    }

    trayMenu->setStyleSheet(donotuseStylesheets ? "" :
        "\
        QMenu {\
                background-color: palette(Base); \
                border-radius: 6px;\
                border: 1px solid palette(Window);\
                padding: 6px; \
        }\
        QMenu::item{\
                padding-top: 6px;\
                padding-left: 13px;\
                padding-right: 10px;\
                padding-bottom: 6px;\
                margin-bottom: 2px; \
                margin-top: 1px; \
        } \
        QMenu::icon {\
                padding-right: 16px;\
        }\
        QMenu::item:selected {\
                background: palette(Window); /*rgb(40, 40, 40);*/ \
                border-radius: 4px;\
        } \
        QMenu::separator { \
                padding: 5px; \
        } \
        QMenu::indicator { \
                left: 8px; \
        } \
        "
        );


    ui->scrollArea->verticalScrollBar()->setStyleSheet(donotuseStylesheets ? "" : ""
     "QScrollBar:vertical {      \
         border: 0px solid palette(window);   \
         background: transparent;   \
         width: 8px;   \
         margin: 0;   \
         border-radius: 10px; \
     }   \
     QScrollBar::handle:vertical {   \
         background: white;   \
         min-height: 20px;   \
         border: 0px solid red;   \
         border-radius: 4px;   \
         background-color: palette(base);   \
     }   \
     \
     QScrollBar::add-line:vertical {   \
         border: none;   \
         background: #32CC99;   \
         height: 0px;   \
         subcontrol-position: bottom;   \
         subcontrol-origin: margin;   \
     }   \
     \
     QScrollBar::sub-line:vertical {   \
         border: none;   \
         background: #32CC99;   \
         height: 0px;   \
         subcontrol-position: top;   \
         subcontrol-origin: margin;   \
     }   \
     QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical {   \
         width: 0px;   \
         height: 0px;   \
         border: none;   \
         background: transparent;   \
     }   \
    \
    QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {   \
        background: none;   \
    }   \
     ");
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
    int ges = spotmngr->getGesamtAnzahl();
    QMessageBox::information(this, "Gesamtanzahl übersprungener Werbungen", "Insgesamt hast du bereits " + QString::number(ges) + " Werbungen übersprungen!\nDamit hast mehr als du dir mehr als " + formatiereSekunden(ges * 30) + " Werbung erspart!" );
}

void MainWindow::on_action_ber_triggered()
{
    dialogUeber->exec();
}



void MainWindow::on_radioButtonStstaus_clicked(bool status)
{

    bool oldState = status_btn->blockSignals(true);
    status_btn->setChecked(status);
    status_btn->blockSignals(oldState);


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

void MainWindow::on_pushButtonExit_clicked()
{
    this->close();
}



void MainWindow::on_radioButtonStstaus_toggled(bool checked)
{
    ui->radioButtonStstaus->setText(checked ? "Ein " : "Aus ");
}

void MainWindow::on_pushButtonChangePath_clicked()
{
    ui->actionSpotify_Pfad_setzen->trigger();
}

void MainWindow::on_comboBoxOSTheme_currentIndexChanged(int index)
{
    QSysInfo inf;
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                       QSettings::NativeFormat);

    if(inf.productVersion() == "10") {



    } else if(inf.productVersion() == "11") {

        settings.setValue("SystemUsesLightTheme", (index == 0 /*lighttheme*/) ? 1 : 0);
        settings.setValue("AppsUseLightTheme", (index == 0 /*lighttheme*/) ? 1 : 0);
    }


}



void MainWindow::on_actionAuf_Updates_pr_fen_triggered()
{
    this->dialogUeber->updater()->checkForUpdates(true, false);
}


void MainWindow::on_comboBoxTestRate_currentIndexChanged(int index)
{
    int ms = index == 0 ? TEST_INTERVALL_HOCH : index == 1 ? TEST_INTERVALL_MITTEL : TEST_INTERVALL_GERING;
    this->spotmngr->setMs_checkrate(ms);
}


void MainWindow::on_comboBox_delayTime_currentIndexChanged(int index)
{
    int ms = index == 0 ? DELAY_INTERVALL_AUS : index == 1 ? DELAY_INTERVALL_GERING : index == 2 ? DELAY_INTERVALL_MITTEL : DELAY_INTERVALL_HOCH;
    QSettings s("M4RKUS", "SpotifyEnhancer");
    s.setValue("playdelay", ms);
    spotmngr->setPlayDelay(ms);
}

void MainWindow::on_pushButtonremovewatermark_clicked()
{
    // Unterordner im Temp-Verzeichnis anlegen
    QDir subDir(QDir::tempPath() + "/SpotifyEnhancer");
    if (!subDir.exists() && !subDir.mkpath(".")) {
        QMessageBox::warning(this, "Fehler", "Unterordner im Temp-Verzeichnis konnte nicht erstellt werden.");
        return;
    }

    // Zielpfad im Unterordner
    QString targetPath = subDir.filePath("uwd.exe");

    // Datei immer neu kopieren (stellt sicher, dass sie aktuell ist)
    if (QFile::exists(targetPath))
        QFile::remove(targetPath);

    if (!QFile::copy(":/executables/uwd.exe", targetPath)) {
        QMessageBox::warning(this, "Fehler", "Kopieren der Datei fehlgeschlagen.");
        return;
    }

    // Explorer öffnen und Datei im Unterordner markieren
    QProcess::startDetached("explorer.exe",
                            {"/select," + QDir::toNativeSeparators(targetPath)});

    // Hinweis an den Benutzer
    QMessageBox::information(this,
                             "Manueller Start erforderlich",
                             "Die Datei wurde in den folgenden Ordner kopiert:\n" +
                             QDir::toNativeSeparators(subDir.absolutePath()) +
                             "\n\nBitte führen Sie 'uwd.exe' manuell aus.");
}


