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

bool Helper::getCpuTime(long long int &total_time)
{
    total_time=0;

    QFile qfile("/proc/stat");
    if(qfile.open(QIODevice::ReadOnly)){
        QString content(qfile.readLine());
        QStringList time(content.simplified().split(' '));
        for(auto &tmp: time)
            total_time+=tmp.toInt();
        qfile.close();
    }
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
            proInfo.push_back(splited[2]);  //task_state
            proInfo.push_back(splited[18]); //nice
            proInfo.push_back(splited[13]); //utime
            proInfo.push_back(splited[14]); //stime
        }
        filePath=dirPath+"/statm";
        fs.setFileName(filePath);
        if(fs.open(QIODevice::ReadOnly))
        {
            auto allcontents=fs.readAll();
            QTextStream streamall(&allcontents);
            QString line=streamall.readAll();
            QStringList splited=line.simplified().split(" ");
            proInfo.push_back(splited[1]);  //Resident(pages)
        }
        infoVec.push_back(proInfo);
    }

    return true;
}
