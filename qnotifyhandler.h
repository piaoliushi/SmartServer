#ifndef QNOTIFYHANDLER_H
#define QNOTIFYHANDLER_H

#include <QObject>
#include <string>
using namespace std;


class QNotifyHandler : public QObject
{
	Q_OBJECT
public:
	QNotifyHandler(QObject *parent);
	~QNotifyHandler();
public:
	//已连接设备
    void OnDevStatus(string sDevId,int nResult);
	//客户端在线
	void OnClientOnline(string sIp,short port);
	//客户端离线
	void OnClientOffline(string sIp,short port);
	//用户登录
	void OnClientLogin(string sIp,short port,string sName,string sID);
	//用户退出
	void OnClientLogout(string sIp,short port);
    //数据库状态
    void OnDatabase(bool bOk);
signals:
    void S_OnDevStatus(QString sDevId,int nResult);
	void S_OnClientOnline(QString sIp);//
	void S_OnClientOffline(QString sIp);//客户端下线
	void S_OnClientLogin(QString sAddr,QString sName,QString sID);
	void S_OnClientLogout(QString sAddr);
    void S_OnDatabase(bool bOk);
};

#endif // QNOTIFYHANDLER_H
