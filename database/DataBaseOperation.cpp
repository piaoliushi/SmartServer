#include "DataBaseOperation.h"
#include <sstream>
#include <QSqlError>
#include <QDateTime>
#include <QVariant>
DataBaseOperation::DataBaseOperation()
{
	dsn="";
}

DataBaseOperation::~DataBaseOperation()
{

}

bool DataBaseOperation::OpenDb( const std::string& serveraddress, const std::string& database, const std::string& uid, const std::string& pwd, int timeout, std::string link_driver,std::string driverName/*="SQL Native Client"*/ )
{

    //QString dsn = QString("SERVER=%1;DATABASE=%2;UID=%3;PWD=%4;").arg(QString::fromStdString(serveraddress))
    //	                                                                     .arg(QString::fromStdString(database)).arg(QString::fromStdString(uid)).arg(QString::fromStdString(pwd));
    q_db = QSqlDatabase::addDatabase("QPSQL");
    q_db.setHostName(QString::fromStdString(serveraddress));//设置主机名
    q_db.setDatabaseName(QString::fromStdString(database));//设置数据库名
    q_db.setUserName(QString::fromStdString(uid));//设置用户名
    q_db.setPassword(QString::fromStdString(pwd));//设置用户密码

	if(!q_db.open())
	{
		return false;
	}
	return true;
}

bool DataBaseOperation::CloseDb()
{
	if(q_db.isOpen())
	{
		q_db.close();
	}
	return true;
}

bool DataBaseOperation::IsOpen()
{
	return q_db.isOpen();
}

bool DataBaseOperation::ReOpen()
{
	if(q_db.isOpen())
	{
		q_db.close();
	}
    //q_db.setDatabaseName(dsn);
	if(!q_db.open())
	{
		return false;
	}
	return true;
}

bool DataBaseOperation::GetAllDevInfo( vector<ModleInfo>& v_Linkinfo )
{
    if(!IsOpen())
    {
        std::cout<<"数据库未打开"<<std::endl;
        return false;
    }
    QSqlQuery netquery;
    QString strSql=QString("select NetType,IpAddress,LocalPort,PeerPort,ConnectType,CommTypeNumber from Net_Communication_Mode");
    netquery.prepare(strSql);
    if(netquery.exec())
    {
        while(netquery.next())
        {
            ModeInfo info;
            info.iCommunicationMode = 1;
            info.netmode.inet_type = netquery.value(0).toInt();
            info.netmode.strIp = netquery.value(1).toString().toStdString();
            info.netmode.ilocal_port = netquery.value(2).toInt();
            info.netmode.iremote_port = netquery.value(3).toInt();
            info.netmode.ilink_type = netquery.value(4).toInt();
            QString qtrNum = netquery.value(5).toString();
            info.sModleNumber = qtrNum.toStdString();
            QString strQdev = QString("select DeviceNumber from Device_Bind_Comm where CommTypeNumber=%1").arg(qtrNum);
            QSqlQuery net1query;
            if(net1query.exec(strQdev))
            {
                while(net1query.next())
                {
                    Device dev;
                    GetDevInfo(net1query.value(0).toString().toStdString(),dev);
                    info.map_DevInfo[net1query.value(0).toString().toStdString()]=dev;
                }
            }
            v_Linkinfo.push_back(info);
        }
    }
    QSqlQuery comquery;
    strSql=QString("select Com,Baudrate,Databit,Stopbit,Parity,CommTypeNumber from Com_Communication_Mode");
    comquery.prepare(strSql);
    if(comquery.exec())
    {
        while(comquery.next())
        {
            ModeInfo info;
            info.iCommunicationMode = 0;
            info.commode.icomport = comquery.value(0).toInt();
            info.commode.irate = comquery.value(1).toInt();
            info.commode.idata_bit = comquery.value(2).toInt();
            info.commode.istop_bit = comquery.value(3).toInt();
            info.commode.iparity_bit = comquery.value(4).toInt();

            QString qtrNum = netquery.value(5).toString();
            QString strQdev = QString("select DeviceNumber from Device_Bind_Comm where CommTypeNumber=%1").arg(qtrNum);
            QSqlQuery net1query;
            if(net1query.exec(strQdev))
            {
                while(net1query.next())
                {
                    Device dev;
                    GetDevInfo(net1query.value(0).toString().toStdString(),dev);
                    info.map_DevInfo[net1query.value(0).toString().toStdString()]=dev;
                }
            }
            v_Linkinfo.push_back(info);
        }
    }

    return true;
}

