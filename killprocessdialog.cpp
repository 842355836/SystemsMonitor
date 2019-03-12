#include "killprocessdialog.h"
#include "ui_killprocessdialog.h"
#include "unistd.h"

killProcessDialog::killProcessDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::killProcessDialog)
{
    ui->setupUi(this);
}

killProcessDialog::~killProcessDialog()
{
    delete ui;
}

void killProcessDialog::setBrowserText(QStringList list)
{
    ui->textBrowser->setFontPointSize(12);
    ui->textBrowser->append("pid: "+list[0]);
    ui->textBrowser->append("comm: "+list[1].remove('(').remove(')'));
    ui->textBrowser->append("stat: "+list[2]);
    ui->textBrowser->append("ppid: "+list[3]);
    ui->textBrowser->append("tty: "+list[4]);
    ui->textBrowser->append("nice: "+list[5]);
    ui->textBrowser->append("utime: "+list[6]);
    ui->textBrowser->append("stime: "+list[7]);
    this->setWindowTitle("kill process: "+list[0]);
}

void killProcessDialog::on_buttonBox_accepted()
{
    //get pid form the window title
    QString pid=this->windowTitle().split(':').value(1);
    pid=pid.simplified();

    //kill process
    QString command="kill 9 "+pid;
    system(command.toStdString().c_str());
}
