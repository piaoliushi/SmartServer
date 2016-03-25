#include "qsvcstatepage.h"
#include <QGridLayout>
#include <QDebug>
#include<QMessageBox>
#include <QDateTime>
#include <QTimer>
#include <QCoreApplication>
#include <QLocale>
#include <QSqlDatabase>
#include "./net/SvcMgr.h"
#include "./net/config.h"
#include "StationConfig.h"
#include "LocalConfig.h"

using namespace db;

QSvcStatePage::QSvcStatePage(QNotifyHandler &Notify,QWidget *parent)
    : QWidget(parent)
    ,m_Notify(Notify)
    ,m_IsRunning(false)
    ,d_pOnlineDevValueLabel(NULL)
{
    QHBoxLayout *pTopLyt =new QHBoxLayout();
    QHBoxLayout *pHLyt = new QHBoxLayout();
    QVBoxLayout *pVMainLyt = new QVBoxLayout();

    QSpacerItem *pLeftTopSpace = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Maximum);
    d_plbDateTime = new QLabel(this);
    d_plbDateTime->setFixedSize(245,20);
    d_plbDateTime->setStyleSheet(tr("font: 18pt; color:#45c9d5;"));
    pTopLyt->addSpacerItem(pLeftTopSpace);
    pTopLyt->addWidget(d_plbDateTime);

    pVMainLyt->addLayout(pTopLyt);

    QSpacerItem *pSpaceTop = new QSpacerItem(40, 40, QSizePolicy::Fixed, QSizePolicy::Fixed);
    pVMainLyt->addSpacerItem(pSpaceTop);

    QSpacerItem *pSpaceLeft = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Maximum);
    pHLyt->addSpacerItem(pSpaceLeft);
    QGridLayout *pGridLayout = new QGridLayout();
    pGridLayout->setObjectName(QString::fromUtf8("SvcStateLayout"));
    QLabel *pSvcStateLabel = new QLabel(QObject::tr("服务状态:"));
    pSvcStateLabel->setFixedSize(80,30);
    pGridLayout->addWidget(pSvcStateLabel,0,0,1,1);

    d_pSvcStateValueLabel = new QLabel(QObject::tr("已停止"));
    d_pSvcStateValueLabel->setFixedHeight(30);
    d_pSvcStateValueLabel->setStyleSheet(tr("font: 18pt; color:rgb(117,250,0);"));
    pGridLayout->addWidget(d_pSvcStateValueLabel,0,1,1,1);

    QLabel *pOnlineDevLabel = new QLabel(QObject::tr("在线设备:"));
    pOnlineDevLabel->setFixedSize(80,30);
    pGridLayout->addWidget(pOnlineDevLabel,1,0,1,1);

    d_pOnlineDevValueLabel = new QLabel(QObject::tr("0"));
    d_pOnlineDevValueLabel->setFixedHeight(30);
    d_pOnlineDevValueLabel->setStyleSheet(tr("font: 18pt; color:rgb(117,250,0);"));
    pGridLayout->addWidget(d_pOnlineDevValueLabel,1,1,1,1);

    QLabel *pHttpServerStateLabel = new QLabel(QObject::tr("上传服务:"));
    pHttpServerStateLabel->setFixedSize(80,30);
    pGridLayout->addWidget(pHttpServerStateLabel,2,0,1,1);

    d_pHttpServerStateValueLabel = new QLabel(QObject::tr("已停止"));
    d_pHttpServerStateValueLabel->setFixedHeight(30);
    d_pHttpServerStateValueLabel->setStyleSheet(tr("font: 18pt; color:rgb(117,250,0);"));
    pGridLayout->addWidget(d_pHttpServerStateValueLabel,2,1,1,1);

    QLabel *pDatabaseStateLabel = new QLabel(QObject::tr("数据库:"));
    pDatabaseStateLabel->setFixedSize(80,30);
    pGridLayout->addWidget(pDatabaseStateLabel,3,0,1,1);

    d_pDatabaseStateValueLabel = new QLabel(QObject::tr("已断开"));
    d_pDatabaseStateValueLabel->setFixedHeight(30);
    d_pDatabaseStateValueLabel->setStyleSheet(tr("font: 18pt; color:red;"));
    pGridLayout->addWidget(d_pDatabaseStateValueLabel,3,1,1,1);


    pHLyt->addLayout(pGridLayout);
    QSpacerItem *pRightSpace = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Maximum);
    pHLyt->addSpacerItem(pRightSpace);
    pVMainLyt->addLayout(pHLyt);
    QSpacerItem *pBottomSpace = new QSpacerItem(20, 20, QSizePolicy::Maximum, QSizePolicy::Expanding);
    pVMainLyt->addSpacerItem(pBottomSpace);
    setLayout(pVMainLyt);

    QTimer *pTime = new QTimer(this);
    connect(pTime,SIGNAL(timeout()),this,SLOT(timeUpdate()));
    pTime->start(1000);

    connect(&m_Notify,SIGNAL(S_OnConnected(QString,int)),this,SLOT(OnDevConnect(QString,int)));

    StartSvc();
}

QSvcStatePage::~QSvcStatePage()
{

}

bool QSvcStatePage::IsStart()
{
    return m_IsRunning;
}
void QSvcStatePage::StartSvc()
{
    if(m_IsRunning==false)
    {
        QString AppDir = QCoreApplication::applicationDirPath();
        AppDir.append("/ServerLocalConfig.xml");

        if(!GetInst(LocalConfig).load_local_config(AppDir.toLatin1().constData()))
            return;
        if(GetInst(DataBaseOperation).OpenDb(GetInst(LocalConfig).database_ip(),
                                             "postgres",
                                             GetInst(LocalConfig).database_user(),
                                             GetInst(LocalConfig).database_password()))
        {
            d_pDatabaseStateValueLabel->setStyleSheet("color:rgb(117,250,0)");
            d_pDatabaseStateValueLabel->setText(tr("连接正常"));
            GetInst(StationConfig).load_station_config();
            emit updateDevList(true);
        }
        else  {
           // d_pDatabaseStateValueLabel->setStyleSheet("color:red");
           // d_pDatabaseStateValueLabel->setText(tr("已断开"));
            return;
        }

        GetInst(hx_net::SvcMgr).set_notify(&m_Notify);
        GetInst(hx_net::SvcMgr).Start();
        m_IsRunning=true;
        d_pSvcStateValueLabel->setText(tr("正在运行"));
        d_pHttpServerStateValueLabel->setText("正在运行");
        //emit updateMenu();
    }
    else
    {
        GetInst(hx_net::SvcMgr).Stop();
       // GetInst(DataBaseOperation).CloseDb();//清理数据库
        m_IsRunning=false;
        d_pSvcStateValueLabel->setText(tr("已停止"));
        emit updateDevList(false);
    }

}

void QSvcStatePage::OnDevConnect(QString sDevId,int nResult)//const char* sDevId,
{
    /*QString strTmp(tr(""));
    QString str(tr("无在线设备"));
    int nModleCount = GetInst(net::SvcMgr).get_modle_online_count();
    if(nModleCount>=0)
    {
        strTmp = tr("，转换模块[%1]");
        str+=strTmp.arg(nModleCount);
    }
    m_pOnlineDevValueLabel->setText(str);
    m_pOnlineDevValueLabel->setToolTip(str);*/

}
void QSvcStatePage::timeUpdate()
{
    QDateTime current_date_time = QDateTime::currentDateTime();
    QLocale lo = QLocale::Chinese;
    QString current_date = lo.toString(current_date_time,"yyyy-MM-dd hh:mm:ss dddd");
    d_plbDateTime->setText(current_date);
}
