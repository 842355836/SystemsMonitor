#include "helper.h"
#include <iostream>
#include <fstream>
#include <string>
#include <QDebug>
#include <QApplication>
#include <QMap>

#include <QDirIterator>
#include <QFile>

#include <stdlib.h>
#include "time.h"

Helper::Helper()
{
    return;
}

bool Helper::getInfoByPid(QString pid,QStringList &pidInfo)
{
    QString fileName="/proc/"+pid+"/stat";
    QFile file(fileName);

    if(!file.open(QIODevice::ReadOnly)){
        qDebug()<<"Helper::getInfoByPid: Failed to open file\n";
        return false;
    }

    auto content=file.readAll();
    file.close();

    QTextStream stream(content);
    QStringList list=stream.readAll().simplified().split(' ');
    //get process information
    pidInfo.clear();
    pidInfo.push_back(list[0]);  //pid
    pidInfo.push_back(list[1]);  //comm
    pidInfo.push_back(list[2]);  //state
    pidInfo.push_back(list[3]);  //ppid
    pidInfo.push_back(list[6]);  //tty
    pidInfo.push_back(list[18]); //nice
    pidInfo.push_back(list[13]); //utime
    pidInfo.push_back(list[14]); //stime

    return true;
}

bool Helper::getMemUsage(long int &memTotal,long int &memFree,long int &swapTotal, long int &swapFree)
{
    QString fileName="/proc/meminfo";

    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly)){
        qDebug()<<"Helper::getMemUsage: Failed to open file \"/proc/meminfo\"\n";
        return false;
    }

    auto content=file.readAll();
    file.close();

    QTextStream stream(content);
    memTotal=memFree=swapTotal=swapFree=0;
    while(!stream.atEnd()){
        QString line(stream.readLine());
        if(line.contains("MemTotal"))
            memTotal=line.simplified().split(' ').value(1).toInt();
        if(line.contains("MemFree"))
            memFree=line.simplified().split(' ').value(1).toInt();
        if(line.contains("SwapTotal"))
            swapTotal=line.simplified().split(' ').value(1).toInt();
        if(line.contains("SwapFree")){
            swapFree=line.simplified().split(' ').value(1).toInt();
            break;
        }
    }
    return true;
}

bool Helper::getCpuTime(long int &total_time, long int &idleTime)
{
    total_time=0;
    idleTime=0;
    QFile qfile("/proc/stat");
    if(!qfile.open(QIODevice::ReadOnly)){
        qDebug()<<"Helper::getCpuIime: Failed to open file\n";
        return false;
    }
    QString content(qfile.readLine());
    QStringList time(content.simplified().split(' '));
    qfile.close();

    //get total cpu time and idle cpu time
    for(int i=0;i<6;i++)
        total_time+=time[i].toInt();
    idleTime=time[3].toInt()+time[4].toInt();

    return true;
}

bool Helper::getCpuInfo(QVector<QString> &cpuInfo)
{
    QFile qfile("/proc/cpuinfo");
    if(qfile.open(QIODevice::ReadOnly)){
        auto content=qfile.readAll();
        QTextStream stream(&content);
        while(!stream.atEnd())
            cpuInfo.push_back(stream.readLine().simplified());
        qfile.close();
    }
    return true;
}

bool Helper::getMemInfo(QVector<QString> &memInfo)
{
    QFile qfile("/proc/meminfo");
    if(qfile.open(QIODevice::ReadOnly)){
        auto content=qfile.readAll();
        QTextStream stream(&content);
        while(!stream.atEnd())
            memInfo.push_back(stream.readLine().simplified());
        qfile.close();
    }
    return true;
}


bool Helper::getHostInfo(QVector<QString> &infoVec)
{
    std::string path1="/proc/sys/kernel/hostname";
    std::string path2="/proc/uptime";
    std::string path3="/proc/sys/kernel/ostype";
    std::string path4="/proc/sys/kernel/osrelease";
    std::ifstream fs;
    std::string str,str2;
    time_t runTime,nowTime,startTime;
    struct tm *tmRunTime;

    fs.open(path1);
    fs>>str;
    fs.close();
    qDebug()<<"Helper::geHostInfo: "<<"The hostname is "<<QString::fromStdString(str)<<'\n';
    infoVec.push_back(QString::fromStdString(str));

    fs.open(path2);
    fs>>runTime;
    fs.close();

    nowTime=time(NULL);
    startTime=nowTime-runTime;
    str=ctime(&startTime);
    infoVec.push_back(QString::fromStdString(str));
    qDebug()<<"Helper::geHostInfo: "<<"System startup time is"<<QString::fromStdString(str)<<'\n';

    tmRunTime=localtime(&runTime);
    infoVec.push_back(QString("%1天%2时%3分%4秒").arg(tmRunTime->tm_mday).arg(tmRunTime->tm_hour).arg(tmRunTime->tm_min).arg(tmRunTime->tm_sec));
    qDebug()<<"Helper::geHostInfo: "<<"System running time is"<<tmRunTime->tm_mday<<"天"<<tmRunTime->tm_hour<<"时"
           <<tmRunTime->tm_min<<"分"<<tmRunTime->tm_sec<<"秒\n";

    fs.open(path3);
    fs>>str;
    fs.close();
    fs.open(path4);
    fs>>str2;
    fs.close();
    str=str+' '+str2;
    infoVec.push_back(QString::fromStdString(str));
    qDebug()<<"Helper::geHostInfo: "<<"The system version is "<<QString::fromStdString(str)<<'\n';
    return true;
}

bool Helper::getProcessInfo(QVector<QStringList> &infoVec)
{
    QDirIterator iter("/proc");

    while(iter.hasNext()){
        QString dirPath=iter.next();
        QString dirName=(QString)dirPath.simplified().split("/").value(2);

        int j=0;
        for(;j<dirName.size();++j){
            if(!dirName[j].isDigit())
                break;
        }
        if(j!=dirName.size())
            continue;

        QStringList proInfo;
        QString filePath=dirPath+"/stat";
        QFile fs(filePath);
        if(fs.open(QIODevice::ReadOnly))
        {
            auto allcontents=fs.readAll();
            QTextStream streamall(&allcontents);
            QString line=streamall.readAll();
            QStringList splited=line.simplified().split(" ");
            proInfo.push_back(splited[0]);  //pid
            proInfo.push_back(splited[1]);  //comm
            proInfo.push_back(splited[2]);  //state
            proInfo.push_back(splited[18]); //nice
            proInfo.push_back(splited[13]); //utime
            proInfo.push_back(splited[14]); //stime
        }
        fs.close();

        filePath=dirPath+"/statm";
        fs.setFileName(filePath);
        if(fs.open(QIODevice::ReadOnly))
        {
            auto allcontents=fs.readAll();
            QTextStream streamall(&allcontents);
            QString line=streamall.readAll();
            QStringList splited=line.simplified().split(" ");
            proInfo.push_back(splited[1]);  //resident set size
        }
        fs.close();

        infoVec.push_back(proInfo);
    }

    return true;
}
