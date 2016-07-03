#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QDateTime>
#include <QDesktopWidget>
#include <time.h>
#include <QProcess>
#include <QMenu>
#include <QCloseEvent>
#include "qsysteminfopage.h"
#include "qsvcstatepage.h"
#include "qdevstatepage.h"
#include "qclientstatepage.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    d_Notify(parent)
{
#ifdef Q_OS_WIN
#else
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

    QTimer *pTime = new QTimer(this);
    connect(pTime,SIGNAL(timeout()),this,SLOT(timeUpdate()));
    pTime->start(1000);


    createActions();
    createTrayIcon();

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
        this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

    trayIcon->setIcon(QIcon(tr(":/new/images/server.png")));
    trayIcon->show();

    setWindowIcon(QIcon(tr(":/new/images/server.png")));

    setWindowTitle(tr("Device monitor server(Ver:2.0.0.1)"));//设备监测服务器(版本:2.0.0.1)


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

void MainWindow::showMessage(QString &sHeader,QString &sContent)
{
    trayIcon->showMessage(sHeader,sContent, QSystemTrayIcon::Information,3000);

}

void MainWindow::closeEvent(QCloseEvent *event)
{

    if (trayIcon->isVisible()) //程序将在托盘区继续运行,想要退出程序请在快捷菜单中选择\"退出程序\"
    {
        showMessage(tr("remind"),tr("Program will continue to run in the area of the tray, you want to quit the program, please select the shortcut menu in the \"exit\""));
        hide();
        event->ignore();
    }
}

void MainWindow::OnUpdateDevList(bool bLoad)
{
    if(bLoad)
        d_pDevStatePage->LoadDevToList();
    else
        d_pDevStatePage->clearDevList();
}

void MainWindow::startSvc(){
    d_pSvcStatePage->StartSvc();
    OnUpdateMenu();
}

void MainWindow::OnUpdateMenu()
{
    if(d_pSvcStatePage->IsStart())
    {
        startupAction->setText(tr("StopSvc"));
        startupAction->setIcon(QIcon(tr(":/new/images/player_stop.png")));

        tSvcStartTime = QDateTime::currentDateTime();
        trayIcon->setToolTip(QString(tr("Server is running...\n%1")).arg(tSvcStartTime.toString(tr("yyyy-MM-dd hh:mm:ss dddd"))));

    }
    else
    {
        startupAction->setText(tr("StartSvc"));
        startupAction->setIcon(QIcon(tr(":/new/images/player_play.png")));
        trayIcon->setToolTip(QString(tr("Server is stopped!")));
    }
}


void MainWindow::timeUpdate()
{
    if(!d_pSvcStatePage->IsStart())
        return ;

    double ninterval = difftime(QDateTime::currentDateTime().toTime_t(),tSvcStartTime.toTime_t());
    QString printStr = QDateTime::fromTime_t(ninterval).toString("yyyy-MM-dd hh:mm:ss");
    int secondsPerDay = 3600*24;
    int ndays = (int)ninterval/secondsPerDay;//dataCur.day()-tSvcStartTime.date().day();
    int hourSpan = (int)ninterval%secondsPerDay;
    int nHours = hourSpan/3600;
    int minuteSpan = hourSpan%3600;
    int nMinutes = minuteSpan/60;
    int nSeconds = minuteSpan%60;

    trayIcon->setToolTip(QString(tr("Server is running...\n%1\n Run length：%2day%3hour%4minute%5second")).arg(tSvcStartTime.toString(tr("yyyy-MM-dd hh:mm:ss dddd")))\
                         .arg(ndays).arg(nHours).arg(nMinutes).arg(nSeconds));

}

void MainWindow::createTrayIcon()
{
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(startupAction);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
}

void MainWindow::createActions()
{
    startupAction = new QAction(QIcon(tr(":/new/images/player_play.png")),tr("StartSvc(&S)"), this);
    connect(startupAction, SIGNAL(triggered()), this, SLOT(startSvc()));//
    restoreAction = new QAction(QIcon(tr(":/new/images/video_display.png")),tr("StopSvc(&R)"), this);
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));
    quitAction = new QAction(QIcon(tr(":/new/images/stock_exit.png")),tr("Exit(&E)"), this);
    connect(quitAction, SIGNAL(triggered()), this, SLOT(quitApp()));
}

void MainWindow::quitApp()
{
    d_pSvcStatePage->disconnect();
    d_pDevStatePage->disconnect();
    if(d_pSvcStatePage->IsStart())
        d_pSvcStatePage->StartSvc();

    qApp->quit();
}


void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        showNormal();
        break;
    default:
        ;
    }
}

void MainWindow::signalAppSlot(const QString& activeMsg )
{
    showNormal();
}
