#ifndef QSYSTEMINFOPAGE_H
#define QSYSTEMINFOPAGE_H

#include <QObject>
#include "qnotifyhandler.h"
#include <QTableWidget>
#include <QLabel>
#include <map>
class QLineEdit;
class QHBoxLayout;
class QPushButton;
class QComboBox;
class QGroupBox;
class QCheckBox;
class QSystemInfoPage : public QWidget
{
	Q_OBJECT

public:
    QSystemInfoPage(QWidget *parent);
    ~QSystemInfoPage();

protected:
	QHBoxLayout* createTitleLayout(QString sTitle);
	void loadConfigData();
public slots:
	void unLockEdit(bool bOk=true);
	void saveCurConfig();
	//void testCom();
	void transmitterChanged(const QString);
	void moidfyParm();
	void setAutoStart(bool is_auto_start);
	bool isAutoStart();
private:
	QLineEdit *d_svcNumber;
	QLineEdit *d_svcPort;
	QLineEdit *d_stationNumber;
	QLineEdit *d_stationName;
	QLineEdit *d_databaseIp;
	QLineEdit *d_databaseUser;
	QComboBox *d_comPort;
	QComboBox *d_baudRate;
	QLineEdit *d_smsCenterNumber;
	QGroupBox *d_smsEnable;
	QPushButton *d_testSmsBt;
	QPushButton *d_modifyBt;
	QPushButton *d_savebt;


	QPushButton *d_btModify;
	QComboBox   *d_Transmitters;
	QCheckBox   *d_UseAutoSwitch;
	QLineEdit   *d_PowerLimit;
	QCheckBox   *d_pAutoRun;

};

#endif // QSYSTEMINFOPAGE_H
