#include "qsvcstatepage.h"
#include <QGridLayout>
#include <QProcess>
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
#include "./protocol/bohui_protocol.h"
using namespace db;

QSvcStatePage::QSvcStatePage(QNotifyHandler &Notify,QWidget *parent)
    : QWidget(parent)
    ,m_Notify(Notify)
    ,m_IsRunning(false)
    ,d_pOnlineDevValueLabel(NULL)
    ,d_bUseNtp(false)
    ,d_nModNtp(1)
    ,d_nValueNtp(2)
{
    QHBoxLayout *pTopLyt =new QHBoxLayout();
    QHBoxLayout *pHLyt = new QHBoxLayout();
    QVBoxLayout *pVMainLyt = new QVBoxLayout();

    QSpacerItem *pLeftTopSpace = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Maximum);
    d_plbDateTime = new QLabel(this);
    d_plbDateTime->setFixedSize(260,20);
#ifdef ARM_LINUX_DEF
    d_plbDateTime->setStyleSheet(tr("font: 18pt; color:#45c9d5;"));
#else
    d_plbDateTime->setStyleSheet(tr("font: 14pt; color:#45c9d5;"));
#endif
    pTopLyt->addSpacerItem(pLeftTopSpace);
    QPushButton *pAdjustTimeBt = new QPushButton(this);
    pAdjustTimeBt->setFocusPolicy(Qt::NoFocus);
    pAdjustTimeBt->setIcon(QIcon(":/new/images/clock.png"));
    pAdjustTimeBt->setFlat(true);
    connect(pAdjustTimeBt,SIGNAL(clicked()),this,SLOT(onMunualAdjustTime()));
    pTopLyt->addWidget(pAdjustTimeBt);
    pTopLyt->addWidget(d_plbDateTime);

    pVMainLyt->addLayout(pTopLyt);

    QSpacerItem *pSpaceTop = new QSpacerItem(40, 40, QSizePolicy::Fixed, QSizePolicy::Fixed);
    pVMainLyt->addSpacerItem(pSpaceTop);

    QSpacerItem *pSpaceLeft = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Maximum);
    pHLyt->addSpacerItem(pSpaceLeft);
    QGridLayout *pGridLayout = new QGridLayout();
    pGridLayout->setObjectName(QString::fromUtf8("SvcStateLayout"));
    QLabel *pSvcStateLabel = new QLabel(QObject::tr("LocalServer:"));
    pSvcStateLabel->setFixedSize(80,30);
    pGridLayout->addWidget(pSvcStateLabel,0,0,1,1);

    d_pSvcStateValueLabel = new QLabel(QObject::tr("Stopped"));
    d_pSvcStateValueLabel->setFixedHeight(30);
    d_pSvcStateValueLabel->setStyleSheet(tr("font: 18pt; color:rgb(117,250,0);"));
    pGridLayout->addWidget(d_pSvcStateValueLabel,0,1,1,1);


    QLabel *pHttpServerStateLabel = new QLabel(QObject::tr("UploadServer:"));
    pHttpServerStateLabel->setFixedSize(80,30);
    pGridLayout->addWidget(pHttpServerStateLabel,1,0,1,1);

    d_pHttpServerStateValueLabel = new QLabel(QObject::tr("Stopped"));
    d_pHttpServerStateValueLabel->setFixedHeight(30);
    d_pHttpServerStateValueLabel->setStyleSheet(tr("font: 18pt; color:rgb(117,250,0);"));
    pGridLayout->addWidget(d_pHttpServerStateValueLabel,1,1,1,1);

    QLabel *pDatabaseStateLabel = new QLabel(QObject::tr("Database:"));
    pDatabaseStateLabel->setFixedSize(80,30);
    pGridLayout->addWidget(pDatabaseStateLabel,2,0,1,1);

    d_pDatabaseStateValueLabel = new QLabel(QObject::tr("Disconnect"));
    d_pDatabaseStateValueLabel->setFixedHeight(30);
    d_pDatabaseStateValueLabel->setStyleSheet(tr("font: 18pt; color:red;"));
    pGridLayout->addWidget(d_pDatabaseStateValueLabel,2,1,1,1);


    GetInst(LocalConfig).ntp_config(d_bUseNtp,d_nModNtp,d_nValueNtp,d_sTimeNtp);

    QLabel *pAdjustTime = new QLabel(QObject::tr("AutoAjustTime:"));
    pAdjustTime->setFixedSize(80,30);
    pGridLayout->addWidget(pAdjustTime,3,0,1,1);
    d_pABaseTimeLabel = new QLabel(tr("00:00:00"));
    d_pABaseTimeLabel->setStyleSheet(tr("font: 16pt; color:rgb(117,250,0);"));
    d_pABaseTimeLabel->setFixedHeight(60);
    pGridLayout->addWidget(d_pABaseTimeLabel,3,1,1,1);
     d_pAdjustTimeModLabel = new QLabel(QObject::tr("Weekly"));
     d_pAdjustTimeModLabel->setStyleSheet(tr("font: 16pt; color:rgb(117,250,0);"));
     pGridLayout->addWidget(d_pAdjustTimeModLabel,3,2,1,1);

     d_pABaseTimeLabel->setText(d_sTimeNtp.c_str());
     if(d_bUseNtp==false){
          d_pABaseTimeLabel->setStyleSheet(tr("font: 16pt; color:gray;"));
          d_pAdjustTimeModLabel->setStyleSheet(tr("font: 16pt; color:gray;"));
     }else
     {
         if(d_nModNtp==0)
             d_pAdjustTimeModLabel->setText(QObject::tr("Daily"));
         else if(d_nModNtp==2)
              d_pAdjustTimeModLabel->setText(QObject::tr("Monthly"));

     }





    pGridLayout->addWidget(d_pDatabaseStateValueLabel,2,1,1,1);



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

    connect(&m_Notify,SIGNAL(S_OnDatabase(bool)),this,SLOT(OnDatabase(bool)));


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
        GetInst(DataBaseOperation).set_notify(&m_Notify);
        if(GetInst(DataBaseOperation).OpenDb(GetInst(LocalConfig).database_ip(),
                                             GetInst(LocalConfig).database_name(),
                                             GetInst(LocalConfig).database_user(),
                                             GetInst(LocalConfig).database_password()))
        {
            d_pDatabaseStateValueLabel->setStyleSheet("font: 18pt;color:rgb(117,250,0)");
            d_pDatabaseStateValueLabel->setText(tr("connect ok"));
            if(GetInst(StationConfig).load_station_config()==true){

                GetInst(DataBaseOperation).GetDataDictionary(Bohui_Protocol::mapTypeToStr);
                emit updateDevList(true);

            }
        }
        else  {
             GetInst(DataBaseOperation).CloseDb();
            return;
        }

        GetInst(hx_net::SvcMgr).set_notify(&m_Notify);
        GetInst(hx_net::SvcMgr).Start();
        m_IsRunning=true;
        d_pSvcStateValueLabel->setText(tr("running"));
        d_pHttpServerStateValueLabel->setText(tr("running"));

    }
    else
    {
        GetInst(hx_net::SvcMgr).Stop();
        m_IsRunning=false;
        d_pSvcStateValueLabel->setText(tr("stopped"));
        emit updateDevList(false);
    }

}

