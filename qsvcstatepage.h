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
        void OnDatabase(bool bOk);
        void onMunualAdjustTime();
protected:
        void checkAutoAdjustTime();
private:
	QNotifyHandler &m_Notify;
	bool m_IsRunning;

    QLabel   *d_pOnlineDevValueLabel;
    QLabel   *d_pSvcStateValueLabel;
    QLabel   *d_pHttpServerStateValueLabel;
    QLabel   *d_pDatabaseStateValueLabel;
    QLabel   *d_pAdjustTimeModLabel;
    QLabel   *d_pABaseTimeLabel;
    QLabel   *d_plbDateTime;

    QPushButton *d_pStartBt;

    bool     d_bUseNtp;
    int      d_nModNtp;
    int      d_nValueNtp;
    string   d_sTimeNtp;
};

#endif // QSVCSTATEPAGE_H
