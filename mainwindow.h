#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QLineSeries>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void showHostInfo();

    void showCpuUsagePercentage();

    void showMemUsagePercentage();

    void initMySeries();

    void updateStatusBar();

    void runNewProcess();

public slots:
    void showProInfo();

private slots:
    void on_tableView_doubleClicked(const QModelIndex &index);

private:
    Ui::MainWindow *ui;

    QtCharts::QLineSeries *cpuSeries;
    QtCharts::QLineSeries *memSeries;
    QtCharts::QLineSeries *swapSeries;

    QStandardItemModel *proInfoItemModel;
};

#endif // MAINWINDOW_H
