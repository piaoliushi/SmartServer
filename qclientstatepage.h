#ifndef QCLIENTSTATEPAGE_H
#define QCLIENTSTATEPAGE_H

#include <QWidget>
#include "qnotifyhandler.h"
#include <QTableWidget>
class QClientStatePage : public QWidget
{
	Q_OBJECT

public:
	QClientStatePage(QNotifyHandler &Notify,QWidget *parent);
	~QClientStatePage();
public slots:
	void OnClientOnline(QString sAddr);
	void OnClientOffline(QString sAddr);
	void OnClientLogin(QString sAddr,QString sName,QString sID);
	void OnClientLogout(QString sAddr);
private:
	QNotifyHandler &m_Notify;
	QTableWidget *pClientList;
};

#endif // QCLIENTSTATEPAGE_H
