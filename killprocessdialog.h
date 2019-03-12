#ifndef KILLPROCESSDIALOG_H
#define KILLPROCESSDIALOG_H

#include <QDialog>

namespace Ui {
class killProcessDialog;
}

class killProcessDialog : public QDialog
{
    Q_OBJECT

public:
    explicit killProcessDialog(QWidget *parent = 0);
    ~killProcessDialog();

    void setBrowserText(QStringList list);

private slots:
    void on_buttonBox_accepted();

private:
    Ui::killProcessDialog *ui;
};

#endif // KILLPROCESSDIALOG_H
