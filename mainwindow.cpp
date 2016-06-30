#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QDateTime>
#include <QDesktopWidget>
#include <time.h>
#include <QProcess>
#include "qsysteminfopage.h"
#include "qsvcstatepage.h"
#include "qdevstatepage.h"
#include "qclientstatepage.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    d_Notify(parent)
{
#ifdef ARM_LINUX_DEF
    setWindowFlags(Qt::FramelessWindowHint);
#endif
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

   // QTimer *pTime = new QTimer(this);
    //connect(pTime,SIGNAL(timeout()),this,SLOT(timeUpdate()));
    //pTime->start(1000);
    //d_process_ptr = new QProcess(this);
    //connect(d_process_ptr,SIGNAL(readyReadStandardOutput()),this,SLOT(readStandardOutput()));
    //QString sexecute = QString("cat /sys/class/backlight/pwm-backlight/brightness");

    //d_process_ptr->execute(sexecute);


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
    //t=mktime(&nowtime);
    int nRt;// = stime(&t);
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

/*bool MainWindow::event(QEvent *event)
{
    switch(event->type()) {
    case QEvent::MouseButtonPress:
        OpenBacklight();
        break;
    }
    return QMainWindow::event(event);
}

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
             OpenBacklight();
    }
    return QMainWindow::eventFilter(object,event);
}

void MainWindow::timeUpdate()
{
    if(d_close_backlight_count>60){
        if(d_cur_backlight<20)
            return;
       // QString sexecute = QString("cat /sys/class/backlight/pwm-backlight/brightness");
       //  d_process_ptr->execute(sexecute);
        if(d_cur_backlight>20){
            QString  sexecute = QString("echo echo 10 >/sys/class/backlight/pwm-backlight/brightness");
            d_process_ptr->execute(sexecute);
            d_cur_backlight = 10;
        }
    }else
        d_close_backlight_count++;
}

void MainWindow::readStandardOutput()
{
    QString  sResult = d_process_ptr->readAllStandardOutput();
    d_cur_backlight = sResult.toInt();
    cout<<"d_cur_backlight="<<d_cur_backlight<<"----readall = "<<sResult.toStdString().c_str()<<endl;
}

void MainWindow::OpenBacklight()
{
    d_close_backlight_count = 0;
    //QString sexecute = QString("cat /sys/class/backlight/pwm-backlight/brightness");
    //d_process_ptr->execute(sexecute);
  //  if(d_cur_backlight>20){
       QString  sexecute = QString("echo echo 80 >/sys/class/backlight/pwm-backlight/brightness");
        d_process_ptr->execute(sexecute);
        d_cur_backlight = 80;
   // }
}*/
