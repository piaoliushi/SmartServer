#include "DataBaseOperation.h"
#include <sstream>
#include <QSqlError>
#include <QDateTime>
#include <QVariant>
#include "../rapidxml/rapidxml_print.hpp"
#include "../rapidxml/rapidxml_utils.hpp"
#include "../protocol/bohui_const_define.h"

#define BOHUI_ALARMID_BASE    4000

namespace db {

DataBaseOperation::DataBaseOperation()
{
    dsn="";
}

DataBaseOperation::~DataBaseOperation()
{

}

bool DataBaseOperation::OpenDb( const std::string& serveraddress, const std::string& database, const std::string& uid, const std::string& pwd, int timeout, std::string link_driver,std::string driverName/*="SQL Native Client"*/ )
{
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
    if(dsn.isEmpty())
    {
        return false;
    }
    q_db.setDatabaseName(dsn);
    if(!q_db.open())
    {
        return false;
    }
    return true;
}

//获得数据字典映射表
bool DataBaseOperation::GetDataDictionary(map<int,pair<string,string> >& mapDicry)
{
    if(!IsOpen())
    {
        std::cout<<"数据库未打开"<<std::endl;
        return false;
    }
    QSqlQuery query;
    QString strSql=QString("select code,name,remark from data_dictionary where type='IndexType' union select alarmswitch,alarmswitchname,remark from alarm_switch");
    query.prepare(strSql);
    if(!query.exec())
    {
        return false;
    }
    while(query.next())
    {
        mapDicry[query.value(0).toInt()] = pair<string,string>(query.value(1).toString().toStdString(),query.value(2).toString().toStdString());
        cout<<query.value(1).toString().toStdString()<<endl;
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
    QString strSql=QString("select a.DeviceNumber,a.AssociateNumber,a.DeviceName,a.DeviceType,a.IsAssociate,a.IsMultiChannel,a.ChannelSize,a.IsUse,a.AddressCode,b.MainCategoryNumber,b.SubCategoryNumber,a.ProtocolNumber\
                           from Device a,Device_Map_Protocol b where a.DeviceNumber='%1' and b.ProtocolNumber=a.ProtocolNumber").arg(QString::fromStdString(strDevnum));
            devquery.prepare(strSql);
            if(devquery.exec())
    {
            if(devquery.next())  {
            device.sDevNum = devquery.value(0).toString().toStdString();
            //   QString sAsos = devquery.value(1).toString();
            device.sDevName = devquery.value(2).toString().toStdString();
            device.iDevType = devquery.value(3).toInt();
            device.bAst = devquery.value(4).toBool();
            device.bMulChannel = devquery.value(5).toBool();
            device.iChanSize = devquery.value(6).toInt();
            device.bUsed = devquery.value(7).toBool();
            device.iAddressCode = devquery.value(8).toInt();
            device.nDevProtocol = devquery.value(9).toInt();
            device.nSubProtocol = devquery.value(10).toInt();
            QString sprotoclnum = devquery.value(11).toString();
            GetDevMonItem(strDevnum,sprotoclnum,device.map_MonitorItem);
            GetDevMonitorSch(strDevnum,device.vMonitorSch);
            GetCmd(strDevnum,device.vCommSch);
            GetDevProperty(strDevnum,device.map_DevProperty);
            GetAlarmConfig(strDevnum,device.map_AlarmConfig);
            GetAssDevChan(QString::fromStdString(strDevnum),device.map_AssDevChan);

}
}
            else
    {
            std::cout<<devquery.lastError().text().toStdString()<<std::endl;
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
            ModleInfo info;
            info.iCommunicationMode = 1;
            info.netMode.inet_type = netquery.value(0).toInt();
            info.netMode.strIp = netquery.value(1).toString().toStdString();
            info.netMode.ilocal_port = netquery.value(2).toInt();
            info.netMode.iremote_port = netquery.value(3).toInt();
            info.netMode.ilink_type = netquery.value(4).toInt();

            QString qtrNum = netquery.value(5).toString();
            info.sModleNumber = qtrNum.toStdString();
            QString strQdev = QString("select DeviceNumber from Device_Bind_Comm where CommTypeNumber='%1'").arg(qtrNum);
            QSqlQuery net1query;
            if(net1query.exec(strQdev))
            {

                while(net1query.next())
                {
                    DeviceInfo dev;
                    GetDevInfo(net1query.value(0).toString().toStdString(),dev);
                    info.mapDevInfo[net1query.value(0).toString().toStdString()]=dev;
                }
            }

            if(net1query.size()>0 && info.mapDevInfo.size()>0)
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
            ModleInfo info;
            info.iCommunicationMode = 0;
            info.comMode.icomport = comquery.value(0).toInt();
            info.comMode.irate = comquery.value(1).toInt();
            info.comMode.idata_bit = comquery.value(2).toInt();
            info.comMode.istop_bit = comquery.value(3).toInt();
            info.comMode.iparity_bit = comquery.value(4).toInt();

            QString qtrNum = comquery.value(5).toString();
            QString strQdev = QString("select DeviceNumber from Device_Bind_Comm where CommTypeNumber='%1'").arg(qtrNum);
            QSqlQuery net1query;
            if(net1query.exec(strQdev))
            {
                while(net1query.next())
                {
                    DeviceInfo dev;
                    GetDevInfo(net1query.value(0).toString().toStdString(),dev);
                    info.mapDevInfo[net1query.value(0).toString().toStdString()]=dev;
                }
            }

            if(net1query.size()>0)
                v_Linkinfo.push_back(info);
        }
    }

    return true;
}
bool DataBaseOperation::GetDevMonitorSch( string strDevnum,map<int,vector<Monitoring_Scheduler> >& mapMonitorSch )
{
    if(!IsOpen())
    {
        std::cout<<"数据库未打开"<<std::endl;
        return false;
    }
    QSqlQuery schquery;
    QString strSql=QString("select id,ObjectNumber,WeekDay,Enable,StartTime,EndTime,datetype,month,day, \
                           alarmendtime from Monitoring_Scheduler where ObjectNumber='%1'").arg(QString::fromStdString(strDevnum));
    schquery.prepare(strSql);
    if(schquery.exec())
    {
        while(schquery.next())
        {
            Monitoring_Scheduler msch;
            //	msch.gid = schquery.value(0).toInt();//"Guid"
            msch.iMonitorWeek = schquery.value(2).toInt();
            msch.bMonitorFlag = schquery.value(3).toBool();
            msch.tStartTime = schquery.value(4).toDateTime().toTime_t();
            msch.tEndTime = schquery.value(5).toDateTime().toTime_t();
            int iMonitorType = schquery.value(6).toInt();
            msch.iMonitorMonth = schquery.value(7).toInt();
            msch.iMonitorDay = schquery.value(8).toInt();
            msch.tAlarmEndTime = schquery.value(9).toDateTime().toTime_t();

            mapMonitorSch[iMonitorType].push_back(msch);
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
    QString strSql=QString("select Param0,HasParam1,Param1 from Param_Define where ParamNumber='%1'").arg(QString::fromStdString(strCmdnum));
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

bool DataBaseOperation::GetCmd( string strDevnum,vector<Command_Scheduler>& vcmdsch )
{
    if(!IsOpen())
    {
        std::cout<<"数据库未打开"<<std::endl;
        return false;
    }
    QSqlQuery cmdschquery;
    QString strSql=QString("select id,CommandType,WeekDay,StartTime,HasParam,ParamNumber,month,day,commandendtime,datetype from Command_Scheduler where ObjectNumber='%1' and Enable=1").arg(QString::fromStdString(strDevnum));
    cmdschquery.prepare(strSql);
    if(cmdschquery.exec())
    {
        while(cmdschquery.next())
        {
            Command_Scheduler cmd_sch;
            cmd_sch.gid = cmdschquery.value(0).toInt();
            cmd_sch.iCommandType = cmdschquery.value(1).toInt();
            cmd_sch.iWeek = cmdschquery.value(2).toInt();
            cmd_sch.tExecuteTime = cmdschquery.value(3).toDateTime().toTime_t();
            cmd_sch.iHasParam = cmdschquery.value(4).toInt();
            if(cmd_sch.iHasParam>=1)
            {
                string sparnum = cmdschquery.value(5).toString().toStdString();
                GetCmdParam(sparnum,cmd_sch.cParam);
            }
            cmd_sch.iMonitorMonth = cmdschquery.value(6).toInt();
            cmd_sch.iMonitorDay = cmdschquery.value(7).toInt();
            cmd_sch.tCmdEndTime = cmdschquery.value(8).toDateTime().toTime_t();
            cmd_sch.iDateType = cmdschquery.value(9).toInt();
            vcmdsch.push_back(cmd_sch);
        }
    }
    else
    {
        std::cout<<cmdschquery.lastError().text().toStdString()<<std::endl;
        return false;
    }
    return true;
}

bool DataBaseOperation::GetDevMonItem( string strDevnum,QString qsPrtocolNum,map<int,DeviceMonitorItem>& map_item )
{
    if(!IsOpen())
    {
        std::cout<<"数据库未打开"<<std::endl;
        return false;
    }
    QSqlQuery itemschquery;
    /*QString strSql=QString("select MonitoringIndex,MonitoringName,Ratio,ItemType,ItemValueType,AlarmEnable,IsUpload,UnitString from Monitoring_Device_Item \
                           where DeviceNumber='%1'").arg(QString::fromStdString(strDevnum));*/

    QString strSql=QString("select a.MonitoringIndex,a.MonitoringName,a.Ratio,a.ItemType,a.ItemValueType,a.AlarmEnable,a.IsUpload,a.UnitString,b.alarmtype,b.type,b.moduletype,b.moduleid \
                           from Monitoring_Device_Item a,base_device_item b\
                           where a.DeviceNumber='%1' and b.monitoringindex=a.monitoringindex and b.protocolnumber='%2'").arg(QString::fromStdString(strDevnum)).arg(qsPrtocolNum);
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
            int iAlarmid = itemschquery.value(8).toInt();
            GetItemAlarmConfig(strDevnum,iAlarmid,item.vItemAlarm);
            item.iTargetId = itemschquery.value(9).toInt();
            item.iModTypeId = itemschquery.value(10).toInt();
            item.iModDevId = itemschquery.value(11).toInt();
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
    QString strSql=QString("select a.BasePropertyNumber,a.PropertyValueType,a.PropertyValue,b.PropertyName from Device_Property_Role_Bind a,Base_Property b \
                           where a.DeviceNumber='%1' and b.BasePropertyNumber=a.BasePropertyNumber").arg(QString::fromStdString(strDevnum));
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
            map_property[dp.property_name] = dp;
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
                           where CommTypeNumber='%1'").arg(QString::fromStdString(strConTypeNumber));
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
                           where CommTypeNumber='%1'").arg(QString::fromStdString(strConTypeNumber));
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
                           where a.ParameterNumber='%1' and b.ActionParameterContentNumber=a.ActionParameterContentNumber order by a.parameterindex"
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
                           where a.LinkageRoleNumber='%1' and b.ActionNumber=a.ActionNumber").arg(QString::fromStdString(strLinkRolenum));
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

/*bool DataBaseOperation::GetAlarmConfig( string strDevnum,map<int,Alarm_config>& map_Alarmconfig )
{
    if(!IsOpen())
    {
        std::cout<<"数据库未打开"<<std::endl;
        return false;
    }
    QSqlQuery alarmconfigquery;
    QString strSql=QString("select a.MonitoringIndex,a.LimitValue,a.AlarmLevel,a.JumpLimitType,a.LinkageEnable,a.LinkageRoleNumber,a.delaytime,a.LinkageRoleNumber,a.alarmconfigtype \
                           from Alarm_Item_config a where a.DeviceNumber='%1' and a.alarmenable>0 and a.alarmconfigtype<>0").arg(QString::fromStdString(strDevnum));
            alarmconfigquery.prepare(strSql);
            if(alarmconfigquery.exec())
    {
            while(alarmconfigquery.next())
    {
            Alarm_config acfig;
            //	acfig.iItemid = alarmconfigquery.value(0).toInt();
            int iItemid = alarmconfigquery.value(0).toInt();
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
            acfig.iAlarmtype = alarmconfigquery.value(8).toInt();//0:监控量 1:整机
            acfig.iAlarmid = iItemid;
            map_Alarmconfig[iItemid] = acfig;
}
}
            else
    {
            std::cout<<alarmconfigquery.lastError().text().toStdString()<<std::endl;
            return false;
}
            return true;
}*/

bool DataBaseOperation::GetAssDevChan( QString strDevNum,map<int,vector<AssDevChan> >& mapAssDev )
{
    if(!IsOpen())
    {
        std::cout<<"数据库未打开"<<std::endl;
        return false;
    }

    QSqlQuery itemschquery;
    QString strSql=QString("select objectnumberb,channelnumberb,channelnumbera from associate_object  where objectnumbera='%1'").arg(strDevNum);
    itemschquery.prepare(strSql);
    if(itemschquery.exec())
    {
        while(itemschquery.next())
        {
            AssDevChan ac;
            ac.sAstNum = itemschquery.value(0).toString().toStdString();
            ac.iChannel = itemschquery.value(1).toInt();
            mapAssDev[itemschquery.value(2).toInt()].push_back(ac);
        }
    }
    else
    {
        std::cout<<itemschquery.lastError().text().toStdString()<<std::endl;
        return false;
    }
    strSql=QString("select objectnumbera,channelnumbera,channelnumberb from associate_object  where objectnumberb='%1'").arg(strDevNum);
    itemschquery.prepare(strSql);
    if(itemschquery.exec())
    {
        while(itemschquery.next())
        {
            AssDevChan ac;
            ac.sAstNum = itemschquery.value(0).toString().toStdString();
            ac.iChannel = itemschquery.value(1).toInt();
            mapAssDev[itemschquery.value(2).toInt()].push_back(ac);
        }
    }
    else
    {
        std::cout<<itemschquery.lastError().text().toStdString()<<std::endl;
        return false;
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
                           from Alarm_Item_config a,device_alarm_switch b where a.DeviceNumber='%1' and b.alarmenable>0 and a.alarmconfigtype<>0 and b.devicenumber=a.DeviceNumber and \
            b.alarmswitchtype=a.MonitoringIndex").arg(QString::fromStdString(strDevnum));
            alarmconfigquery.prepare(strSql);
            if(alarmconfigquery.exec()) {
            while(alarmconfigquery.next()){
            Alarm_config acfig;
            //	acfig.iItemid = alarmconfigquery.value(0).toInt();
            int iItemid = alarmconfigquery.value(0).toInt();
            acfig.fLimitvalue = alarmconfigquery.value(1).toDouble();
            acfig.iAlarmlevel = alarmconfigquery.value(2).toInt();
            acfig.iLimittype = alarmconfigquery.value(3).toInt();
            acfig.iLinkageEnable = alarmconfigquery.value(4).toInt();
            if(acfig.iLinkageEnable>0)
                GetLinkAction(alarmconfigquery.value(5).toString().toStdString(),acfig.vLinkAction);

            acfig.iDelaytime = alarmconfigquery.value(6).toInt();
            acfig.strLinkageRoleNumber = alarmconfigquery.value(7).toString().toStdString();
            acfig.iAlarmtype = alarmconfigquery.value(8).toInt();//0:监控量 1:整机
            acfig.iAlarmid = iItemid;
            map_Alarmconfig[iItemid] = acfig;
}
} else {
            std::cout<<alarmconfigquery.lastError().text().toStdString()<<std::endl;
            return false;
}
            return true;
}


bool DataBaseOperation::GetItemAlarmConfig( string strDevnum,int iIndex,vector<Alarm_config>& vAlarmconfig )
{
    if(!IsOpen())
    {
        std::cout<<"数据库未打开"<<std::endl;
        return false;
    }
    QSqlQuery alarmconfigquery;
    QString strSql=QString("select a.LimitValue,a.AlarmLevel,a.JumpLimitType,a.LinkageEnable,a.LinkageRoleNumber,a.delaytime,a.LinkageRoleNumber,a.alarmconfigtype \
                           from Alarm_Item_config a ,device_alarm_switch b where a.DeviceNumber='%1' and monitoringindex=%2 and a.alarmconfigtype=0 and b.alarmenable>0 \
            and b.devicenumber=a.DeviceNumber and b.alarmswitchtype=a.MonitoringIndex").arg(QString::fromStdString(strDevnum)).arg(iIndex);
            alarmconfigquery.prepare(strSql);
            if(alarmconfigquery.exec())
    {
            while(alarmconfigquery.next())
    {
            Alarm_config acfig;
            acfig.fLimitvalue = alarmconfigquery.value(0).toDouble();
            acfig.iAlarmlevel = alarmconfigquery.value(1).toInt();
            acfig.iLimittype = alarmconfigquery.value(2).toInt();
            acfig.iLinkageEnable = alarmconfigquery.value(3).toInt();
            if(acfig.iLinkageEnable>0)
    {
            GetLinkAction(alarmconfigquery.value(4).toString().toStdString(),acfig.vLinkAction);
}
            acfig.iDelaytime = alarmconfigquery.value(5).toInt();
            acfig.strLinkageRoleNumber = alarmconfigquery.value(6).toString().toStdString();
            acfig.iAlarmtype = alarmconfigquery.value(7).toInt();//0:监控量 1:整机
            acfig.iAlarmid = iIndex;
            vAlarmconfig.push_back(acfig);
}
}
            else
    {
            std::cout<<alarmconfigquery.lastError().text().toStdString()<<std::endl;
            return false;
}
            return true;
}

/*bool DataBaseOperation::GetItemAlarmConfig( string strDevnum,int iIndex,vector<Alarm_config>& vAlarmconfig )
{
    if(!IsOpen())
    {
        std::cout<<"数据库未打开"<<std::endl;
        return false;
    }
    QSqlQuery alarmconfigquery;
    QString strSql=QString("select a.LimitValue,a.AlarmLevel,a.JumpLimitType,a.LinkageEnable,a.LinkageRoleNumber,a.delaytime,a.LinkageRoleNumber,a.alarmconfigtype \
                           from Alarm_Item_config a where a.DeviceNumber='%1' and monitoringindex=%2 and a.alarmenable>0 and a.alarmconfigtype=0").arg(QString::fromStdString(strDevnum)).arg(iIndex);
            alarmconfigquery.prepare(strSql);
            if(alarmconfigquery.exec())
    {
            while(alarmconfigquery.next())
    {
            Alarm_config acfig;
            acfig.fLimitvalue = alarmconfigquery.value(0).toDouble();
            acfig.iAlarmlevel = alarmconfigquery.value(1).toInt();
            acfig.iLimittype = alarmconfigquery.value(2).toInt();
            acfig.iLinkageEnable = alarmconfigquery.value(3).toInt();
            if(acfig.iLinkageEnable>0)
    {
            GetLinkAction(alarmconfigquery.value(4).toString().toStdString(),acfig.vLinkAction);
}
            acfig.iDelaytime = alarmconfigquery.value(5).toInt();
            acfig.strLinkageRoleNumber = alarmconfigquery.value(6).toString().toStdString();
            acfig.iAlarmtype = alarmconfigquery.value(7).toInt();//0:监控量 1:整机
            acfig.iAlarmid = iIndex;
            vAlarmconfig.push_back(acfig);
}
}
            else
    {
            std::cout<<alarmconfigquery.lastError().text().toStdString()<<std::endl;
            return false;
}
            return true;
}*/

bool DataBaseOperation::SetEnableMonitor( string strDevnum,int iItemIndex,bool bEnabled/*=true*/ )
{
    if(!IsOpen())
    {
        std::cout<<"数据库未打开"<<std::endl;
        return false;
    }
    QSqlQuery qquery;
    QString strSql=QString("update alarm_item_config set alarmenable=%1 where DeviceNumber='%2' and MonitoringIndex=%3").arg(bEnabled).arg(QString::fromStdString(strDevnum)).arg(iItemIndex);
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

bool DataBaseOperation::UpdateItemAlarmConfig( string strDevnum,int iIndex,Alarm_config alarm_config )
{
    if(!IsOpen())
    {
        std::cout<<"数据库未打开"<<std::endl;
        return false;
    }
    QSqlQuery qquery;
    QString strSql=QString("update action_parameter_content set LimitValue=:LimitValue,AlarmLevel=:AlarmLevel,JumpLimitType=:JumpLimitType,LinkageEnable=:LinkageEnable,\
                           LinkageRoleNumber=:LinkageRoleNumber where DeviceNumber=:DeviceNumber and MonitoringIndex=:MonitoringIndex");
            qquery.prepare(strSql);
            qquery.bindValue(":LimitValue",alarm_config.fLimitvalue);
    qquery.bindValue(":AlarmLevel",alarm_config.iAlarmlevel);
    qquery.bindValue(":JumpLimitType",alarm_config.iLimittype);
    qquery.bindValue(":LinkageEnable",alarm_config.iLinkageEnable);
    qquery.bindValue(":LinkageRoleNumber",QString::fromStdString(alarm_config.strLinkageRoleNumber));
    qquery.bindValue(":DeviceNumber",QString::fromStdString(strDevnum));
    qquery.bindValue(":MonitoringIndex",iIndex);
    if(!qquery.exec())
    {
        std::cout<<qquery.lastError().text().toStdString()<<std::endl;
        return false;
    }
    return true;
}

bool DataBaseOperation::UpdateItemAlarmConfigs( string strDevnum,map<int,Alarm_config> mapAlarmConfig )
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
            map<int,Alarm_config>::iterator iter = mapAlarmConfig.begin();
    QSqlDatabase::database().transaction();
    for(;iter!=mapAlarmConfig.end();++iter)
    {
        qquery.bindValue(":LimitValue",(*iter).second.fLimitvalue);
        qquery.bindValue(":AlarmLevel",(*iter).second.iAlarmlevel);
        qquery.bindValue(":JumpLimitType",(*iter).second.iLimittype);
        qquery.bindValue(":LinkageEnable",(*iter).second.iLinkageEnable);
        qquery.bindValue(":LinkageRoleNumber",QString::fromStdString((*iter).second.strLinkageRoleNumber));
        qquery.bindValue(":DeviceNumber",QString::fromStdString(strDevnum));
        qquery.bindValue(":MonitoringIndex",(*iter).first);
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

bool DataBaseOperation::AddItemAlarmRecord( string strDevnum,time_t startTime,int nMonitoringIndex,int nlimitType,int nalarmTypeId,double dValue,
                                            const string &sreason,unsigned long long& irecordid )
{
    if(!IsOpen())
    {
        std::cout<<"数据库未打开"<<std::endl;
        return false;
    }
    QSqlQuery inquery;
    QString strSql=QString("insert into device_alarm_record(devicenumber,monitoringindex,alarmstarttime,alarmendtime,limittype,alarmvalue,alarmtypeid,alarmreason) values(:devicenumber,:monitoringindex,\
                           :alarmstarttime,:limittype,:alarmvalue,:alarmtypeid,:alarmreason)");
            inquery.prepare(strSql);
    inquery.bindValue(":devicenumber",QString::fromStdString(strDevnum));
    inquery.bindValue(":monitoringindex",nMonitoringIndex);
    tm *ltime = localtime(&startTime);
    QDateTime qdt;
    qdt.setDate(QDate(ltime->tm_year+1900,ltime->tm_mon+1,ltime->tm_mday));
    qdt.setTime(QTime(ltime->tm_hour,ltime->tm_min,ltime->tm_sec));
    inquery.bindValue(":alarmstarttime",qdt);
    inquery.bindValue(":limittype",nlimitType);
    inquery.bindValue(":alarmvalue",dValue);
    inquery.bindValue(":alarmtypeid",nalarmTypeId);
    inquery.bindValue(":alarmreason",QString::fromStdString(sreason));
    boost::mutex::scoped_lock lock(db_connect_mutex_);
    if(!inquery.exec())
    {
        std::cout<<inquery.lastError().text().toStdString()<<std::endl;
        return false;
    }
    strSql=QString("select max(id) from device_alarm_record");
    inquery.prepare(strSql);
    if(inquery.exec())
    {
        if(inquery.next())
        {
            irecordid = inquery.value(0).toULongLong();
        }
    }
    return  true;
}

bool DataBaseOperation::AddItemEndAlarmRecord( time_t endTime,unsigned long long irecordid )
{
    if(!IsOpen())
    {
        std::cout<<"数据库未打开"<<std::endl;
        return false;
    }
    QSqlQuery inquery;
    QString strSql=QString("update device_alarm_record set alarmendtime=:alarmendtime where id=:id ");//:alarmendtime,
    inquery.prepare(strSql);
    inquery.bindValue(":id",irecordid);

    QDateTime qdt;
    tm *ltime = localtime(&endTime);
    qdt.setDate(QDate(ltime->tm_year+1900,ltime->tm_mon+1,ltime->tm_mday));
    qdt.setTime(QTime(ltime->tm_hour,ltime->tm_min,ltime->tm_sec));
    inquery.bindValue(":alarmendtime",qdt);
    if(!inquery.exec())
    {
        std::cout<<inquery.lastError().text().toStdString()<<std::endl;
        return false;
    }

    return  true;
}
bool DataBaseOperation::AddItemMonitorRecord( string strDevnum,time_t savetime,DevMonitorDataPtr pdata)
{
    if(!IsOpen())
    {
        std::cout<<"数据库未打开"<<std::endl;
        return false;
    }
    QSqlQuery inquery;
    QString strSql=QString("insert into device_monitoring_record(devicenumber,monitoringindex,monitoringtime,monitoringvalue) values(:devicenumber,:monitoringindex,\
                           :monitoringtime,:monitoringvalue)");
            inquery.prepare(strSql);
    QString qstrNum = QString::fromStdString(strDevnum);
    inquery.bindValue(":devicenumber",qstrNum);
    map<int,DataInfo>::iterator iter = pdata->mValues.begin();
    QSqlDatabase::database().transaction();
    for(;iter!=pdata->mValues.end();++iter)
    {
        inquery.bindValue(":monitoringindex",(*iter).first);
        QDateTime qdt;
        tm *ltime = localtime(&savetime);
        qdt.setDate(QDate(ltime->tm_year+1900,ltime->tm_mon+1,ltime->tm_mday));
        qdt.setTime(QTime(ltime->tm_hour,ltime->tm_min,ltime->tm_sec));
        inquery.bindValue(":monitoringtime",qdt);
        inquery.bindValue(":monitoringvalue",(*iter).second.fValue);
        if(!inquery.exec())
        {
            std::cout<<inquery.lastError().text().toStdString()<<std::endl;
            QSqlDatabase::database().rollback();
            return false;
        }
    }
    QSqlDatabase::database().commit();
    return true;
}
//改完待测2016-03-30 16:53
bool DataBaseOperation::SetEnableAlarm( rapidxml::xml_node<char>* root_node,int& resValue,vector<string>& vecDevid )
{
    if(!IsOpen())
    {
        std::cout<<"数据库未打开"<<std::endl;
        resValue = 3;
        return false;
    }
    rapidxml::xml_node<>* tranNode = root_node->first_node("TranInfo");
    if(tranNode==NULL)
    {
        tranNode = root_node->first_node("Dev");
        if(tranNode==NULL){
            resValue = 11;
            return false;
        }
    }
    for(;tranNode!=NULL;tranNode=tranNode->next_sibling())
    {
        rapidxml::xml_attribute<char> * attr = tranNode->first_attribute("TransmitterID");
        if(attr==NULL)
        {
            attr = tranNode->first_attribute("ID");
            if(attr==NULL){
                resValue = 11;
                return false;
            }
        }
        QString qsTransNum = attr->value();
        boost::mutex::scoped_lock lock(db_connect_mutex_);
        QSqlDatabase::database().transaction();
        QString strDel = QString("delete from device_alarm_switch where devicenumber='%1'").arg(qsTransNum);
        QSqlQuery qsDel;

        if(!qsDel.exec(strDel))
        {
            QSqlDatabase::database().rollback();
            resValue = 3;
            return false;
        }


        rapidxml::xml_node<>* alswNode = tranNode->first_node("AlarmSwitch");
        QSqlQuery qsInsert;
        QString strSql = QString("insert into device_alarm_switch(devicenumber,alarmswitchtype,alarmenable,description) values(:devicenumber,:alarmswitchtype,:alarmenable,:description)");
        qsInsert.prepare(strSql);
        qsInsert.bindValue(":devicenumber",qsTransNum);
        for(;alswNode!=NULL;alswNode=alswNode->next_sibling())
        {

            rapidxml::xml_attribute<>* atType = alswNode->first_attribute("Type");
            if(atType!=NULL)
            {
                int itype = atoi(atType->value());
                QSqlQuery qsSelIndex;
                qsInsert.bindValue(":alarmswitchtype",itype);
                rapidxml::xml_attribute<>* atSwitch = alswNode->first_attribute("Switch");
                int iSwitch = 1;
                if(atSwitch)
                {
                    iSwitch = atoi(atSwitch->value());
                }
                qsInsert.bindValue(":alarmenable",iSwitch);
                rapidxml::xml_attribute<>* atDesc = alswNode->first_attribute("Desc");
                if(atDesc!=NULL)
                {
                    qsInsert.bindValue(":description",atDesc->value());
                }
                if(!qsInsert.exec())
                {
                    QSqlDatabase::database().rollback();
                    resValue = 3;
                    return false;
                }

            }
        }
        QSqlDatabase::database().commit();
        vecDevid.push_back(qsTransNum.toStdString());
    }

    resValue = 0;
    return true;
}
//改完待测2016-03-30 16:40
bool DataBaseOperation::SetAlarmLimit( rapidxml::xml_node<char>* root_node,int& resValue,vector<string>& vecDevid )
{
    if(!IsOpen())
    {
        std::cout<<"数据库未打开"<<std::endl;
        resValue = 3;
        return false;
    }
    rapidxml::xml_node<>* tranNode = root_node->first_node("TranInfo");
    if(tranNode==NULL)
    {
        tranNode = root_node->first_node("Dev");
        if(tranNode==NULL){
            resValue = 11;
            return false;
        }
    }

    for(;tranNode!=NULL;tranNode=tranNode->next_sibling())
    {
        rapidxml::xml_attribute<char> * attr = tranNode->first_attribute("TransmitterID");
        if(attr==NULL)
        {
            attr = tranNode->first_attribute("ID");
            if(attr==NULL){
                resValue = 11;
                return false;
            }
        }
        QString qsTransNum = attr->value();
        boost::mutex::scoped_lock lock(db_connect_mutex_);
        QSqlDatabase::database().transaction();
        QSqlQuery qsDel;
        QString strSql=QString("delete from alarm_item_config where devicenumber=:devicenumber");
        qsDel.prepare(strSql);
        qsDel.bindValue(":devicenumber",qsTransNum);
        if(!qsDel.exec())
        {
            QSqlDatabase::database().rollback();
            resValue = 3;
            return false;
        }
        rapidxml::xml_node<>* alswNode = tranNode->first_node("AlarmParam");
        QSqlQuery qsInsert;
        strSql=QString("insert into alarm_item_config(devicenumber,monitoringindex,limitvalue,jumplimittype,delaytime,resumeduration,alarmconfigtype,alarmenable) \
                       values(:devicenumber,:monitoringindex,:limitvalue,:jumplimittype,:delaytime,:resumeduration,:alarmconfigtype,1)");
                       qsInsert.prepare(strSql);
                qsInsert.bindValue(":devicenumber",qsTransNum);
        if(alswNode == NULL){
            QSqlDatabase::database().rollback();
            resValue = 11;
            return false;
        }

        for(;alswNode!=NULL;alswNode=alswNode->next_sibling())
        {
            rapidxml::xml_attribute<>* atType = alswNode->first_attribute("Type");
            if(atType!=NULL)
            {
                int itype = atoi(atType->value());

                if(itype!=512 && itype!=511)
                {
                    qsInsert.bindValue(":alarmconfigtype",0);
                }
                else
                {
                    qsInsert.bindValue(":alarmconfigtype",1);
                }
                qsInsert.bindValue(":monitoringindex",itype);



                rapidxml::xml_attribute<>* atDuration=NULL;
                if(itype==512)
                {
                    atDuration = alswNode->first_attribute("EarlyDuration");
                    if(atDuration==NULL)
                    {
                        QSqlDatabase::database().rollback();
                        resValue = 11;
                        return false;
                    }
                    qsInsert.bindValue(":delaytime",atoi(atDuration->value()));
                    atDuration = alswNode->first_attribute("DelayedDuration");
                    if(atDuration==NULL)
                    {
                        QSqlDatabase::database().rollback();
                        resValue = 11;
                        return false;//resumeduration
                    }
                    qsInsert.bindValue(":resumeduration",atoi(atDuration->value()));
                }
                else
                {
                    atDuration = alswNode->first_attribute("Duration");
                    if(atDuration==NULL)
                    {
                        QSqlDatabase::database().rollback();
                        resValue = 11;
                        return false;
                    }
                    qsInsert.bindValue(":delaytime",atoi(atDuration->value()));
                    atDuration = alswNode->first_attribute("ResumeDuration");
                    if(atDuration==NULL)
                    {
                        QSqlDatabase::database().rollback();
                        resValue = 11;
                        return false;
                    }
                    qsInsert.bindValue(":resumeduration",atoi(atDuration->value()));
                }

                 rapidxml::xml_attribute<>* atTP = alswNode->first_attribute("DownThreshold");
                 if(atTP!=NULL){
                      qsInsert.bindValue(":jumplimittype",1);
                      qsInsert.bindValue(":limitvalue",atof(atTP->value()));
                        if(!qsInsert.exec())
                         {
                             QSqlDatabase::database().rollback();
                             resValue = 3;
                             return false;
                         }
                 }

                 atTP = alswNode->first_attribute("UpThreshold");
                                 if(atTP!=NULL){
                                      qsInsert.bindValue(":jumplimittype",0);
                                      qsInsert.bindValue(":limitvalue",atof(atTP->value()));
                                        if(!qsInsert.exec())
                                         {
                                             QSqlDatabase::database().rollback();
                                             resValue = 3;
                                             return false;
                                         }
                                 }
               if(itype==(511) || itype==(512))
               {
                   qsInsert.bindValue(":jumplimittype",4);
                if(!qsInsert.exec())
                {
                    QSqlDatabase::database().rollback();
                    resValue = 3;
                    return false;
                }
               }
            }
        }
        QSqlDatabase::database().commit();
        vecDevid.push_back(qsTransNum.toStdString());
    }
    resValue = 0;
    return true;
}
//设置发射机运行图
bool DataBaseOperation::SetAlarmTime( rapidxml::xml_node<char>* root_node,int& resValue,vector<string>& vecDevid )
{
    if(!IsOpen())
    {
        std::cout<<"数据库未打开"<<std::endl;
        resValue = 3;
        return false;
    }
    rapidxml::xml_node<>* tranNode = root_node->first_node("TranInfo");
    if(tranNode==NULL)
    {
        resValue = 11;
        return false;
    }
    QSqlDatabase::database().transaction();
    for(;tranNode!=NULL;tranNode=tranNode->next_sibling())
    {
        rapidxml::xml_attribute<char> * attr = tranNode->first_attribute("TransmitterID");
        if(attr==NULL)
        {
            resValue = 11;
            return false;
        }
        QString qsTransNum = attr->value();

        QSqlQuery qsDel;
        QString strSql=QString("delete from monitoring_scheduler where objectnumber=:objectnumber");
        qsDel.prepare(strSql);
        qsDel.bindValue(":objectnumber",qsTransNum);
        if(!qsDel.exec())
        {
            QSqlDatabase::database().rollback();
            resValue = 3;
            return false;
        }

        rapidxml::xml_node<>* setnode = tranNode->first_node();
        if(setnode==NULL)
        {
            QSqlDatabase::database().rollback();
            resValue = 11;
            return false;
        }

        QSqlQuery insertQuery;//0:星期 1:月 2:天
        strSql = QString("insert into monitoring_scheduler(objectnumber,weekday,enable,starttime,endtime,datetype,month,day,alarmendtime) \
                         values(:objectnumber,:weekday,:enable,:starttime,:endtime,:datetype,:month,:day,:alarmendtime)");
                         insertQuery.prepare(strSql);
                insertQuery.bindValue(":objectnumber",qsTransNum);
        while(setnode)
        {
            QString name=setnode->name();
            if(name!="MonthTime" && name!="WeeklyTime" && name!="DayTime" )
            {
                QSqlDatabase::database().rollback();
                resValue = 11;
                return false;
            }
            int shutype=0;
            if(name=="MonthTime")
            {
                shutype = 1;
            }
            else if(name=="DayTime")
            {
                shutype = 2;
            }
            insertQuery.bindValue(":datetype",shutype);

            QDateTime qdt;
            qdt.setDate(QDate(1970,1,1));
            rapidxml::xml_attribute<char> * attrStarttime = setnode->first_attribute("StartTime");
            if(attrStarttime==NULL)
            {
                attrStarttime = setnode->first_attribute("StartDateTime");
                if(attrStarttime==NULL){
                    QSqlDatabase::database().rollback();
                    resValue = 11;
                    return false;
                }else
                    qdt=QDateTime::fromString(attrStarttime->value(),"yyyy-MM-dd hh:mm:ss");
            }else
                qdt=QDateTime::fromString(attrStarttime->value(),"hh:mm:ss");


            if(qdt.isValid()==false){
                QSqlDatabase::database().rollback();
                resValue = 11;
                return false;
            }
            insertQuery.bindValue(":starttime",qdt);
            rapidxml::xml_attribute<char> * attrEndtime = setnode->first_attribute("EndTime");
            if(attrEndtime==NULL)
            {
                attrEndtime = setnode->first_attribute("EndDateTime");
                if(attrEndtime==NULL){
                    QSqlDatabase::database().rollback();
                    resValue = 11;
                    return false;
                }else
                    qdt=QDateTime::fromString(attrEndtime->value(),"yyyy-MM-dd hh:mm:ss");
            }else
                qdt=QDateTime::fromString(attrEndtime->value(),"hh:mm:ss");

            if(qdt.isValid()==false){
                QSqlDatabase::database().rollback();
                resValue = 11;
                return false;
            }

            insertQuery.bindValue(":endtime",qdt);
            rapidxml::xml_attribute<char> * attrType = setnode->first_attribute("Type");
            if(attrType==NULL)
            {
                QSqlDatabase::database().rollback();
                resValue = 11;
                return false;
            }
            insertQuery.bindValue(":enable",atoi(attrType->value()));
            int iweek = 0;
            int iMon = -1;
            int iday = 0;
            QDateTime qAlarmendDt;// = QDateTime::currentDateTime();
            switch(shutype)
            {
            case 1:
            {
                rapidxml::xml_attribute<char> * attrMotn = setnode->first_attribute("Month");
                if(attrMotn==NULL)
                {
                    QSqlDatabase::database().rollback();
                    resValue = 11;
                    return false;
                }
                iMon = atoi(attrMotn->value());
                rapidxml::xml_attribute<char> * attrDay = setnode->first_attribute("Day");
                if(attrDay==NULL)
                {
                    QSqlDatabase::database().rollback();
                    resValue = 11;
                    return false;
                }
                iday=atoi(attrDay->value());

                rapidxml::xml_attribute<char> * attrend = setnode->first_attribute("AlarmEndTime");
                if(attrend==NULL)
                {
                    //QSqlDatabase::database().rollback();
                    //resValue = 11;
                    break;//return false;
                }
                qAlarmendDt = QDateTime::fromString(attrend->value(),"yyyy-MM-dd hh:mm:ss");
            }
                break;
            case 0:
            {
                rapidxml::xml_attribute<char> * attrWeek = setnode->first_attribute("DayofWeek");
                if(attrWeek==NULL)
                {
                    QSqlDatabase::database().rollback();
                    resValue = 11;
                    return false;
                }
                iweek = atoi(attrWeek->value());
                rapidxml::xml_attribute<char> * attrend = setnode->first_attribute("AlarmEndTime");
                if(attrend==NULL)
                {
                    //QSqlDatabase::database().rollback();
                    //resValue = 11;
                    break;
                }
                qAlarmendDt = QDateTime::fromString(attrend->value(),"yyyy-MM-dd hh:mm:ss");
            }
                break;
            case 2:
            {

            }
                break;
            }
            insertQuery.bindValue(":weekday",iweek);
            insertQuery.bindValue(":month",iMon);
            insertQuery.bindValue(":day",iday);
            insertQuery.bindValue(":alarmendtime",qAlarmendDt);
            if(!insertQuery.exec())
            {
                QSqlDatabase::database().rollback();
                resValue = 11;
                return false;
            }
            setnode = setnode->next_sibling();
        }
        vecDevid.push_back(qsTransNum.toStdString());
    }
    QSqlDatabase::database().commit();
    resValue = 0;
    return true;
}

bool DataBaseOperation::GetUserInfo( const string sName,UserInformation &user )
{
    if(!IsOpen())
    {
        std::cout<<"数据库未打开"<<std::endl;
        return false;
    }
    QSqlQuery userquery;
    QString strSql=QString("select Number,Password,controllevel,Headship,JobNumber from Users where Name='%1'").arg(QString::fromStdString(sName));
    if(!userquery.exec(strSql))
    {
        return false;
    }
    if(userquery.next())
    {
        user.sName = sName;
        user.sNumber = userquery.value(0).toString().toStdString();
        user.sPassword = userquery.value(1).toString().toStdString();
        user.nControlLevel = userquery.value(2).toInt();
        user.sHeadship = userquery.value(3).toString().toStdString();
        user.sJobNumber = userquery.value(4).toString().toStdString();
    }
    return true;
}

bool DataBaseOperation::GetAllAuthorizeDevByUser( const string sUserId,vector<string> &vDevice )
{
    if(!IsOpen())
    {
        std::cout<<"数据库未打开"<<std::endl;
        return false;
    }
    QSqlQuery query;
    QString strSql=QString("select a.objectnumber from user_role_object a,users b where b.number='%1' and a.rolenumber=b.rolenumber").arg(QString::fromStdString(sUserId));
    if(!query.exec(strSql))
    {
        return false;
    }
    while(query.next())
    {
        string sdevnum;
        sdevnum = query.value(0).toString().toStdString();
        vDevice.push_back(sdevnum);
    }
    return true;
}

}
