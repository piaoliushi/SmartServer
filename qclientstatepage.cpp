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
    QLabel*pClientStatic = new QLabel(tr("在线用户："));
	pHlyt->addWidget(pClientStatic);
    QLabel*pClientSize = new QLabel(tr(""));
	pHlyt->addWidget(pClientSize);
	
	pHMainLyt->addLayout(pHlyt);
	pClientList = new QTableWidget(this);
    pClientList->horizontalHeader()->setVisible(false);
    pClientList->setColumnCount(3);
    pClientList->setFocusPolicy(Qt::NoFocus);
	QStringList header; 
    header<<tr("用户名")<<tr("IP地址")<<tr("登陆时间");
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
    pClientList->setItem(nrow,0,new QTableWidgetItem(QString(tr(" none"))));
	QDateTime time=QDateTime::currentDateTime();
    pClientList->setItem(nrow,1,new QTableWidgetItem(sAddr));
    pClientList->setItem(nrow,2,new QTableWidgetItem(time.toString("yy/MM/dd hh:mm:ss")));

	pClientList->resizeColumnsToContents();
	pClientList->horizontalHeader()->setStretchLastSection(true);
}

void QClientStatePage::OnClientOffline(QString sAddr)
{
	for(int irow=0;irow<pClientList->rowCount();++irow)
	{
        QTableWidgetItem *p = pClientList->item(irow,1);
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
        QTableWidgetItem *p = pClientList->item(irow,1);
		QString sContent = p->text();
		if(sContent==sAddr)
		{
			pClientList->setItem(irow,0,new QTableWidgetItem(sName));
            //pClientList->setItem(irow,1,new QTableWidgetItem(sID));
			return;
		}
	}
}
void QClientStatePage::OnClientLogout(QString sAddr)
{
	for(int irow=0;irow<pClientList->rowCount();++irow)
	{
        QTableWidgetItem *p = pClientList->item(irow,1);
		QString sContent = p->text();
		if(sContent==sAddr)
		{
			pClientList->setItem(irow,0,new QTableWidgetItem(QString(tr("****"))));
            //pClientList->setItem(irow,1,new QTableWidgetItem(QString(tr("****"))));
			return;
		}
	}
}
