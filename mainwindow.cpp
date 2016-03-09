#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QDateTime>
#include <time.h>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
     setWindowFlags(Qt::FramelessWindowHint);
    ui->setupUi(this);
    QTimer *pTime = new QTimer(this);
    connect(pTime,SIGNAL(timeout()),this,SLOT(timeUpdate()));
    pTime->start(1000);
    connect(ui->tbServer,SIGNAL(clicked()),this,SLOT(testSetTime()));

    ui->lbDatetime->setStyleSheet("color:#7dc5ec;");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::timeUpdate()
{
    QDateTime current_date_time = QDateTime::currentDateTime();
    QString current_date = current_date_time.toString("yyyy-MM-dd hh:mm:ss");

    ui->lbDatetime->setText(current_date);
}

void MainWindow::testSetTime()
{
    struct tm nowtime;
    time_t t;
    nowtime.tm_sec=56;/* Seconds.[0-60](1 leap second)*/
    nowtime.tm_min=34;/* Minutes.[0-59] */
    nowtime.tm_hour=12;/* Hours. [0-23] */
    nowtime.tm_mday=23;/*  Day.[1-31]  */
    nowtime.tm_mon=8-1;/* Month.[0-11]*/
    nowtime.tm_year=2013-1900;/* Year- 1900.*/
    nowtime.tm_isdst=-1;/*DST.[-1/0/1]*/
    t=mktime(&nowtime);
    int nRt = stime(&t);
    if(nRt>0)
        return;

}