bool DataBaseOperation::GetAlarmConfig( string strDevnum,map<int,Alarm_config>& map_Alarmconfig )
{
    if(!IsOpen())
    {
        std::cout<<"数据库未打开"<<std::endl;
        return false;
    }
    QSqlQuery alarmconfigquery;
    QString strSql=QString("select a.MonitoringIndex,a.LimitValue,a.AlarmLevel,a.JumpLimitType,a.LinkageEnable,a.LinkageRoleNumber,a.delaytime,a.LinkageRoleNumber,a.alarmconfigtype \
                           from Alarm_Item_config a,Monitoring_Device_Item b\
                           where b.DeviceNumber=%1 and a.DeviceNumber=b.DeviceNumber and a.MonitoringIndex=b.MonitoringIndex and b.AlarmEnable>0").arg(QString::fromStdString(strDevnum));
    alarmconfigquery.prepare(strSql);
    if(alarmconfigquery.exec())
    {
        while(alarmconfigquery.next())
        {
            Alarm_config acfig;
            acfig.iItemid = alarmconfigquery.value(0).toInt();
            acfig.fLimitvalue = alarmconfigquery.value(1).toDouble();
            acfig.iAlarmlevel = alarmconfigquery.value(2).toInt();
            acfig.iLimittype = alarmconfigquery.value(3).toInt();
            acfig.iLinkageEnable = alarmconfigquery.value(4).toInt();
            if(acfig.iLinkageEnable>0)
            {
                GetLinkAction(alarmconfigquery.value(5).toString().toStdString(),acfig.vLinkAction);
            }
            acfig.iDelaytime = alarmconfigquery.value(6).toInt();
            acfig.strLinkageRoleNumber = alarmconfigquery.value(7).toString().toStdString();
            acfig.iAlarmtype = alarmconfigquery.value(8).toInt();
            map_Alarmconfig[acfig.iItemid] = acfig;
        }
    }
    else
    {
        std::cout<<alarmconfigquery.lastError().text().toStdString()<<std::endl;
        return false;
    }
    return true;
}


bool DataBaseOperation::GetDevInfo( string strDevnum,DeviceInfo& device )
{
    if(!IsOpen())
    {
        std::cout<<"数据库未打开"<<std::endl;
        return false;
    }
    QSqlQuery devquery;
    QString strSql=QString("select a.DeviceNumber,a.AssociateNumber,a.DeviceName,a.DeviceType,a.IsAssociate,a.IsMultiChannel,a.ChannelSize,a.IsUse,a.AddressCode,b.MainCategoryNumber,b.SubCategoryNumber,\
                           from Device a,Device_Map_Protocol b where a.DeviceNumber=%1 and b.ProtocolNumber=a.ProtocolNumber").arg(QString::fromStdString(strDevnum));
    devquery.prepare(strSql);
    if(devquery.exec())
    {
        if(devquery.next())//待修改
        {
            device.sDevNum = devquery.value(0).toString().toStdString();
            device.sAstNum = devquery.value(1).toString().toStdString();
            device.sDevName = devquery.value(2).toString().toStdString();
            device.iDevType = devquery.value(3).toInt();
            device.bAst = devquery.value(4).toBool();
            device.bMulChannel = devquery.value(5).toBool();
            device.iChanSize = devquery.value(6).toInt();
            device.bUsed = devquery.value(7).toBool();
            device.iAddressCode = devquery.value(8).toInt();
            device.iMcNum = devquery.value(9).toInt();
            device.iScNum = devquery.value(10).toInt();

            GetDevMonItem(strDevnum,device.map_MonitorItem);
            GetDevMonitorSch(strDevnum,device.map_MonitorSch);
            GetCmd(strDevnum,device.map_CommSch);
            GetDevProperty(strDevnum,device.map_DevProperty);
            GetAlarmConfig(strDevnum,device.map_AlarmConfig);

        }
    }
    else
    {
        std::cout<<devquery.lastError().text().toStdString()<<std::endl;
        return false;
    }
    return true;
}

