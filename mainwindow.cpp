#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QDateTime>
#include <QDesktopWidget>
#include <time.h>
#include "qsysteminfopage.h"
#include "qsvcstatepage.h"
#include "qdevstatepage.h"
#include "qclientstatepage.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    d_Notify(parent)
{
    setWindowFlags(Qt::FramelessWindowHint);
    ui->setupUi(this);

    d_pSvcStatePage = new QSvcStatePage(d_Notify,this);
    ui->swContainter->addWidget(d_pSvcStatePage);

    d_pDevStatePage = new QDevStatePage(d_Notify,this);
    ui->swContainter->addWidget(d_pDevStatePage);

    d_pClientStatePage = new QClientStatePage(d_Notify,this);
    ui->swContainter->addWidget(d_pClientStatePage);

    d_pSystemInfoPage = new QSystemInfoPage(this);
    ui->swContainter->addWidget(d_pSystemInfoPage);

    connect(ui->tbServer,SIGNAL(clicked()),this,SLOT(ShowSvcStatePage()));
    connect(ui->tbDeviceState,SIGNAL(clicked()),this,SLOT(ShowDevStatePage()));
    connect(ui->tbOnlineUser,SIGNAL(clicked()),this,SLOT(ShowClientStatePage()));
    connect(ui->tbSystem,SIGNAL(clicked()),this,SLOT(ShowSystemInfoPage()));
    connect(d_pSvcStatePage,SIGNAL(updateDevList(bool)),this,SLOT(OnUpdateDevList(bool)));

    ui->tbServer->setChecked(true);
    ShowSvcStatePage();

    d_pSvcStatePage->StartSvc();


}

MainWindow::~MainWindow()
{
    delete ui;
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

void MainWindow::ShowSvcStatePage(){
    ui->swContainter->setCurrentWidget(d_pSvcStatePage);
}

void MainWindow::ShowDevStatePage(){
    ui->swContainter->setCurrentWidget(d_pDevStatePage);
}

void MainWindow::ShowClientStatePage(){
    ui->swContainter->setCurrentWidget(d_pClientStatePage);
}

void MainWindow::ShowSystemInfoPage(){
    ui->swContainter->setCurrentWidget(d_pSystemInfoPage);
}

void MainWindow::OnUpdateDevList(bool bLoad)
{
    if(bLoad)
        d_pDevStatePage->LoadDevToList();
    else
        d_pDevStatePage->clearDevList();
}
