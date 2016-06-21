﻿#include "qsysteminfopage.h"
#include <QHeaderView>
#include <QPushButton>
#include <QBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <vector>
#include <string>
#include <QLineEdit>
#include <QList>
#include <QComboBox>
#include <QGroupBox>
#include <QSettings>
#include <QCheckBox>
#include<QCoreApplication>
#include <QApplication>
#include<QMessageBox>
#include <QNetworkInterface>
#include <QNetworkAddressEntry>
#include <QPalette>
#include "./net/SvcMgr.h"
#include "DataType.h"
#include "StationConfig.h"
#include "ErrorCode.h"
#include "LocalConfig.h"
using namespace std;

QSystemInfoPage::QSystemInfoPage(QWidget *parent)
    : QWidget(parent)
{ 

    QVBoxLayout *pHMainLyt = new QVBoxLayout();

    pHMainLyt->addLayout(createTitleLayout(tr("台站属性")));
    //QHBoxLayout *pHlyt = new QHBoxLayout();
    QGridLayout *pGridLayout = new QGridLayout();
    pGridLayout->setContentsMargins(10,0,0,10);
    QLabel *staticLabel = new QLabel(tr("台站编号:"),this);
    staticLabel->setFixedWidth(70);
    pGridLayout->addWidget(staticLabel,0,0,1,1);

    d_stationNumber = new QLabel(this);
    d_stationNumber->setStyleSheet("color:#5fff53");
    //d_stationNumber->setPalette(palette);
    //d_stationNumber->setFrameShadow(QLabel::Sunken);

    pGridLayout->addWidget(d_stationNumber,0,1,1,1);
    staticLabel = new QLabel(tr("台站名称:"),this);
    staticLabel->setFixedWidth(70);

    pGridLayout->addWidget(staticLabel,0,2,1,1);
    d_stationName = new QLabel(this);
    d_stationName->setStyleSheet("color:#5fff53");


    pGridLayout->addWidget(d_stationName,0,3,1,1);

    staticLabel = new QLabel(tr("设备标识:"),this);
    staticLabel->setFixedWidth(70);
    pGridLayout->addWidget(staticLabel,1,0,1,1);
    d_deviceId = new QLabel(this);
    d_deviceId->setStyleSheet("color:#5fff53");
    pGridLayout->addWidget(d_deviceId,1,1,1,1);

    staticLabel = new QLabel(tr("平台标识:"),this);
    staticLabel->setFixedWidth(70);
    pGridLayout->addWidget(staticLabel,1,2,1,1);

    d_platformId = new QLabel(this);
    d_platformId->setStyleSheet("color:#5fff53");
    pGridLayout->addWidget(d_platformId,1,3,1,1);
    pHMainLyt->addLayout(pGridLayout);

    pHMainLyt->addLayout(createTitleLayout(tr("平台属性")));
    QVBoxLayout *pPltVlyt = new QVBoxLayout();
    pPltVlyt->setContentsMargins(10,0,0,10);
     QHBoxLayout * pRptHlyt = new QHBoxLayout();

     staticLabel = new QLabel(tr("平台服务:"),this);
     staticLabel->setFixedWidth(70);
     pRptHlyt->addWidget(staticLabel);
     d_RptUrl = new QLabel(this);
     d_RptUrl->setStyleSheet("color:#5fff53");
     pRptHlyt->addWidget(d_RptUrl);
     pPltVlyt->addLayout(pRptHlyt);

     QHBoxLayout * pNptHlyt = new QHBoxLayout();
     staticLabel = new QLabel(tr("校时服务:"),this);
     staticLabel->setFixedWidth(70);
     pNptHlyt->addWidget(staticLabel);
     d_NtpIp = new QLabel(this);
     d_NtpIp->setStyleSheet("color:#5fff53");
     pNptHlyt->addWidget(d_NtpIp);
     d_NtpEnable = new QCheckBox(this);
     d_NtpEnable->setStyleSheet("QCheckBox::indicator {width: 20px; height: 20px;}QCheckBox::indicator:checked \
     {image: url(:/new/images/checked.png);} QCheckBox::indicator:unchecked \
     {image: url(:/new/images/unchecked.png);}");

     d_NtpEnable->setFixedWidth(22);
     d_NtpEnable->setFocusPolicy(Qt::NoFocus);
     d_NtpEnable->setEnabled(false);
     d_NtpEnable->setCheckable(true);
     pNptHlyt->addWidget(d_NtpEnable);
     staticLabel = new QLabel(tr("启用"),this);
     staticLabel->setFixedWidth(40);
     pNptHlyt->addWidget(staticLabel);
     QSpacerItem *pNtpSpace = new QSpacerItem(20, 20, QSizePolicy::Expanding,QSizePolicy::Maximum );
     pNptHlyt->addSpacerItem(pNtpSpace);
     pPltVlyt->addLayout(pNptHlyt);

     pHMainLyt->addLayout(pPltVlyt);

     pHMainLyt->addLayout(createTitleLayout(tr("本机网络")));
    QHBoxLayout * pHlyt = new QHBoxLayout();
    pHlyt->setContentsMargins(10,0,0,10);
    staticLabel = new QLabel(tr("网卡1:"));
    staticLabel->setFixedWidth(65);
    pHlyt->addWidget(staticLabel);
    QLabel *ethIp0 = new QLabel(this);
    ethIp0->setStyleSheet("color:#5fff53");
    pHlyt->addWidget(ethIp0);
    QNetworkInterface   interface0 = QNetworkInterface::interfaceFromName("eth0");
    QList<QNetworkAddressEntry>  netlist0 = interface0.addressEntries();
    if(!netlist0.isEmpty())
        ethIp0->setText(netlist0.at(0).ip().toString());
    staticLabel = new QLabel(tr("网卡2:"));
    staticLabel->setFixedWidth(65);
    pHlyt->addWidget(staticLabel);
    QLabel *ethIp1 = new QLabel(this);
    ethIp1->setStyleSheet("color:#5fff53");
    pHlyt->addWidget(ethIp1);
    QNetworkInterface   interface1 = QNetworkInterface::interfaceFromName("eth1");
    QList<QNetworkAddressEntry>  netlist1 = interface1.addressEntries();
    if(!netlist1.isEmpty())
        ethIp1->setText(netlist1.at(0).ip().toString());
    else
        ethIp1->setText(tr("无"));

    pHMainLyt->addLayout(pHlyt);


    QSpacerItem *pBottomSpace = new QSpacerItem(20, 20, QSizePolicy::Maximum, QSizePolicy::Expanding);
    pHMainLyt->addSpacerItem(pBottomSpace);

    QHBoxLayout * pAboutHlyt = new QHBoxLayout();
    pBottomSpace = new QSpacerItem(20, 20, QSizePolicy::Expanding,QSizePolicy::Maximum );
    pAboutHlyt->addSpacerItem(pBottomSpace);
    QVBoxLayout *pAboutVlyt = new QVBoxLayout();
    QLabel *lbAboult = new QLabel("版权所有 © 2006-2016 安徽汇鑫电子有限公司",this);
    lbAboult->setStyleSheet(tr("font: 9pt; color:#45c9d5;"));
    pAboutVlyt->addWidget(lbAboult);
    lbAboult = new QLabel("电话:0551-65319365 , 65314565",this);
    lbAboult->setStyleSheet(tr("font: 10pt; color:#45c9d5;"));
     pAboutVlyt->addWidget(lbAboult);

    pAboutHlyt->addLayout(pAboutVlyt);

    pHMainLyt->addLayout(pAboutHlyt);
    setLayout(pHMainLyt);

    loadConfigData();

}

