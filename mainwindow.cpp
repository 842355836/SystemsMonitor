#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "helper.h"
#include <QStandardItem>
#include <QList>
#include <QDebug>
#include <unordered_map>
#include <unistd.h>
#include <QThread>
#include <QEventLoop>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->showHostInfo();

    this->proInfoItemModel=new QStandardItemModel(0,6);
    ui->tableView->setModel(this->proInfoItemModel);
    ui->tableView->setAlternatingRowColors(true);
    this->proInfoItemModel->setHorizontalHeaderLabels(QStringList()<<"PID"<<"comm"<<"state"<<"nice"<<"cpu"<<"mem");

    QTimer *timer=new QTimer();
    connect(timer,&QTimer::timeout,this,&MainWindow::showProInfo);

    timer->start(1000);
    this->showProInfo();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showHostInfo()
{
    QVector<QString> infoHost;
    QVector<QString> infoCPU;
    QVector<QString> infoMem;
    Helper::getHostInfo(infoHost);
    Helper::getCpuInfo(infoCPU);
    Helper::getMemInfo(infoMem);

    ui->tab1_label_hostname->setText("主机名: "+infoHost[0]);
    ui->tab1_label_startTime->setText("启动时间: "+infoHost[1]);
    ui->tab1_label_runningTime->setText("运行时间: "+infoHost[2]);
    ui->tab1_label_osVersion->setText("系统版本: "+infoHost[3]);
    for(auto &it: infoCPU){
        if(it.contains("model name"))
            ui->tab1_label_CPU->setText("CPU: "+it.split(':').value(1).simplified());
        if(it.contains("cpu MHz")){
            ui->tab1_label_mainFreq->setText("主频: "+it.split(':').value(1).simplified()+" MHz");
            break;
        }
    }

    ui->tab1_textBrowser_cpuInfo->clear();
    ui->tab1_textBrowser_memInfo->clear();
    for(auto &it: infoCPU)
        ui->tab1_textBrowser_cpuInfo->append(it);
    for(auto &it: infoMem)
        ui->tab1_textBrowser_memInfo->append(it);
}

void MainWindow::showProInfo()
{
    static long long int total_time=0;
    static std::unordered_map<int,long long int> mapCpuTime;


    long long int curTotal;
    QVector<QStringList> infoPro;

    Helper::getProcessInfo(infoPro);
    Helper::getCpuTime(curTotal);


    if(infoPro.size()!=this->proInfoItemModel->rowCount())
        this->proInfoItemModel->setRowCount(infoPro.size());

    //update data
    for(int i=0;i<infoPro.size();i++){
        for(int j=0;j<4;j++){
            this->proInfoItemModel->setData(this->proInfoItemModel->index(i,j),infoPro[i][j].remove('(').remove(')'));
            this->proInfoItemModel->item(i,j)->setEditable(false);
        }
        //cpu usage
        long long int preCpuTime=0,curCpuTime;
        curCpuTime=infoPro[i][4].toInt()+infoPro[i][5].toInt();
        if(mapCpuTime.find(infoPro[i][0].toInt())!=mapCpuTime.end())
            preCpuTime=mapCpuTime[infoPro[i][0].toInt()];
        float cpuUsage=(curCpuTime-preCpuTime)*100.0/(curTotal-total_time);
        mapCpuTime[infoPro[i][0].toInt()]=curCpuTime;

        this->proInfoItemModel->setData(this->proInfoItemModel->index(i,4),QString::number(cpuUsage)+'%');
        this->proInfoItemModel->item(i,4)->setEditable(false);

        //Resident memory size
        float memSize=infoPro[i][6].toInt()*4.0/1024;
        this->proInfoItemModel->setData(this->proInfoItemModel->index(i,5),QString::number(memSize)+'M');
        this->proInfoItemModel->item(i,5)->setEditable(false);
    }
    total_time=curTotal;
}


