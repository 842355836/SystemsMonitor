#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "helper.h"
#include <QStandardItem>
#include <QList>
#include <QDebug>
#include <unordered_map>
#include <unistd.h>
#include <QTimer>
#include <killprocessdialog.h>
#include <QChart>
#include <QLineSeries>
#include <QValueAxis>
#include <QAction>
#include <QPointF>
#include <QDateTime>
#include <QInputDialog>
#include <QProcess>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle("System Monitor");

    this->proInfoItemModel=new QStandardItemModel(0,6);
    ui->tableView->setModel(this->proInfoItemModel);
    ui->tableView->setAlternatingRowColors(true);
    this->proInfoItemModel->setHorizontalHeaderLabels(QStringList()<<"PID"<<"comm"<<"state"<<"nice"<<"cpu"<<"mem");

    QTimer *timer=new QTimer();
    connect(timer,&QTimer::timeout,this,&MainWindow::showProInfo);
    connect(timer,&QTimer::timeout,this,&MainWindow::showHostInfo);
    connect(timer,&QTimer::timeout,this,&MainWindow::showCpuUsagePercentage);
    connect(timer,&QTimer::timeout,this,&MainWindow::showMemUsagePercentage);
    connect(timer,&QTimer::timeout,this,&MainWindow::updateStatusBar);

    timer->start(1000);
    this->showProInfo();
    this->showHostInfo();
    this->initMySeries();

    QAction *act=new QAction();
    act->setText("run a new process");
    this->menuBar()->addAction(act);
    connect(act,&QAction::triggered,this,&MainWindow::runNewProcess);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::runNewProcess()
{
    QString title="Input dialog";
    QString label="Please enter a command";
    QString comm=QInputDialog::getText(NULL,title,label);
    qDebug()<<comm<<"\n";
//    system(comm.toStdString().c_str());

    QProcess process;
    process.start(comm);
    process.waitForFinished();
    QString output=process.readAllStandardOutput();
    qDebug()<<output<<'\n';

    title="Output dialog";
    label="Program running result\n";
    QMessageBox::information(NULL,title,label+output,QMessageBox::Ok);
}

void MainWindow::updateStatusBar()
{
    static long int cpuTotalTime=0;
    static long int cpuFreeTime=0;
    static float cpuUsagePer=0;
    static float memUsagePer=0;

    long int curCpuTotalTime,curCpuFreeTime;
    long int curMemTotal,curMemFree;
    long int swapTotal,swapFree;

    //get current time
    QString timeStamper=QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss.zzz ddd");;

    //update cpu and memory usage percentage
    if(Helper::getCpuTime(curCpuTotalTime,curCpuFreeTime)){
        cpuUsagePer=100.0*(1-((float)(curCpuFreeTime-cpuFreeTime)/(curCpuTotalTime-cpuTotalTime)));
        cpuTotalTime=curCpuTotalTime;
        cpuFreeTime=curCpuFreeTime;
    }
    qDebug()<<cpuUsagePer<<' '<<QString::number(cpuUsagePer,'f',1)<<'\n';

    if(Helper::getMemUsage(curMemTotal,curMemFree,swapTotal,swapFree))
        memUsagePer=100.0*(float)(curMemTotal-curMemFree)/curMemTotal;

    this->statusBar()->showMessage("系统时间: "+timeStamper+"\tCPU使用率: "+QString::number(cpuUsagePer,'f',1)+"\t内存使用率: "+QString::number(memUsagePer,'f',1));
}