QSystemInfoPage::~QSystemInfoPage()
{

}

QHBoxLayout* QSystemInfoPage::createTitleLayout(QString sTitle)
{
    QHBoxLayout *titlelayout = new QHBoxLayout();
    titlelayout->setMargin(0);
    titlelayout->setSpacing(0);
    QFrame *hline = new QFrame;
    hline->setFrameShape(QFrame::HLine);
    hline->setFrameShadow(QFrame::Plain);
    QLabel *ipLabel = new QLabel;
    QFont f = ipLabel->font();
    f.setPointSize(12);
    ipLabel->setFont(f);
    ipLabel->setFixedWidth(80);
    ipLabel->setText(sTitle);
    titlelayout->addWidget(ipLabel);
    titlelayout->addWidget(hline);
    return titlelayout;
}

void QSystemInfoPage::unLockEdit(bool bOk)
{
}

void QSystemInfoPage::loadConfigData()
{
    string stationId = GetInst(LocalConfig).local_station_id();
    string stationName = GetInst(LocalConfig).local_station_name();
    string svcNumber = GetInst(LocalConfig).local_dev_server_number();
    string sSrcCode = GetInst(LocalConfig).src_code();
    string sDstCode = GetInst(LocalConfig).dst_code();
    string sRpturl = GetInst(LocalConfig).report_svc_url();
    string sNtpIp = GetInst(LocalConfig).ntp_svc_ip();

    d_stationNumber->setText(stationId.c_str());
    d_stationName->setText(stationName.c_str());
    d_deviceId->setText(sSrcCode.c_str());
    d_platformId->setText(sDstCode.c_str());
    d_RptUrl->setText(sRpturl.c_str());
    d_NtpIp->setText(sNtpIp.c_str());
    d_NtpEnable->setChecked(GetInst(LocalConfig).ntp_svc_use());

}

