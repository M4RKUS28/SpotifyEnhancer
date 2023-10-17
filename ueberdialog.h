#ifdef UEBERDIALOG_H
#define UEBERDIALOG_H

#include "updater.h"

#include <QDialog>

namespace Ui {
class UeberDialog;
}

class UeberDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UeberDialog(Updater * updater, QString version, QPixmap ico, QWidget *parent = nullptr);
    ~UeberDialog();

public slots:
    void updaterStatusChanged();

private slots:
    void on_pushButtonUpdaterButton_clicked();

    void on_pushButtonClose_clicked();

private:
    Ui::UeberDialog *ui;
    Updater * updater;
    void delay(int sec = 1);
};

#endif // UEBERDIALOG_H
