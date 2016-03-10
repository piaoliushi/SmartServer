#ifndef QDEVSTATEPAGE_H
#define QDEVSTATEPAGE_H

#include <QObject>
#include "qnotifyhandler.h"
#include <QTableWidget>
#include <QTreeWidget>
#include <QLabel>
#include <map>
class QDevStatePage : public QWidget
{
	Q_OBJECT

public:
	QDevStatePage(QNotifyHandler &Notify,QWidget *parent);
	~QDevStatePage();
	void LoadDevToList();
	void clearDevList();
public slots:
		void OnDevConnect(QString sDevId,int nResult);
		void OnRunState(QString sDevId,int nResult);
private:
	QNotifyHandler &m_Notify;
	QTableWidget *pDevList;//QTreeWidget
	std::map<QString,int> m_mapListItems;

	QLabel*   pDevSize;
	QLabel*   pDevConSize;
	QLabel*   pDevRunSize;
};

#endif // QDEVSTATEPAGE_H
