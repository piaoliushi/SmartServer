#include "qnotifyhandler.h"
#include "MsgDefine.h"

QNotifyHandler::QNotifyHandler(QObject *parent)
	: QObject(parent)
{

}

QNotifyHandler::~QNotifyHandler()
{

}

void QNotifyHandler::OnConnected(string sDevId,int nResult)
{
	emit S_OnConnected(QString(QString::fromLocal8Bit(sDevId.c_str())),nResult);
}

void QNotifyHandler::OnRunState(string sDevId,int nResult)
{
	emit S_OnRunState(QString(QString::fromLocal8Bit(sDevId.c_str())),nResult);
}

void QNotifyHandler::OnClientOnline(string sIp,short port)
{
	QString sEndpoint = QString(tr("%1:%2")).arg(QString(QString::fromLocal8Bit(sIp.c_str())))\
		                .arg(port);
	emit S_OnClientOnline(sEndpoint);
}

void QNotifyHandler::OnClientOffline(string sIp,short port)
{
	QString sEndpoint = QString(tr("%1:%2")).arg(QString(QString::fromLocal8Bit(sIp.c_str())))\
		.arg(port);
	emit S_OnClientOffline(sEndpoint);
}

void QNotifyHandler::OnClientLogin(string sIp,short port,string sName,string sID)
{
	QString sEndpoint = QString(tr("%1:%2")).arg(QString::fromLocal8Bit(sIp.c_str()))\
		.arg(port);
	emit S_OnClientLogin(sEndpoint,QString::fromLocal8Bit(sName.c_str()),QString::fromLocal8Bit(sID.c_str()));
}

void QNotifyHandler::OnClientLogout(string sIp,short port)
{
	QString sEndpoint = QString(tr("%1:%2")).arg(QString(QString::fromLocal8Bit(sIp.c_str())))\
		.arg(port);
	emit S_OnClientLogout(sEndpoint);
}

// void QNotifyHandler::OnUpdateDevCount()
// {
// 	emit S_UpdateDevCount();
// }
// void QNotifyHandler::OnUpdateClientCount()
// {
// 	emit S_UpdateClientCount();
// }