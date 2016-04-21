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
    boost::recursive_mutex::scoped_lock lock(mutex_);
    emit S_OnDevStatus(sDevId.c_str(),nResult);
}

void QNotifyHandler::OnClientOnline(string sIp,short port)
{
    boost::recursive_mutex::scoped_lock lock(mutex_);
    //QString sEndpoint = QString(tr("%1:%2")).arg(sIp.c_str()).arg(port);sEndpoint
    emit S_OnClientOnline(QString::fromStdString(sIp));
}

//数据库状态
void QNotifyHandler::OnDatabase(bool bOk)
{
    boost::recursive_mutex::scoped_lock lock(mutex_);
    emit S_OnDatabase(bOk);
}

void QNotifyHandler::OnClientOffline(string sIp,short port)
{
    boost::recursive_mutex::scoped_lock lock(mutex_);
    //QString sEndpoint = QString(tr("%1:%2")).arg(sIp.c_str()).arg(port);
    emit S_OnClientOffline(QString::fromStdString(sIp));
}

void QNotifyHandler::OnClientLogin(string sIp,short port,string sName,string sID)
{
     boost::recursive_mutex::scoped_lock lock(mutex_);
    //QString sEndpoint = QString(tr("%1:%2")).arg(sIp.c_str()).arg(port);
    emit S_OnClientLogin(QString::fromStdString(sIp),sName.c_str(),sID.c_str());
}

void QNotifyHandler::OnClientLogout(string sIp,short port)
{
    boost::recursive_mutex::scoped_lock lock(mutex_);
    //QString sEndpoint = QString(tr("%1:%2")).arg(sIp.c_str()).arg(port);
    emit S_OnClientLogout(QString::fromStdString(sIp));
}