bool DataBaseOperation::GetDevMonitorSch( string strDevnum,map<int,vector<Monitoring_Scheduler> >& map_MonitorSch )
{
	if(!IsOpen())
	{
		std::cout<<"数据库未打开"<<std::endl;
		return false;
	}
	QSqlQuery schquery;
	QString strSql=QString("select Guid,ObjectNumber,WeekDay,Enable,StartTime,EndTime,datetype from Monitoring_Scheduler where ObjectNumber=%1 and Enable=1").arg(QString::fromStdString(strDevnum));
	schquery.prepare(strSql);
	if(schquery.exec())
	{
		while(schquery.next())
		{
			Monitoring_Scheduler msch;
			msch.gid = schquery.value(0).toInt();//"Guid"
			msch.iWeek = schquery.value(2).toInt();
			QDateTime stime = schquery.value(4).toDateTime();
			msch.tStartTime.tm_year = stime.date().year();
			msch.tStartTime.tm_mon = stime.date().month();
			msch.tStartTime.tm_mday = stime.date().day();
			msch.tStartTime.tm_hour = stime.time().hour();
			msch.tStartTime.tm_min = stime.time().minute();
			msch.tStartTime.tm_sec = stime.time().second();

			QDateTime etime = schquery.value(5).toDateTime();
			msch.tEndTime.tm_year = etime.date().year();
			msch.tEndTime.tm_mon = etime.date().month();
			msch.tEndTime.tm_mday = etime.date().day();
			msch.tEndTime.tm_hour = etime.time().hour();
			msch.tEndTime.tm_min = etime.time().minute();
			msch.tEndTime.tm_sec = etime.time().second();
			msch.iDatetype = schquery.value(6).toInt();
			map_MonitorSch[msch.iWeek].push_back(msch);
		}
	}
	else
	{
		std::cout<<schquery.lastError().text().toStdString()<<std::endl;
		return false;
	}
	return true;
}

bool DataBaseOperation::GetCmdParam( string strCmdnum,CmdParam& param )
{
	if(!IsOpen())
	{
		std::cout<<"数据库未打开"<<std::endl;
		return false;
	}
	QSqlQuery cmdparquery;
	QString strSql=QString("select Param0,HasParam1,Param1 from Param_Define where ParamNumber=%1").arg(QString::fromStdString(strCmdnum));
	cmdparquery.prepare(strSql);
	if(cmdparquery.exec())
	{
		if(cmdparquery.next())
		{
			param.sParam1 = cmdparquery.value(0).toString().toStdString();
			param.bUseP2 = cmdparquery.value(1).toBool();
			param.sParam2 = cmdparquery.value(2).toString().toStdString();
		}
	}
	else
	{
		std::cout<<cmdparquery.lastError().text().toStdString()<<std::endl;
		return false;
	}
	return true;
}

