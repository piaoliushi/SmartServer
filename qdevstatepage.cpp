#include "qdevstatepage.h"
#include <QHeaderView>
#include <QBoxLayout>
#include <QLabel>
#include <vector>
#include "./net/SvcMgr.h"
#include "DataType.h"
#include "StationConfig.h"
//#include "../net/config.h"
#include "ErrorCode.h"


using namespace std;
QDevStatePage::QDevStatePage(QNotifyHandler &Notify,QWidget *parent)
	: QWidget(parent)
	,m_Notify(Notify)
	,pDevList(NULL)
{ 
	QVBoxLayout *pHMainLyt = new QVBoxLayout(this);
	
	QHBoxLayout *pHlyt = new QHBoxLayout();
	  pDevSize = new QLabel(tr("设备数："));
      pHlyt->addWidget(pDevSize);
	  pDevConSize = new QLabel(tr("已连接："));
      pHlyt->addWidget(pDevConSize);
	  pDevRunSize = new QLabel(tr("已运行："));
	  pHlyt->addWidget(pDevRunSize);

	 pHMainLyt->addLayout(pHlyt);
	 pDevList = new QTableWidget(this);
	 pDevList->setColumnCount(6);

	 QStringList header; 
	 header<<tr("类型")<<tr("ID")<<tr("名称")<<tr("IP:PORT")<<tr("连接状态")<<tr("运行状态"); 
	 pDevList->setHorizontalHeaderLabels(header); 
	 pDevList->setEditTriggers(QAbstractItemView::NoEditTriggers);
	 pDevList->setSelectionBehavior ( QAbstractItemView::SelectRows); //设置选择行为，以行为单位
	 pDevList->setSelectionMode ( QAbstractItemView::SingleSelection); //设置选择模式，选择单行
	 pHMainLyt->addWidget(pDevList);

	 setLayout(pHMainLyt);
	 pDevList->horizontalHeader()->setStretchLastSection(true);
	 connect(&m_Notify,SIGNAL(S_OnConnected(QString,int)),this,SLOT(OnDevConnect(QString,int)));
	 connect(&m_Notify,SIGNAL(S_OnRunState(QString,int)),this,SLOT(OnRunState(QString,int)));
}

QDevStatePage::~QDevStatePage()
{

}

void QDevStatePage::clearDevList()
{
	pDevList->clearContents();
	for(int i=pDevList->rowCount()-1;i>=0;--i)
		pDevList->removeRow(i);
}

void QDevStatePage::LoadDevToList()
{
	clearDevList();

	vector<TransmitterInformation>& vTransmitter = GetInst(StationConfig).get_transmitter();
	int nTransmitter =0;

	vector<TransmitterInformation>::iterator iter = vTransmitter.begin();
	for(;iter!=vTransmitter.end();++iter)
	{
		if(!(*iter).IsUsed)
			continue;
		int nrow = pDevList->rowCount();
		pDevList->insertRow(nrow);
		pDevList->setItem(nrow,0,new QTableWidgetItem(QIcon(tr(":/transmitter.png")),QString(tr("发射机"))));
		pDevList->setItem(nrow,1,new QTableWidgetItem(QString::fromLocal8Bit((*iter).sNumber.c_str())));
		pDevList->setItem(nrow,2,new QTableWidgetItem(QString::fromLocal8Bit((*iter).sProgramName.c_str())));
		QString sEndpoint = QString(tr("%1:%2")).arg(QString::fromLocal8Bit((*iter).sAddress.c_str()))\
			.arg((*iter).nPort);
		if((*iter).bUseAgent==true)
		{
			string sIp;
			int nPort;
			GetInst(StationConfig).get_transmitter_agent_endpoint_by_id((*iter).sNumber,sIp,nPort);
			sEndpoint = QString(tr("%1:%2[代理]")).arg(QString::fromLocal8Bit(sIp.c_str())).arg(nPort);
		}

		pDevList->setItem(nrow,3,new QTableWidgetItem(sEndpoint));
		pDevList->setItem(nrow,4,new QTableWidgetItem(QIcon(tr(":/clock.png")),tr("已断开!!!")));
		pDevList->setItem(nrow,5,new QTableWidgetItem(tr("未知")));

		m_mapListItems[QString(QString::fromLocal8Bit((*iter).sNumber.c_str()))]=nrow;

		nTransmitter=nrow;
	}

	vector<ModleInfo>& vModle = GetInst(StationConfig).get_Modle();
	vector<ModleInfo>::iterator Modleiter = vModle.begin();
	for(;Modleiter != vModle.end();Modleiter++)
	{
		map<string,DevParamerInfo>::iterator iter = (*Modleiter).mapDevInfo.begin();
		for(;iter!=(*Modleiter).mapDevInfo.end();++iter)
		{	
			int nrow = pDevList->rowCount();
			pDevList->insertRow(nrow);
			pDevList->setItem(nrow,0,new QTableWidgetItem(QIcon(tr(":/device_state.png")),QString::fromLocal8Bit((*Modleiter).sModName.c_str())));//QString(tr("转换模块"))
			pDevList->setItem(nrow,1,new QTableWidgetItem(QString::fromLocal8Bit((*iter).first.c_str())));
			pDevList->setItem(nrow,2,new QTableWidgetItem(QString::fromLocal8Bit((*iter).second.sDevName.c_str())));
			QString sEndpoint = QString(tr("%1:%2[%3]")).arg(QString::fromLocal8Bit((*Modleiter).sModIP.c_str()))\
				.arg((*Modleiter).nModPort).arg((*iter).second.nDevAddr);
			pDevList->setItem(nrow,3,new QTableWidgetItem(sEndpoint));
			pDevList->setItem(nrow,4,new QTableWidgetItem(QIcon(tr(":/clock.png")),tr("已断开!!!")));
			pDevList->setItem(nrow,5,new QTableWidgetItem(tr("未知")));
			m_mapListItems[QString::fromLocal8Bit((*iter).first.c_str())]=nrow;//(*Modleiter).sModleNumber.c_str()
		}
	}

	int nDevNum = pDevList->rowCount();
	pDevSize->setText(QString(tr("设备数：%1")).arg(nDevNum));

	pDevList->resizeColumnsToContents();
}

void QDevStatePage::OnDevConnect(QString sDevId,int nResult)
{
	if(pDevList->rowCount()<=0)
		return;
	if(m_mapListItems.find(sDevId)==m_mapListItems.end())
		return;
	switch ((con_state)nResult)
	{
	case con_connected:
		pDevList->item(m_mapListItems[sDevId],4)->setText(tr("已连接"));
		pDevList->item(m_mapListItems[sDevId],4)->setTextColor(QColor(0,0,255));
		break;
	case con_disconnected:
		pDevList->item(m_mapListItems[sDevId],4)->setText(tr("已断开"));
		pDevList->item(m_mapListItems[sDevId],4)->setTextColor(QColor(0,0,0));
		break;
	case con_connecting:
		pDevList->item(m_mapListItems[sDevId],4)->setText(tr("正在重连..."));
		pDevList->item(m_mapListItems[sDevId],4)->setTextColor(QColor(255,0,0));
		break;
	}

	int nConSize =   GetInst(net::SvcMgr).get_modle_online_count();
	if(nConSize<0)
		nConSize=0;
	pDevConSize->setText(QString(tr("已连接：%1")).arg(nConSize));
	
}

void QDevStatePage::OnRunState(QString sDevId,int nResult)
{
}
