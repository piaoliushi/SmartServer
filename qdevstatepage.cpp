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

	 pHMainLyt->addLayout(pHlyt);
	 pDevList = new QTableWidget(this);
     pDevList->setColumnCount(3);

	 QStringList header; 
     header<<tr("设备名称")<<tr("IP:PORT")<<tr("当前状态");
	 pDevList->setHorizontalHeaderLabels(header); 
	 pDevList->setEditTriggers(QAbstractItemView::NoEditTriggers);
	 pDevList->setSelectionBehavior ( QAbstractItemView::SelectRows); //设置选择行为，以行为单位
	 pDevList->setSelectionMode ( QAbstractItemView::SingleSelection); //设置选择模式，选择单行
	 pHMainLyt->addWidget(pDevList);

	 setLayout(pHMainLyt);
	 pDevList->horizontalHeader()->setStretchLastSection(true);
     connect(&m_Notify,SIGNAL(S_OnDevStatus(QString,int)),this,SLOT(OnDevStatus(QString,int)));
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

    vector<ModleInfo>& vModle = GetInst(StationConfig).get_Modle();
	vector<ModleInfo>::iterator Modleiter = vModle.begin();
	for(;Modleiter != vModle.end();Modleiter++)
	{
        map<string,DeviceInfo>::iterator iter = (*Modleiter).mapDevInfo.begin();
		for(;iter!=(*Modleiter).mapDevInfo.end();++iter)
		{	
			int nrow = pDevList->rowCount();
			pDevList->insertRow(nrow);
            QIcon sIcon;
            if(iter->second.iDevType == DEVICE_TRANSMITTER)
                sIcon.addFile(QString::fromUtf8(":/new/images/transmitter.png"));
            else if(iter->second.iDevType > DEVICE_TRANSMITTER && iter->second.iDevType <DEVICE_SWITCH)
                 sIcon.addFile(QString::fromUtf8(":/new/images/envir.png"));
            else
                   sIcon.addFile(QString::fromUtf8(":/new/images/link.png"));
            pDevList->setItem(nrow,0,new QTableWidgetItem(sIcon,(*iter).second.sDevName.c_str()));
            QString sEndpoint = QString(tr("%1:%2[%3]")).arg((*Modleiter).netMode.strIp.c_str()) \
                .arg((*Modleiter).netMode.ilocal_port).arg((*iter).second.iAddressCode);
            //pDevList->setItem(nrow,1,new QTableWidgetItem((*iter).second.sDevNum.c_str()));
            pDevList->setItem(nrow,1,new QTableWidgetItem(sEndpoint));
            pDevList->setItem(nrow,2,new QTableWidgetItem(tr("未知")));

            m_mapListItems[(*iter).first.c_str()]=nrow;
		}
    }

	int nDevNum = pDevList->rowCount();
	pDevSize->setText(QString(tr("设备数：%1")).arg(nDevNum));

	pDevList->resizeColumnsToContents();
}

void QDevStatePage::OnDevStatus(QString sDevId,int nResult)
{
	if(pDevList->rowCount()<=0)
		return;
	if(m_mapListItems.find(sDevId)==m_mapListItems.end())
		return;
    switch (nResult)
	{
    case -1://连接中断
        pDevList->item(m_mapListItems[sDevId],2)->setText(tr("中断"));
        pDevList->item(m_mapListItems[sDevId],2)->setTextColor(QColor(150,150,150));
		break;
    case 0://连接正常
        pDevList->item(m_mapListItems[sDevId],2)->setText(tr("已断开"));
        pDevList->item(m_mapListItems[sDevId],2)->setTextColor(QColor(0,0,255));
		break;
    case 1://已开机
        pDevList->item(m_mapListItems[sDevId],2)->setText(tr("已开机"));
        pDevList->item(m_mapListItems[sDevId],2)->setTextColor(QColor(0,255,0));
		break;
    case 2://已关机
        pDevList->item(m_mapListItems[sDevId],2)->setText(tr("已关机"));
        pDevList->item(m_mapListItems[sDevId],2)->setTextColor(QColor(255,255,0));
        break;
    case 3://天线->主机
        pDevList->item(m_mapListItems[sDevId],2)->setText(tr("天线->主机"));
        pDevList->item(m_mapListItems[sDevId],2)->setTextColor(QColor(0,255,0));
        break;
    case 4://天线->备机
        pDevList->item(m_mapListItems[sDevId],2)->setText(tr("天线->备机"));
        pDevList->item(m_mapListItems[sDevId],2)->setTextColor(QColor(0,255,0));
        break;
	}
	
}
