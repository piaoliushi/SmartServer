#include "qsysteminfopage.h"
#include <QHeaderView>
#include <QPushButton>
#include <QBoxLayout>
#include <QLabel>
#include <vector>
#include <string>
#include <QLineEdit>
#include <QComboBox>
#include <QGroupBox>
#include <QSettings>
#include <QCheckBox>
#include<QCoreApplication>
#include <QApplication>
#include<QMessageBox>
#include "./net/SvcMgr.h"
#include "DataType.h"
#include "StationConfig.h"
#include "ErrorCode.h"
#include "LocalConfig.h"
//#include "./sms/Comm.h"
//#include "./sms/Sms.h"
//#include "./sms/SmsTraffic.h"
using namespace std;

#define REG_RUN "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"

QSystemInfoPage::QSystemInfoPage(QWidget *parent)
	: QWidget(parent)
{ 
	QVBoxLayout *pHMainLyt = new QVBoxLayout();
	  pHMainLyt->addLayout(createTitleLayout(tr("服务属性")));
	 QHBoxLayout *pHlyt = new QHBoxLayout();
	  pHlyt->setContentsMargins(10,0,0,10);
	  QLabel *staticLabel = new QLabel(tr("服务编号："));
      pHlyt->addWidget(staticLabel);
	  d_svcNumber = new QLineEdit(this);
	  pHlyt->addWidget(d_svcNumber);
	  staticLabel = new QLabel(tr("服务端口："));
	  pHlyt->addWidget(staticLabel);
	  d_svcPort = new QLineEdit(this);
	  pHlyt->addWidget(d_svcPort);

	  d_pAutoRun = new QCheckBox(tr("开机自动运行"),this);
	  pHlyt->addWidget(d_pAutoRun);
	
	  pHMainLyt->addLayout(pHlyt);

	   pHMainLyt->addLayout(createTitleLayout(tr("台站属性")));
	  pHlyt = new QHBoxLayout();
	   pHlyt->setContentsMargins(10,0,0,10);
	  staticLabel = new QLabel(tr("台站编号："));
	  pHlyt->addWidget(staticLabel);
	  d_stationNumber = new QLineEdit(this);
	  pHlyt->addWidget(d_stationNumber);
	  staticLabel = new QLabel(tr("台站名称："));
	  pHlyt->addWidget(staticLabel);
	  d_stationName = new QLineEdit(this);
	  pHlyt->addWidget(d_stationName);
	  pHMainLyt->addLayout(pHlyt);

	  pHMainLyt->addLayout(createTitleLayout(tr("数据库属性")));
	  pHlyt = new QHBoxLayout();
	  pHlyt->setContentsMargins(10,0,0,10);
	  staticLabel = new QLabel(tr("连接地址："));
	  pHlyt->addWidget(staticLabel);
	  d_databaseIp = new QLineEdit(this);
	  pHlyt->addWidget(d_databaseIp);
	  staticLabel = new QLabel(tr("用 户 名："));
	  pHlyt->addWidget(staticLabel);
	  d_databaseUser = new QLineEdit(this);
	  pHlyt->addWidget(d_databaseUser);
	  pHMainLyt->addLayout(pHlyt);

	  pHlyt = new QHBoxLayout();
	  pHlyt->setContentsMargins(10,50,10,10);
	  QSpacerItem *pSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	  pHlyt->addSpacerItem(pSpacer);
	  d_modifyBt = new QPushButton(tr("修改"));
	  connect(d_modifyBt,SIGNAL(clicked()),this,SLOT(unLockEdit()));
	  d_modifyBt->setFixedSize(60,24);
	  pHlyt->addWidget(d_modifyBt);
	  d_savebt = new QPushButton(tr("保存"));
	  connect(d_savebt,SIGNAL(clicked()),this,SLOT(saveCurConfig()));
	  d_savebt->setFixedSize(60,24);
	  pHlyt->addWidget(d_savebt);

	  pHMainLyt->addLayout(pHlyt);


	  pSpacer = new QSpacerItem(40, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
	  pHMainLyt->addSpacerItem(pSpacer);



	 setLayout(pHMainLyt);

     //loadConfigData();
	
	 unLockEdit(false);

 
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
	string databaseIp = GetInst(LocalConfig).database_ip();
	string databaseUser = GetInst(LocalConfig).database_user();
	int svcPort = GetInst(LocalConfig).local_port();

	d_svcNumber->setText(QString::fromUtf8(svcNumber.c_str()));
	d_svcPort->setText(QString::number(svcPort));
	d_stationNumber->setText(QString::fromUtf8(stationId.c_str()));
	d_stationName->setText(QString::fromUtf8(stationName.c_str()));
	d_databaseIp->setText(QString::fromUtf8(databaseIp.c_str()));
	d_databaseUser->setText(QString::fromUtf8(databaseUser.c_str()));


	bool bsms_use = GetInst(LocalConfig).sms_use();
	string comId = GetInst(LocalConfig).sms_com();
	int    baudRate = GetInst(LocalConfig).sms_baud_rate();
	string centerNumber = GetInst(LocalConfig).sms_center_number();

	d_smsEnable->setChecked(bsms_use);
	d_comPort->setCurrentIndex(d_comPort->findText(QString::fromUtf8(comId.c_str())));
	d_baudRate->setCurrentIndex(d_baudRate->findText(QString::number(baudRate)));
	d_smsCenterNumber->setText(QString::fromUtf8(centerNumber.c_str()));

	map<string,pTransmitterPropertyExPtr> &vTransmitter = GetInst(LocalConfig).transmitter_property_ex();
	map<string,pTransmitterPropertyExPtr>::iterator transmitter_iter = vTransmitter.begin();
	for(;transmitter_iter!=vTransmitter.end();++transmitter_iter)
	{
		d_Transmitters->addItem((*transmitter_iter).first.c_str());
	}
	
	d_pAutoRun->setChecked(isAutoStart());
}

void QSystemInfoPage::transmitterChanged(const QString sTrsmitNumber)
{
	pTransmitterPropertyExPtr curTransmitter = GetInst(LocalConfig).transmitter_property_ex(sTrsmitNumber.toStdString());
	if(curTransmitter)
	{
		if(curTransmitter->is_auto_switch_transmitter)
			d_UseAutoSwitch->setCheckState(Qt::Checked);
		else 
			d_UseAutoSwitch->setCheckState(Qt::Unchecked);
		d_PowerLimit->setText(QString::number(curTransmitter->switch_power_lower_limit_value));
	}
}

void QSystemInfoPage::moidfyParm()
{
	QString sCurTrsmitId = d_Transmitters->currentText();
	QString sSwitchPower = d_PowerLimit->text();
	bool bUse=false;
	if(d_UseAutoSwitch->checkState()==Qt::Checked)
		bUse=true;
	QString AppDir = QCoreApplication::applicationDirPath();
	AppDir.append("/ServerLocalConfig.xml");
	if(!GetInst(LocalConfig).writeSwitchPar(AppDir.toLocal8Bit().constData(),sCurTrsmitId.toLocal8Bit().constData()
		,bUse,sSwitchPower.toLocal8Bit().constData()))
		QMessageBox::information(this,tr("注意"),tr("保存自动倒备机配置参数失败，请检查！"));
	else
		QMessageBox::information(this,tr("提示"),tr("保存参数成功！"));
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
}