bool DataBaseOperation::GetCmd( string strDevnum,map<int,vector<Command_Scheduler> >& map_cmd_sch )
{
	if(!IsOpen())
	{
		std::cout<<"数据库未打开"<<std::endl;
		return false;
	}
	QSqlQuery cmdschquery;
	QString strSql=QString("select Guid,CommandType,WeekDay,StartTime,HasParam,ParamNumber from Command_Scheduler where ObjectNumber=%1 and Enable=1").arg(QString::fromStdString(strDevnum));
	cmdschquery.prepare(strSql);
	if(cmdschquery.exec())
	{
		while(cmdschquery.next())
		{
			Command_Scheduler cmd_sch;
			cmd_sch.gid = cmdschquery.value(0).toInt();
			cmd_sch.iCommandType = cmdschquery.value(1).toInt();
			cmd_sch.iWeek = cmdschquery.value(2).toInt();
			QDateTime sttime = cmdschquery.value(3).toDateTime();
			cmd_sch.tExecuteTime.tm_year = sttime.date().year();
			cmd_sch.tExecuteTime.tm_mon = sttime.date().month();
			cmd_sch.tExecuteTime.tm_mday = sttime.date().day();
			cmd_sch.tExecuteTime.tm_hour = sttime.time().hour();
			cmd_sch.tExecuteTime.tm_min = sttime.time().minute();
			cmd_sch.tExecuteTime.tm_sec = sttime.time().second();
			cmd_sch.iHasParam = cmdschquery.value(4).toInt();
			if(cmd_sch.iHasParam>=1)
			{
				string sparnum = cmdschquery.value(5).toString().toStdString();
				GetCmdParam(sparnum,cmd_sch.cParam);
			}
			map_cmd_sch[cmd_sch.iWeek].push_back(cmd_sch);
		}
	}
	else
	{
		std::cout<<cmdschquery.lastError().text().toStdString()<<std::endl;
		return false;
	}
	return true;
}

bool DataBaseOperation::GetDevMonItem( string strDevnum,map<int,DeviceMonitorItem>& map_item )
{
	if(!IsOpen())
	{
		std::cout<<"数据库未打开"<<std::endl;
		return false;
	}
	QSqlQuery itemschquery;
	QString strSql=QString("select MonitoringIndex,MonitoringName,Ratio,ItemType,ItemValueType,AlarmEnable,IsUpload,UnitString from Monitoring_Device_Item \
						   where DeviceNumber=%1").arg(QString::fromStdString(strDevnum));
	itemschquery.prepare(strSql);
	if(itemschquery.exec())
	{
		while(itemschquery.next())
		{
			DeviceMonitorItem item;
			item.iItemIndex = itemschquery.value(0).toInt();
			item.sItemName = itemschquery.value(1).toString().toStdString();
			item.dRatio = itemschquery.value(2).toDouble();
			item.iItemType = itemschquery.value(3).toInt();
			item.iItemvalueType = itemschquery.value(4).toInt();
			item.bAlarmEnable = itemschquery.value(5).toBool();
			item.bUpload = itemschquery.value(6).toBool();
			item.sUnit = itemschquery.value(7).toString().toStdString();
			map_item[item.iItemIndex] = item;
		}
	}
	else
	{
		std::cout<<itemschquery.lastError().text().toStdString()<<std::endl;
		return false;
	}
	return true;
}

bool DataBaseOperation::GetDevProperty( string strDevnum,map<string,DevProperty>& map_property )
{
	if(!IsOpen())
	{
		std::cout<<"数据库未打开"<<std::endl;
		return false;
	}
	QSqlQuery itemschquery;
	QString strSql=QString("select a.BasePropertyNumber,a.PropertyValueType,a.PropertyValue,b.PropertylName from Device_Property_Role_Bind a,Base_Propertyb \
						   where a.DeviceNumber=%1 and b.BasePropertyNumber=a.BasePropertyNumber").arg(QString::fromStdString(strDevnum));
	itemschquery.prepare(strSql);
	if(itemschquery.exec())
	{
		while(itemschquery.next())
		{
			DevProperty dp;
			dp.property_num = itemschquery.value(0).toString().toStdString();
			dp.property_type = itemschquery.value(1).toInt();
			dp.property_value = itemschquery.value(2).toString().toStdString();
			dp.property_name = itemschquery.value(3).toString().toStdString();
			map_property[dp.property_num] = dp;
		}
	}
	else
	{
		std::cout<<itemschquery.lastError().text().toStdString()<<std::endl;
		return false;
	}
	return true;
}

