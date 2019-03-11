#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>

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


public slots:
    void showProInfo();

private:
    Ui::MainWindow *ui;

    QStandardItemModel *proInfoItemModel;
};

#endif // MAINWINDOW_H
