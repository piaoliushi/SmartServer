#include "qnotifyhandler.h"
#include "MsgDefine.h"

QNotifyHandler::QNotifyHandler(QObject *parent)
	: QObject(parent)
{

}

QNotifyHandler::~QNotifyHandler()
{

}

void QNotifyHandler::OnDevStatus(string sDevId,int nResult)
{
    emit S_OnDevStatus(sDevId.c_str(),nResult);
}

void QNotifyHandler::OnClientOnline(string sIp,short port)
{
    QString sEndpoint = QString(tr("%1:%2")).arg(sIp.c_str()).arg(port);
	emit S_OnClientOnline(sEndpoint);
}

void QNotifyHandler::OnClientOffline(string sIp,short port)
{
    QString sEndpoint = QString(tr("%1:%2")).arg(sIp.c_str()).arg(port);
	emit S_OnClientOffline(sEndpoint);
}

void QNotifyHandler::OnClientLogin(string sIp,short port,string sName,string sID)
{
    QString sEndpoint = QString(tr("%1:%2")).arg(sIp.c_str()).arg(port);
    emit S_OnClientLogin(sEndpoint,sName.c_str(),sID.c_str());
}

void QNotifyHandler::OnClientLogout(string sIp,short port)
{
    QString sEndpoint = QString(tr("%1:%2")).arg(sIp.c_str()).arg(port);
	emit S_OnClientLogout(sEndpoint);
}