bool DataBaseOperation::GetNetProperty( string strConTypeNumber,NetCommunicationMode& nmode )
{
	if(!IsOpen())
	{
		std::cout<<"数据库未打开"<<std::endl;
		return false;
	}
	QSqlQuery netquery;
	QString strSql=QString("select NetType,IpAddress,LocalPort,PeerPort,ConnectType from Net_Communication_Mode \
						   where CommTypeNumber=%1").arg(QString::fromStdString(strConTypeNumber));
	netquery.prepare(strSql);
	if(netquery.exec())
	{
		if(netquery.next())
		{
			nmode.inet_type = netquery.value(0).toInt();
			nmode.strIp = netquery.value(1).toString().toStdString();
			nmode.ilocal_port = netquery.value(2).toInt();
			nmode.iremote_port = netquery.value(3).toInt();
			nmode.ilink_type = netquery.value(4).toInt();
		}
		else
		{
			return false;
		}
	}
	else
	{
		std::cout<<netquery.lastError().text().toStdString()<<std::endl;
		return false;
	}
	return true;
}

bool DataBaseOperation::GetComProperty( string strConTypeNumber,ComCommunicationMode& cmode )
{
	if(!IsOpen())
	{
		std::cout<<"数据库未打开"<<std::endl;
		return false;
	}
	QSqlQuery comquery;
	QString strSql=QString("select Com,Baudrate,Databit,Stopbit,Parity from Com_Communication_Mode \
						   where CommTypeNumber=%1").arg(QString::fromStdString(strConTypeNumber));
	comquery.prepare(strSql);
	if(comquery.exec())
	{
		if(comquery.next())
		{
			cmode.icomport = comquery.value(0).toInt();
			cmode.irate = comquery.value(1).toInt();
			cmode.idata_bit = comquery.value(2).toInt();
			cmode.istop_bit = comquery.value(3).toInt();
			cmode.iparity_bit = comquery.value(4).toInt();
		}
		else
		{
			return false;
		}
	}
	else
	{
		std::cout<<comquery.lastError().text().toStdString()<<std::endl;
		return false;
	}
	return true;
}


bool DataBaseOperation::GetLinkActionParam( string strParamnum,map<int,ActionParam>& map_Params )
{
	if(!IsOpen())
	{
		std::cout<<"数据库未打开"<<std::endl;
		return false;
	}
	QSqlQuery actpquery;
	QString strSql=QString("select a.parameterindex,b.ParameterContent,b.ActionParameterType from Action_Parameter_Bind_Content a,Action_Parameter_Content b \
						   where a.ParameterNumber=%1 and b.ActionParameterContentNumber=a.ActionParameterContentNumber order by a.parameterindex"
						   ).arg(QString::fromStdString(strParamnum));
	actpquery.prepare(strSql);
	if(actpquery.exec())
	{
		while(actpquery.next())
		{
			ActionParam aparam;
			aparam.strParamValue = actpquery.value(1).toString().toStdString();
			aparam.iParamType = actpquery.value(2).toInt();
			map_Params[actpquery.value(0).toInt()] = aparam;
		}
	}
	else
	{
		std::cout<<actpquery.lastError().text().toStdString()<<std::endl;
		return false;
	}
	return true;
}
bool DataBaseOperation::GetLinkAction( string strLinkRolenum,vector<LinkAction>& vLinkAction )
{
	if(!IsOpen())
	{
		std::cout<<"数据库未打开"<<std::endl;
		return false;
	}
	QSqlQuery actquery;
	QString strSql=QString("select b.ActionNumber,b.ActionName,b.ActionType,b.IsParam,b.ParameterNumber from Linkage_Role_Bind_Action a,Action b \
						   where a.LinkageRoleNumber=%1 and b.ActionNumber=a.ActionNumber").arg(QString::fromStdString(strLinkRolenum));
	actquery.prepare(strSql);
	if(actquery.exec())
	{
		while(actquery.next())
		{
			LinkAction laction;
			laction.strActionNum = actquery.value(0).toString().toStdString();
			laction.strActionNam = actquery.value(1).toString().toStdString();
			laction.iActionType = actquery.value(2).toInt();
			laction.iIshaveParam = actquery.value(3).toInt();
			if(!GetLinkActionParam(actquery.value(4).toString().toStdString(),laction.map_Params))
				laction.iIshaveParam = 0;
			vLinkAction.push_back(laction);
		}
	}
	else
	{
		std::cout<<actquery.lastError().text().toStdString()<<std::endl;
		return false;
	}
	return true;
}

