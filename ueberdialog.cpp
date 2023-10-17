#ifdef a

#include "ueberdialog.h"
#include "ui_ueberdialog.h"

#include <QTime>

UeberDialog::UeberDialog(Updater *updater, QString version, QPixmap ico, QWidget *parent) :
    QDialog(parent), ui(new Ui::UeberDialog),
    updater(updater)
{
    ui->setupUi(this);
    if(!updater) return;
    this->ui->label_icon->setPixmap(ico);

    this->ui->labelUpdateStatus->setText(updater->getStatusStr());

    ui->label_version->setText("SpotifyEnhancer " + version);
    ui->label_qt_version->setText("Qt " + QString(QT_VERSION_STR));
    ui->label_compile_date->setText(QString(__DATE__) + QString(" at ") + QString(__TIME__));
    ui->label_arch->setText(QSysInfo::currentCpuArchitecture());

    updater->setAutoShowMessageBox(false);
    updaterStatusChanged();
    connect(updater, SIGNAL(statusChanged()), this, SLOT(updaterStatusChanged()));

}

UeberDialog::~UeberDialog()
{
    delete ui;
}

void UeberDialog::updaterStatusChanged()
{
    ui->pushButtonUpdaterButton->hide();
    ui->labelUpdateStatus->setText(updater->getStatusStr());
    this->ui->pushButtonUpdaterButton->setText(updater->getStatus() == Updater::UPDATE_STATUS::NOT_CHECKED || updater->getStatus() == Updater::UPDATE_STATUS::UP_TO_DATE ? "Nach Updates suchen" : "Aktualisieren");

    switch (updater->getStatus()) {
    case Updater::NOT_CHECKED:
        ui->pushButtonUpdaterButton->show();
        break;
    case Updater::UPDTAE_NEEDED:
        ui->pushButtonUpdaterButton->show();
        break;
    case Updater::UP_TO_DATE:
        ui->labelUpdateStatus->setText("Sie verwenden die neuste Version!");
        ui->pushButtonUpdaterButton->show();
        break;
    case Updater::UPDATING: {
        ui->labelUpdateStatus->setText("Updater ausgeführt!");/* Programm schließt sich in 3 Sekunden!");
        this->delay();
        ui->labelUpdateStatus->setText("Updater ausgeführt! Programm schließt sich in 2 Sekunden!");
        this->delay();
        ui->labelUpdateStatus->setText("Updater ausgeführt! Programm schließt sich in 1 Sekunden!");
        this->delay(2);
        QApplication::exit();*/
        break;
        }
    default:
        break;
    }
}

void UeberDialog::delay(int sec)
{
    QTime dieTime= QTime::currentTime().addSecs(sec);
    while (QTime::currentTime() < dieTime)
        QApplication::processEvents(QEventLoop::AllEvents, 100);
}


void UeberDialog::on_pushButtonUpdaterButton_clicked()
{
    if(updater->getStatus() == Updater::UPDATE_STATUS::NOT_CHECKED || updater->getStatus() == Updater::UPDATE_STATUS::UP_TO_DATE) {
        updater->checkForUpdates();
    } else if(updater->getStatus() == Updater::UPDATE_STATUS::UPDTAE_NEEDED) {
        updater->startUpdate();
    }
}


void UeberDialog::on_pushButtonClose_clicked()
{
    this->done(0);
}

#endif