void QSystemInfoPage::transmitterChanged(const QString sTrsmitNumber)
{
    /*pTransmitterPropertyExPtr curTransmitter = GetInst(LocalConfig).transmitter_property_ex(sTrsmitNumber.toStdString());
    if(curTransmitter)
    {
        if(curTransmitter->is_auto_switch_transmitter)
            d_UseAutoSwitch->setCheckState(Qt::Checked);
        else
            d_UseAutoSwitch->setCheckState(Qt::Unchecked);
        d_PowerLimit->setText(QString::number(curTransmitter->switch_power_lower_limit_value));
    }*/
}

void QSystemInfoPage::moidfyParm()
{

}

void QSystemInfoPage::saveCurConfig()
{
    setAutoStart(d_pAutoRun->isChecked());

    string stationId = d_stationNumber->text().toUtf8().data();
    string stationName = d_stationName->text().toUtf8().data();
    string svcNumber = d_svcNumber->text().toUtf8().data();
    string databaseIp = d_databaseIp->text().toUtf8().data();
    string databaseUser = d_databaseUser->text().toUtf8().data();
    unsigned short svcPort = d_svcPort->text().toUShort();
    bool bsms_use = d_smsEnable->isChecked();
    string comId = d_comPort->currentText().toUtf8().data();
    int    baudRate = d_baudRate->currentText().toInt();
    string centerNumber = d_smsCenterNumber->text().toUtf8().data();
    QString AppDir = QCoreApplication::applicationDirPath();
    AppDir.append("/ServerLocalConfig.xml");
    if(GetInst(LocalConfig).writeLocalParToXml(AppDir.toLocal8Bit().constData(),stationId,stationName
                                               ,svcNumber,svcPort,databaseIp,databaseUser)==false ||
            GetInst(LocalConfig).writeSmsParToXml(AppDir.toLocal8Bit().constData(),bsms_use,comId,baudRate,centerNumber)==false)
    {
        QMessageBox::information(this,tr("注意"),tr("保存本地配置参数失败，请检查！"));
    }
    else
    {

        unLockEdit(false);
    }

}

void QSystemInfoPage::setAutoStart(bool is_auto_start)
{
}

bool QSystemInfoPage::isAutoStart()
{
    return false;
}