void MainWindow::initMySeries()
{
    QtCharts::QValueAxis *axisX=new QtCharts::QValueAxis();
    QtCharts::QValueAxis *axisY=new QtCharts::QValueAxis();
    ui->cpuChartView->chart()->addAxis(axisX,Qt::AlignBottom);
    ui->cpuChartView->chart()->addAxis(axisY,Qt::AlignLeft);
    ui->cpuChartView->chart()->axisX()->setRange(0,120);
    ui->cpuChartView->chart()->axisY()->setRange(0,100);

    axisX->setTitleText("time(Second)");
    axisY->setTitleText("percentage(%)");

    this->cpuSeries= new QtCharts::QLineSeries();
    this->cpuSeries->setName("CPU usage percentage");
    ui->cpuChartView->chart()->addSeries(this->cpuSeries);
    this->cpuSeries->attachAxis(ui->cpuChartView->chart()->axisX());
    this->cpuSeries->attachAxis(ui->cpuChartView->chart()->axisY());

    ui->cpuChartView->setRenderHint(QPainter::Antialiasing);
    ui->cpuChartView->show();

    this->showCpuUsagePercentage();

    QtCharts::QValueAxis *axisX2=new QtCharts::QValueAxis();
    QtCharts::QValueAxis *axisY2=new QtCharts::QValueAxis();
    ui->memChartView->chart()->addAxis(axisX2,Qt::AlignBottom);
    ui->memChartView->chart()->addAxis(axisY2,Qt::AlignLeft);
    ui->memChartView->chart()->axisX()->setRange(0,120);
    ui->memChartView->chart()->axisY()->setRange(0,100);

    this->memSeries=new QtCharts::QLineSeries();
    this->memSeries->setName("Memory usage percentage");
    ui->memChartView->chart()->addSeries(this->memSeries);
    this->memSeries->attachAxis(ui->memChartView->chart()->axisX());
    this->memSeries->attachAxis(ui->memChartView->chart()->axisY());

    this->swapSeries=new QtCharts::QLineSeries();
    this->swapSeries->setName("Swap space usage percentage");
    ui->memChartView->chart()->addSeries(this->swapSeries);
    this->swapSeries->attachAxis(ui->memChartView->chart()->axisX());
    this->swapSeries->attachAxis(ui->memChartView->chart()->axisY());

    ui->memChartView->setRenderHint(QPainter::Antialiasing);
    ui->memChartView->show();

    this->showCpuUsagePercentage();
}

void MainWindow::showMemUsagePercentage()
{
    static QVector<float> memPercentage(120,0),swapPercentage(120,0);
    QVector<QPointF> points(120);

    long int curMemTotal,curMemFree,curSwapTotal,curSwapFree;
    if(Helper::getMemUsage(curMemTotal,curMemFree,curSwapTotal,curSwapFree)){
        memPercentage.pop_front();
        memPercentage.push_back(100.0*(float)(curMemTotal-curMemFree)/curMemTotal);
        swapPercentage.pop_front();
        swapPercentage.push_back(100.0*(float)(curSwapTotal-curSwapFree)/curSwapTotal);;
    }

    for(int i=0;i<memPercentage.size();i++){
        points[i].setX(i);
        points[i].setY(memPercentage[i]);
    }
    this->memSeries->replace(points);

    for(int i=0;i<swapPercentage.size();i++){
        points[i].setX(i);
        points[i].setY(swapPercentage[i]);
    }
    this->swapSeries->replace(points);
}

void MainWindow::showCpuUsagePercentage()
{
    static long int totalTime=0;
    static long int freeTime=0;
    static QVector<float> percentage(120,0);
    QVector<QPointF> points(120);

    long int curTotalTime,curFreeTime;
    if(Helper::getCpuTime(curTotalTime,curFreeTime)){
        percentage.pop_front();
        percentage.push_back(100.0*(1-((float)(curFreeTime-freeTime)/(curTotalTime-totalTime))));
    }
    totalTime=curTotalTime;
    freeTime=curFreeTime;

    //update points
    for(int i=0;i<percentage.size();i++){
        points[i].setX(i);
        points[i].setY(percentage[i]);
    }
    this->cpuSeries->replace(points);
}

void MainWindow::on_tableView_doubleClicked(const QModelIndex &index)
{
    QStringList list;
    killProcessDialog *killPro=new killProcessDialog();

    //get pid
    QString pid(this->proInfoItemModel->itemData(this->proInfoItemModel->index(index.row(),0)).value(0).toByteArray());

    //get process informaton by pid
    Helper::getInfoByPid(pid,list);

    //show dialog
    killPro->setBrowserText(list);
    killPro->show();
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
    static long int total_time=0;
    static long int idle_time=0;
    static std::unordered_map<int,long int> mapCpuTime;

    long int curTotal,curIdle;
    QVector<QStringList> infoPro;

    Helper::getProcessInfo(infoPro);
    Helper::getCpuTime(curTotal,curIdle);

    if(infoPro.size()!=this->proInfoItemModel->rowCount())
        this->proInfoItemModel->setRowCount(infoPro.size());

    //update data
    for(int i=0;i<infoPro.size();i++){
        if(infoPro[i].size()!=7)
            continue;
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

        this->proInfoItemModel->setData(this->proInfoItemModel->index(i,4),QString::number(cpuUsage,'f',1)+'%');
        this->proInfoItemModel->item(i,4)->setEditable(false);

        //Resident memory size
        float memSize=infoPro[i][6].toInt()*4.0/1024;
        this->proInfoItemModel->setData(this->proInfoItemModel->index(i,5),QString::number(memSize,'f',1)+'M');
        this->proInfoItemModel->item(i,5)->setEditable(false);
    }

    total_time=curTotal;
    idle_time=curIdle;
}
