#ifndef QSVCSTATEPAGE_H
#define QSVCSTATEPAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include "qnotifyhandler.h"

class QSvcStatePage : public QWidget
{
	Q_OBJECT

public:
	QSvcStatePage(QNotifyHandler &Notify,QWidget *parent=NULL);
	~QSvcStatePage();
public:
	bool IsStart();

 signals:
 		void updateMenu();
		void updateDevList(bool);
public slots:
		void StartSvc();
        void timeUpdate();
		void OnDevConnect(QString sDevId,int nResult);
private:
	QNotifyHandler &m_Notify;
	bool m_IsRunning;

    QLabel   *d_pOnlineDevValueLabel;
    QLabel   *d_pSvcStateValueLabel;
    QLabel   *d_plbDateTime;
};

#endif // QSVCSTATEPAGE_H
