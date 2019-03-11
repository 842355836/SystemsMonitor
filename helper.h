#ifndef HELPER_H
#define HELPER_H

#include <QVector>
#include <QString>


class Helper
{
public:
    Helper();

    /*函数功能：获取主机信息
    * 入口参数：无
    * 出口参数：infoVec	返回主机信息
    * 返回值：成功返回true，失败返回false
    */
    static bool getHostInfo(QVector<QString> &infoVec);

    /*函数功能：获取进程信息
    * 入口参数：无
    * 出口参数：infoVec	返回进程信息
    * 返回值：成功返回true，失败返回false
    */
    static bool getProcessInfo(QVector<QStringList> &infoVec);

    static bool getCpuInfo(QVector<QString> &cpuInfo);
    static bool getMemInfo(QVector<QString> &memInfo);
    static bool getCpuTime(long long int &total_time);
};

#endif // HELPER_H