bool DataBaseOperation::SetEnableMonitor( string strDevnum,int iItemIndex,bool bEnabled/*=true*/ )
{
	if(!IsOpen())
	{
		std::cout<<"数据库未打开"<<std::endl;
		return false;
	}
	QSqlQuery qquery;
	QString strSql=QString("update Monitoring_Device_Item set AlarmEnable=%1 where DeviceNumber=%2 and MonitoringIndex=%3").arg(bEnabled).arg(QString::fromStdString(strDevnum)).arg(iItemIndex);
	qquery.prepare(strSql);
	if(!qquery.exec())
	{
		std::cout<<qquery.lastError().text().toStdString()<<std::endl;
		return false;
	}
	return true;
}

bool DataBaseOperation::UpdateMonitorItem( string strDevnum,DeviceMonitorItem ditem )
{
	if(!IsOpen())
	{
		std::cout<<"数据库未打开"<<std::endl;
		return false;
	}
	QSqlQuery qquery;
	QString strSql=QString("update Monitoring_Device_Item set MonitoringName=:MonitoringName,Ratio=:Ratio,ItemType=:ItemType,ItemValueType=:ItemValueType,\
						   AlarmEnable=:AlarmEnable,IsUpload=:IsUpload,UnitString:UnitString where DeviceNumber=:DeviceNumber and MonitoringIndex=:MonitoringIndex");
	qquery.prepare(strSql);
	qquery.bindValue(":MonitoringName",QString::fromStdString(ditem.sItemName));
	qquery.bindValue(":Ratio",ditem.dRatio);
	qquery.bindValue(":ItemType",ditem.iItemType);
	qquery.bindValue(":ItemValueType",ditem.iItemvalueType);
	qquery.bindValue(":AlarmEnable",ditem.bAlarmEnable);
	qquery.bindValue(":IsUpload",ditem.bUpload);
	qquery.bindValue(":UnitString",QString::fromStdString(ditem.sUnit));
	qquery.bindValue(":DeviceNumber",QString::fromStdString(strDevnum));
	qquery.bindValue(":MonitoringIndex",ditem.iItemIndex);
	if(!qquery.exec())
	{
		std::cout<<qquery.lastError().text().toStdString()<<std::endl;
		return false;
	}
	return true;
}

bool DataBaseOperation::UpdateMonitorItems( string strDevnum,vector<DeviceMonitorItem> v_ditem )
{
	if(!IsOpen())
	{
		std::cout<<"数据库未打开"<<std::endl;
		return false;
	}
	
/*	for(;iter!=v_ditem.end();++iter)
	{
		UpdateMonitorItem(strDevnum,(*iter));
	}*/


	QSqlQuery qquery;
	QString strSql=QString("update Monitoring_Device_Item set MonitoringName=:MonitoringName,Ratio=:Ratio,ItemType=:ItemType,ItemValueType=:ItemValueType,\
						   AlarmEnable=:AlarmEnable,IsUpload=:IsUpload,UnitString:UnitString where DeviceNumber=:DeviceNumber and MonitoringIndex=:MonitoringIndex");
	qquery.prepare(strSql);

	vector<DeviceMonitorItem>::iterator iter = v_ditem.begin();
	QSqlDatabase::database().transaction();
	for(;iter!=v_ditem.end();++iter)
	{
		qquery.bindValue(":MonitoringName",QString::fromStdString((*iter).sItemName));
		qquery.bindValue(":Ratio",(*iter).dRatio);
		qquery.bindValue(":ItemType",(*iter).iItemType);
		qquery.bindValue(":ItemValueType",(*iter).iItemvalueType);
		qquery.bindValue(":AlarmEnable",(*iter).bAlarmEnable);
		qquery.bindValue(":IsUpload",(*iter).bUpload);
		qquery.bindValue(":UnitString",QString::fromStdString((*iter).sUnit));
		qquery.bindValue(":DeviceNumber",QString::fromStdString(strDevnum));
		qquery.bindValue(":MonitoringIndex",(*iter).iItemIndex);
		if(!qquery.exec())
		{
			std::cout<<qquery.lastError().text().toStdString()<<std::endl;
			QSqlDatabase::database().rollback();
			return false;
		}
	}
	
	QSqlDatabase::database().commit();
	return true;
}

