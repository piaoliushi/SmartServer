#include "qclientstatepage.h"
#include <QHeaderView>
#include <QBoxLayout>
#include <QLabel>
#include <QDateTime>


QClientStatePage::QClientStatePage(QNotifyHandler &Notify,QWidget *parent)
	: QWidget(parent)
	,m_Notify(Notify)
{
	QVBoxLayout *pHMainLyt = new QVBoxLayout(this);
	QHBoxLayout *pHlyt = new QHBoxLayout();
	QLabel*pClientStatic = new QLabel(tr("在线用户数："));
	pHlyt->addWidget(pClientStatic);
	QLabel*pClientSize = new QLabel(tr("0"));
	pHlyt->addWidget(pClientSize);
// 
	
	pHMainLyt->addLayout(pHlyt);
	pClientList = new QTableWidget(this);
	pClientList->setColumnCount(5);
	QStringList header; 
	header<<tr("用户名称")<<tr("用户ID")<<tr("用户类型")<<tr("IP地址")<<tr("登陆时间"); 
	pClientList->setHorizontalHeaderLabels(header); 
	pClientList->setEditTriggers(QAbstractItemView::NoEditTriggers);
	pClientList->setSelectionBehavior ( QAbstractItemView::SelectRows); //设置选择行为，以行为单位
	pClientList->setSelectionMode ( QAbstractItemView::SingleSelection); //设置选择模式，选择单行
	pHMainLyt->addWidget(pClientList);


 	
	connect(&m_Notify,SIGNAL(S_OnClientOnline(QString)),this,SLOT(OnClientOnline(QString)));
	connect(&m_Notify,SIGNAL(S_OnClientOffline(QString)),this,SLOT(OnClientOffline(QString)));
	connect(&m_Notify,SIGNAL(S_OnClientLogin(QString,QString,QString)),this,SLOT(OnClientLogin(QString,QString,QString)));
	connect(&m_Notify,SIGNAL(S_OnClientLogout(QString)),this,SLOT(OnClientLogout(QString)));
	
}

QClientStatePage::~QClientStatePage()
{

}

void QClientStatePage::OnClientOnline(QString sAddr)
{
	int nrow = pClientList->rowCount();
	pClientList->insertRow(nrow);
	pClientList->setItem(nrow,0,new QTableWidgetItem(QIcon(tr(":/users32x32.png")),QString(tr(" none"))));
	pClientList->setItem(nrow,1,new QTableWidgetItem(QString(tr("****"))));
	pClientList->setItem(nrow,2,new QTableWidgetItem(QString(tr("****"))));

	QDateTime time=QDateTime::currentDateTime();
	pClientList->setItem(nrow,3,new QTableWidgetItem(sAddr));
	pClientList->setItem(nrow,4,new QTableWidgetItem(time.toString("yyyy-MM-dd hh:mm:ss dddd")));

	pClientList->resizeColumnsToContents();
	pClientList->horizontalHeader()->setStretchLastSection(true);
}

void QClientStatePage::OnClientOffline(QString sAddr)
{
	for(int irow=0;irow<pClientList->rowCount();++irow)
	{
		QTableWidgetItem *p = pClientList->item(irow,3);
		QString sContent = p->text();
		if(sContent==sAddr)
		{
			pClientList->removeRow(irow);
			//pClientList->resizeColumnsToContents();
			//pClientList->horizontalHeader()->setStretchLastSection(true);
			return;
		}
	}
}


void QClientStatePage::OnClientLogin(QString sAddr,QString sName,QString sID)
{
	for(int irow=0;irow<pClientList->rowCount();++irow)
	{
		QTableWidgetItem *p = pClientList->item(irow,3);
		QString sContent = p->text();
		if(sContent==sAddr)
		{
			pClientList->setItem(irow,0,new QTableWidgetItem(sName));
			pClientList->setItem(irow,1,new QTableWidgetItem(sID));
			return;
		}
	}
}
void QClientStatePage::OnClientLogout(QString sAddr)
{
	for(int irow=0;irow<pClientList->rowCount();++irow)
	{
		QTableWidgetItem *p = pClientList->item(irow,3);
		QString sContent = p->text();
		if(sContent==sAddr)
		{
			pClientList->setItem(irow,0,new QTableWidgetItem(QString(tr("****"))));
			pClientList->setItem(irow,1,new QTableWidgetItem(QString(tr("****"))));
			return;
		}
	}
}