void QSvcStatePage::OnDatabase(bool bOk)
{
    if(bOk){
        d_pDatabaseStateValueLabel->setStyleSheet("font: 18pt;color:rgb(117,250,0)");
        d_pDatabaseStateValueLabel->setText(tr("connect ok"));
    }else
    {
        d_pDatabaseStateValueLabel->setStyleSheet("font: 18pt;color:red");
        d_pDatabaseStateValueLabel->setText(tr("disconnect"));
    }
}

void QSvcStatePage::timeUpdate()
{
    QDateTime current_date_time = QDateTime::currentDateTime();
    QLocale lo = QLocale::Chinese;
    QString current_date = lo.toString(current_date_time,"yyyy-MM-dd hh:mm:ss dddd");
    d_plbDateTime->setText(current_date);

    if(IsStart()==false){
        StartSvc();
        onMunualAdjustTime();
    }
    if(d_bUseNtp)
        checkAutoAdjustTime();

}

 void QSvcStatePage::onMunualAdjustTime()
 {
     QString sexecute = QString("./ntpclient -s -t -i 1 -h %1 &").arg(GetInst(LocalConfig).ntp_svc_ip().c_str());
     QProcess::execute(sexecute);
     sexecute = QString("hwclock -w");
     QProcess::execute(sexecute);
 }

 void QSvcStatePage::checkAutoAdjustTime()
 {
     QDateTime curTm = QDateTime::currentDateTime();
     QDateTime configTm = QDateTime::fromString(d_sTimeNtp.c_str(),"hh:mm:ss");
     if(configTm.isValid()==false){
           return;
     }
     if(  curTm.time().secsTo(configTm.time()) == 0){
         if(d_nModNtp==0){//按天
             onMunualAdjustTime();
         }else if(d_nModNtp==1){//按星期
             if(curTm.date().dayOfWeek() == d_nValueNtp)
                 onMunualAdjustTime();
         }else if(d_nModNtp==2){//按月
             if(curTm.date().day()==d_nValueNtp)
                  onMunualAdjustTime();
         }
     }
 }