bool DataBaseOperation::UpdateItemAlarmConfig( string strDevnum,Alarm_config alarm_config )
{
	if(!IsOpen())
	{
		std::cout<<"数据库未打开"<<std::endl;
		return false;
	}
	QSqlQuery qquery;
	QString strSql=QString("update Monitoring_Device_Item set LimitValue=:LimitValue,AlarmLevel=:AlarmLevel,JumpLimitType=:JumpLimitType,LinkageEnable=:LinkageEnable,\
						   LinkageRoleNumber=:LinkageRoleNumber where DeviceNumber=:DeviceNumber and MonitoringIndex=:MonitoringIndex");
	qquery.prepare(strSql);
	qquery.bindValue(":LimitValue",alarm_config.fLimitvalue);
	qquery.bindValue(":AlarmLevel",alarm_config.iAlarmlevel);
	qquery.bindValue(":JumpLimitType",alarm_config.iLimittype);
	qquery.bindValue(":LinkageEnable",alarm_config.iLinkageEnable);
	qquery.bindValue(":LinkageRoleNumber",QString::fromStdString(alarm_config.strLinkageRoleNumber));
	qquery.bindValue(":DeviceNumber",QString::fromStdString(strDevnum));
	qquery.bindValue(":MonitoringIndex",alarm_config.iItemid);
	if(!qquery.exec())
	{
		std::cout<<qquery.lastError().text().toStdString()<<std::endl;
		return false;
	}
	return true;
}

bool DataBaseOperation::UpdateItemAlarmConfigs( string strDevnum,vector<Alarm_config> v_alarm_config )
{
	if(!IsOpen())
	{
		std::cout<<"数据库未打开"<<std::endl;
		return false;
	}
	QSqlQuery qquery;
	QString strSql=QString("update Monitoring_Device_Item set LimitValue=:LimitValue,AlarmLevel=:AlarmLevel,JumpLimitType=:JumpLimitType,LinkageEnable=:LinkageEnable,\
						   LinkageRoleNumber=:LinkageRoleNumber where DeviceNumber=:DeviceNumber and MonitoringIndex=:MonitoringIndex");
	qquery.prepare(strSql);
	vector<Alarm_config>::iterator iter = v_alarm_config.begin();
	QSqlDatabase::database().transaction();
	for(;iter!=v_alarm_config.end();++iter)
	{
		qquery.bindValue(":LimitValue",(*iter).fLimitvalue);
		qquery.bindValue(":AlarmLevel",(*iter).iAlarmlevel);
		qquery.bindValue(":JumpLimitType",(*iter).iLimittype);
		qquery.bindValue(":LinkageEnable",(*iter).iLinkageEnable);
		qquery.bindValue(":LinkageRoleNumber",QString::fromStdString((*iter).strLinkageRoleNumber));
		qquery.bindValue(":DeviceNumber",QString::fromStdString(strDevnum));
		qquery.bindValue(":MonitoringIndex",(*iter).iItemid);
		if(!qquery.exec())
		{
			std::cout<<qquery.lastError().text().toStdString()<<std::endl;
			QSqlDatabase::database().rollback();
			return false;
		}
	}
	QSqlDatabase::database().commit();
	return true;
}
