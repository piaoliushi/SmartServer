#include "qsvcstatepage.h"
#include <QGridLayout>
#include <QDebug>
#include<QMessageBox>
#include <QDateTime>
#include <QCoreApplication>
#include <QLocale>
#include "./net/SvcMgr.h"
#include "./net/config.h"
#include "StationConfig.h"
#include "LocalConfig.h"

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

    QLabel *pOnlineClientLabel = new QLabel(QObject::tr("登陆客户:"));
    pOnlineClientLabel->setFixedSize(80,30);
    pGridLayout->addWidget(pOnlineClientLabel,2,0,1,1);

    QLabel *pOnlineClientValueLabel = new QLabel(QObject::tr("0"));
    pOnlineClientValueLabel->setFixedHeight(30);
    pOnlineClientValueLabel->setStyleSheet(tr("font: 18pt; color:rgb(117,250,0);"));
    pGridLayout->addWidget(pOnlineClientValueLabel,2,1,1,1);

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
        AppDir.append("/ServerLocalConfig_1.xml");

        if(!GetInst(LocalConfig).load_local_config(AppDir.toLatin1().constData()))
        {
            return;
        }

        if(GetInst(DbManager).OpenDb(GetInst(LocalConfig).database_ip(),
            "LsmpDataBase",
            GetInst(LocalConfig).database_user(),
            GetInst(LocalConfig).database_password(),1,GetInst(LocalConfig).database_drivername()))
        {
            GetInst(StationConfig).load_station_config();
            emit updateDevList(true);
        }
        else
        {
            setCursor(Qt::ArrowCursor);
            QMessageBox::information(this,tr("提示"),tr("打开数据库失败,请检查数据库配置是否正确!!!"));
            return;
        }

        GetInst(net::SvcMgr).set_notify(&m_Notify);
        GetInst(net::SvcMgr).Start();
        m_IsRunning=true;
       // pSvcStateValueLabel->setText(tr("正在运行"));
       // pStartBt->setText(tr("停止服务"));

        //emit updateMenu();
    }
    else
    {
        setCursor(Qt::WaitCursor);
        GetInst(net::SvcMgr).Stop();
        GetInst(DbManager).CloseDb();//清理数据库
        m_IsRunning=false;
        //pSvcStateValueLabel->setText(tr("已停止"));
        //pStartBt->setText(tr("启动服务"));
        //emit updateMenu();
        //emit updateDevList(false);
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
