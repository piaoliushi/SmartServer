#include "DataBaseOperation.h"
#include <sstream>
#include <QSqlError>
#include <QDateTime>
#include <QVariant>
#include <QStringList>
#include "../rapidxml/rapidxml_print.hpp"
#include "../rapidxml/rapidxml_utils.hpp"
#include "../protocol/bohui_const_define.h"
#include "ConnectionPool.h"
#include "../StationConfig.h"
namespace db {

DataBaseOperation::DataBaseOperation()
{
    //reconnect_thread_.reset();
    //d_db_check_time = QDateTime::currentDateTime();
}

DataBaseOperation::~DataBaseOperation()
{

}

bool DataBaseOperation::OpenDb( const std::string& serveraddress, const std::string& database,
                                const std::string& uid, const std::string& pwd, int timeout,
                                std::string link_driver,std::string driverName/*="SQL Native Client"*/ )
{

    ConnectionPool::initDb(QString::fromStdString(serveraddress),QString::fromStdString(database)
                           ,QString::fromStdString(uid),QString::fromStdString(pwd));

    QSqlDatabase db = ConnectionPool::openConnection();
    if(!db.isOpen() || !db.isValid()) {
        std::cout<<"connect  database  is error ------------------------------ the database is interrupt"<<std::endl;
        return false;
    }
    ConnectionPool::closeConnection(db);
    return true;
}

bool DataBaseOperation::CloseDb()
{
    return true;
}

bool DataBaseOperation::IsOpen()
{
    boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    QSqlDatabase  db = QSqlDatabase::database();
    return db.isOpen();
}

bool DataBaseOperation::check_database()
{
    if(d_db_check_time.secsTo(QDateTime::currentDateTime())>5){
        QSqlQuery query;
        QString sNow("select now()");
        query.prepare(sNow);
        if(!query.exec()){
            d_db_check_time = QDateTime::currentDateTime();
            StartReOpen();
            return false;
        }
        d_db_check_time = QDateTime::currentDateTime();
    }
    return true;
}


void DataBaseOperation::StartReOpen()
{
    if(reconnect_thread_==NULL)
        reconnect_thread_.reset(new boost::thread(boost::bind(&DataBaseOperation::ReOpen,this)));
}

bool DataBaseOperation::ReOpen()
{
    CloseDb();

    d_cur_Notify->OnDatabase(false);

    QSqlDatabase q_db = QSqlDatabase::addDatabase("QPSQL");
    q_db.setHostName(d_serveraddress);//设置主机名
    q_db.setDatabaseName(d_database);//设置数据库名
    q_db.setUserName(d_uid);//设置用户名
    q_db.setPassword(d_pwd);//设置用户密码
    if(!q_db.open())
    {
        boost::this_thread::sleep(boost::posix_time::milliseconds(5000));
        return ReOpen();
    }else {
        d_cur_Notify->OnDatabase(true);
    }
    return true;
}

//获得数据字典映射表(告警与监控量id)
bool DataBaseOperation::GetDataDictionary(map<int,pair<string,string> >& mapDicry)
{
    boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    QSqlDatabase db = ConnectionPool::openConnection();
    if(!db.isOpen() || !db.isValid()) {
        std::cout<<"GetDataDictionary is error ------------------------------ the database is interrupt"<<std::endl;
        return false;
    }

    QSqlQuery query(db);
    QString strSql=QString("select code,name,remark from data_dictionary where type='IndexType' "
                           "union select alarmswitch,alarmswitchname,remark from alarm_switch where alarmswitchtype<>5 and alarmswitchtype<>6");
    query.prepare(strSql);
    if(!query.exec()){
        cout<<query.lastError().text().toStdString()<<"GetDataDictionary---query---error!"<<endl;
        ConnectionPool::closeConnection(db);
        return false;
    }
    while(query.next()) {
        mapDicry[query.value(0).toInt()] = pair<string,string>(query.value(1).toString().toStdString(),query.value(2).toString().toStdString());
        //cout<<query.value(0).toString().toStdString()<<endl;//<<query.value(2).toString().toStdString()
    }
    ConnectionPool::closeConnection(db);
    return true;
}


//获得数据字典服务器字符串常量
bool DataBaseOperation::GetDeviceDataDictionary(map<string,map<int,string> >& mapDicry)
{
    boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    QSqlDatabase db = ConnectionPool::openConnection();
    if(!db.isOpen() || !db.isValid()) {//IsOpen()
        std::cout<<"GetDataDictionary is error ------------------------------ the database is interrupt"<<std::endl;
        return false;
    }

    QSqlQuery query(db);
    QString strSql=QString("select type,code,name,remark from data_dictionary where type in('DeviceType','CommandType','s_tsmt_target_desc','s_sh_tsmt_rf_mod','s_sh_tsmt_gps_s',\
                           's_sh_tsmt_cpilot','s_sh_tsmt_chpn','s_rsps_result_desc','s_dtmb_mod','s_dtmb_base','remind_notify_originator','s_cmd_result_desc','s_cmd_opr_desc','s_base','s_alarm_event','s_cmd_excute_mode') order by type,code");
            query.prepare(strSql);
    if(!query.exec()){
        cout<<query.lastError().text().toStdString()<<"GetDataDictionary---query---error!"<<endl;
        ConnectionPool::closeConnection(db);
        return false;
    }
    while(query.next()) {
        string sType = query.value(0).toString().toStdString();
        int nCode = query.value(1).toInt();
        QString qname = query.value(2).toString();

        map<string,map<int,string> >::iterator iter = mapDicry.find(sType);
        if(iter!=mapDicry.end()){
            mapDicry[sType][nCode] = qname.toStdString();
        }else {
            map<int,string> curInfo;
            curInfo[nCode]=qname.toStdString();
            mapDicry[sType]=curInfo;
        }
    }

    ConnectionPool::closeConnection(db);
    return true;
}

//获取某个设备信息(剔除短信猫配置信息,判断该设备是否属于该授权服务器)
bool DataBaseOperation::GetDevInfo(QSqlDatabase &db, string strDevnum,DeviceInfo& device,string sServerNumber)
{

    //boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    if(!db.isOpen() || !db.isValid()) {
        std::cout<<"GetDevInfo is error ------------------------------- the database is interrupt"<<std::endl;
        return false;
    }

    QSqlQuery devquery(db);
    QString strSql=QString("select a.DeviceNumber,a.AssociateNumber,a.DeviceName,a.DeviceType,a.IsAssociate,a.IsMultiChannel,a.ChannelSize,a.IsUse,a.AddressCode,b.MainCategoryNumber,b.SubCategoryNumber,a.ProtocolNumber\
                           from Device a,Device_Map_Protocol b where a.DeviceNumber='%1' and a.DeviceType<>300 and b.ProtocolNumber=a.ProtocolNumber and a.DeviceNumber in \
            (select c.objectnumber from platform_server_purview c where c.servernumber='%2' and c.objecttype=0 )").arg(QString::fromStdString(strDevnum) ).arg(QString::fromStdString(sServerNumber) );
            devquery.prepare(strSql);
            if(devquery.exec())   {
                    if(devquery.next())  {
                    device.sDevNum = devquery.value(0).toString().toStdString();
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
                    //根据设备id填充台站编号
                    GetObjOwnerStationNum(db,strDevnum,device.sStationNum);
                    //获得监控量信息
                    GetDevMonItem(db,strDevnum,sprotoclnum,device.map_MonitorItem);
                    //获得运行图信息
                    GetDevMonitorSch(db,strDevnum,device.vMonitorSch);
                    //获得命令图信息
                    GetCmd(db,strDevnum,device.vCommSch);
                    //获得设备属性信息
                    GetDevProperty(db,strDevnum,device.map_DevProperty);
                    if(device.bMulChannel && device.iChanSize>1){
                         for(int i=1;i<device.iChanSize;++i){
                            map<string,DevProperty> chlPropertyEx;
                            if(GetDevProperty(db,strDevnum,chlPropertyEx,i))
                            {
                                device.map_DevChannelPropertyEx[i] = chlPropertyEx;
                            }
                         }
                    }
                    //获得告警配置
                    GetAlarmConfig(db,strDevnum,device.map_AlarmConfig);
                    //获得关联配置
                    GetAssDevChan(db,strDevnum,device.map_AssDevChan);

                }
        }else {
            std::cout<<devquery.lastError().text().toStdString()<<"GetDevInfo---query---error!"<<std::endl;
            return false;
         }
        return true;
}

//获取所有设备信息
bool DataBaseOperation::GetAllDevInfo( vector<ModleInfo>& v_Linkinfo,string sStationId,string sServerId)
{
    boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    QSqlDatabase db = ConnectionPool::openConnection();
    if(!db.isOpen() || !db.isValid()) {
        std::cout<<"GetAllDevInfo is error -------------------------------- the database is interrupt"<<std::endl;
        return false;
    }

    QSqlQuery netquery(db);
    //QString strSql=QString("select NetType,IpAddress,LocalPort,PeerPort,ConnectType,CommTypeNumber from Net_Communication_Mode where CommTypeNumber in\
    //                       (select objectnumber from station_bind_object where stationnumber='%1' and objecttype=5)").arg(QString::fromStdString(sStationId));

    QString strSql=QString("select NetType,IpAddress,LocalPort,PeerPort,ConnectType,CommTypeNumber from Net_Communication_Mode where CommTypeNumber in\
                           (select objectnumber from station_bind_object where objecttype=5)");
    netquery.prepare(strSql);
    if(netquery.exec()){
        while(netquery.next()){
            ModleInfo info;
            info.iCommunicationMode = 1;
            info.netMode.inet_type = netquery.value(0).toInt();
            info.netMode.strIp = netquery.value(1).toString().toStdString();
            info.netMode.ilocal_port = netquery.value(2).toInt();
            info.netMode.iremote_port = netquery.value(3).toInt();
            info.netMode.ilink_type = netquery.value(4).toInt();

            QString qtrNum = netquery.value(5).toString();
            info.sModleNumber = qtrNum.toStdString();
            //获取model所属台站
            GetObjOwnerStationNum(db,info.sModleNumber,info.sStationNum);

            QString strQdev = QString("select DeviceNumber from Device_Bind_Comm where CommTypeNumber='%1'").arg(qtrNum);
            QSqlQuery net1query(db);
            if(net1query.exec(strQdev)) {
                while(net1query.next()) {
                    DeviceInfo dev;
                    //获得设备配置信息,且判断该设备是否授权
                    GetDevInfo(db,net1query.value(0).toString().toStdString(),dev,sServerId);
                    if(dev.sDevNum.length()>0)
                    info.mapDevInfo[net1query.value(0).toString().toStdString()]=dev;
                }
            }

            if(net1query.size()>0 && info.mapDevInfo.size()>0)
            v_Linkinfo.push_back(info);
        }
    }
    else {
        cout<<netquery.lastError().text().toStdString()<<"GetAllDevInfo---netquery---error!"<<endl;
    }
    QSqlQuery comquery(db);
    //strSql=QString("select Com,Baudrate,Databit,Stopbit,Parity,CommTypeNumber from Com_Communication_Mode where CommTypeNumber in\
    //               (select objectnumber from station_bind_object where stationnumber='%1' and objecttype=4)").arg(QString::fromStdString(sStationId));
    strSql=QString("select Com,Baudrate,Databit,Stopbit,Parity,CommTypeNumber from Com_Communication_Mode where CommTypeNumber in\
                    (select objectnumber from station_bind_object where objecttype=4)");
    comquery.prepare(strSql);
    if(comquery.exec()){
        while(comquery.next()) {
            ModleInfo info;
            info.iCommunicationMode = 0;
            info.comMode.icomport = comquery.value(0).toInt();
            info.comMode.irate = comquery.value(1).toInt();
            info.comMode.idata_bit = comquery.value(2).toInt();
            info.comMode.istop_bit = comquery.value(3).toInt();
            info.comMode.iparity_bit = comquery.value(4).toInt();

            QString qtrNum = comquery.value(5).toString();
            info.sModleNumber = qtrNum.toStdString();
            QString strQdev = QString("select DeviceNumber from Device_Bind_Comm where CommTypeNumber='%1'").arg(qtrNum);
            QSqlQuery net1query(db);
            if(net1query.exec(strQdev)) {
                while(net1query.next()){
                    DeviceInfo dev;
                    GetDevInfo(db,net1query.value(0).toString().toStdString(),dev,sServerId);
                    if(dev.sDevNum.length()>0)
                        info.mapDevInfo[net1query.value(0).toString().toStdString()]=dev;
                }
            }

            if(net1query.size()>0 && info.mapDevInfo.size()>0)
            v_Linkinfo.push_back(info);
        }
    }else{
        cout<<comquery.lastError().text().toStdString()<<"GetAllDevInfo---comquery---error!"<<endl;
    }


    ConnectionPool::closeConnection(db);
    return true;
}

//获取设备监控运行图
bool DataBaseOperation::GetDevMonitorSch(QSqlDatabase &db, string strDevnum,map<int,vector<Monitoring_Scheduler> >& mapMonitorSch )
{
    //QSqlDatabase db = ConnectionPool::openConnection();
    if(!db.isOpen() || !db.isValid()) {
        std::cout<<"GetDevMonitorSch is error -------------------------------- the database is interrupt"<<std::endl;
        return false;
    }
    boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    QSqlQuery schquery(db);
    QString strSql=QString("select id,ObjectNumber,WeekDay,Enable,StartTime,EndTime,datetype,month,day, \
                           alarmendtime,mode,channelid from Monitoring_Scheduler where ObjectNumber='%1'").arg(QString::fromStdString(strDevnum));
            schquery.prepare(strSql);
            if(schquery.exec()){
            while(schquery.next()) {
            Monitoring_Scheduler msch;
            msch.iMonitorWeek = schquery.value(2).toInt();
            msch.bMonitorFlag = schquery.value(3).toBool();
            msch.tStartTime = schquery.value(4).toDateTime().toTime_t();
            msch.tEndTime = schquery.value(5).toDateTime().toTime_t();
            int iMonitorType = schquery.value(6).toInt();
            msch.iMonitorMonth = schquery.value(7).toInt();
            msch.iMonitorDay = schquery.value(8).toInt();
            msch.tAlarmEndTime = schquery.value(9).toDateTime().toTime_t();
            msch.bRunModeFlag = schquery.value(10).toBool();
            msch.iChannelId = schquery.value(11).toInt();
            mapMonitorSch[iMonitorType].push_back(msch);
}
}
            else {
            cout<<schquery.lastError().text().toStdString()<<"GetDevMonitorSch---schquery---error!"<<endl;
            return false;
}
            return true;
}

//获得命令参数
bool DataBaseOperation::GetCmdParam(QSqlDatabase &db, string strCmdnum,CmdParam& param )
{
    //QSqlDatabase db = ConnectionPool::openConnection();
    if(!db.isOpen() || !db.isValid()) {//IsOpen()
        std::cout<<"GetCmdParam is error -------------------------------- the database is interrupt"<<std::endl;
        return false;
    }
    boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    QSqlQuery cmdparquery(db);
    QString strSql=QString("select Param0,HasParam1,Param1 from Param_Define where ParamNumber='%1'").arg(QString::fromStdString(strCmdnum));
    cmdparquery.prepare(strSql);
    if(cmdparquery.exec()){
        if(cmdparquery.next()){
            param.sParam1 = cmdparquery.value(0).toString().toStdString();
            param.bUseP2 = cmdparquery.value(1).toBool();
            param.sParam2 = cmdparquery.value(2).toString().toStdString();
        }
    }
    else {
        cout<<cmdparquery.lastError().text().toStdString()<<"GetCmdParam---cmdparquery---error!"<<endl;
        return false;
    }
    return true;
}

//获取命令配置
bool DataBaseOperation::GetCmd(QSqlDatabase &db, string strDevnum,vector<Command_Scheduler>& vcmdsch )
{
    //QSqlDatabase db = ConnectionPool::openConnection();
    if(!db.isOpen() || !db.isValid()) {
        std::cout<<"GetCmd is error -------------------------------- the database is interrupt"<<std::endl;
        return false;
    }
    boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    QSqlQuery cmdschquery(db);
    QString strSql=QString("select id,CommandType,WeekDay,StartTime,HasParam,ParamNumber,month,day,commandendtime,\
                           datetype,param1,UseP2,param2,channelid,remindnumber from Command_Scheduler where ObjectNumber='%1' and Enable=1").arg(QString::fromStdString(strDevnum));

            cmdschquery.prepare(strSql);
            if(cmdschquery.exec()){
                while(cmdschquery.next()){
                    Command_Scheduler cmd_sch;
                    cmd_sch.gid = cmdschquery.value(0).toInt();
                    cmd_sch.iCommandType = cmdschquery.value(1).toInt();
                    cmd_sch.iWeek = cmdschquery.value(2).toInt();
                    cmd_sch.tExecuteTime = cmdschquery.value(3).toDateTime().toTime_t();
                    cmd_sch.iHasParam = cmdschquery.value(4).toInt();
                    if(cmd_sch.iHasParam>=1){
                        cmd_sch.cParam.sParam1 = cmdschquery.value(10).toString().toStdString();
                        cmd_sch.cParam.bUseP2 = cmdschquery.value(11).toBool();
                        cmd_sch.cParam.sParam2 = cmdschquery.value(12).toString().toStdString();
                    }
                    cmd_sch.iMonitorMonth = cmdschquery.value(6).toInt();
                    cmd_sch.iMonitorDay = cmdschquery.value(7).toInt();
                    cmd_sch.tCmdEndTime = cmdschquery.value(8).toDateTime().toTime_t();
                    cmd_sch.iDateType = cmdschquery.value(9).toInt();
                    cmd_sch.iChannelId = cmdschquery.value(13).toInt();//add by lk 2017-8-17
                    cmd_sch.remindnumber = cmdschquery.value(14).toString().toStdString();
                    vcmdsch.push_back(cmd_sch);
                }
            }
            else{
                cout<<cmdschquery.lastError().text().toStdString()<<"GetCmd---cmdschquery---error!"<<endl;
                return false;
            }
      return true;
}
//根据设备id填充台站编号
bool DataBaseOperation::GetObjOwnerStationNum(QSqlDatabase &db,string strObjnum,string &sDevOwnerStationId)
{
    if(!db.isOpen() || !db.isValid()) {
        std::cout<<"GetObjOwnerStationNum is error --------------------------------the database is interrupt"<<std::endl;
        return false;
    }

    QSqlQuery stationIdquery(db);
    QString strSql=QString("select distinct stationnumber from station_bind_object where objectnumber='%1'").arg(QString::fromStdString(strObjnum));
    if(stationIdquery.exec(strSql)) {
        if(stationIdquery.next()) {
           sDevOwnerStationId = stationIdquery.value(0).toString().toStdString();
        }
    }else{
        cout<<stationIdquery.lastError().text().toStdString()<<"GetObjOwnerStationNum---stationIdquery---error!"<<endl;
        return false;
    }
    return true;
}
//设备设备监控量
bool DataBaseOperation::GetDevMonItem(QSqlDatabase &db, string strDevnum,QString qsPrtocolNum,map<int,DeviceMonitorItem>& map_item )
{
    //boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    if(!db.isOpen() || !db.isValid()) {
        std::cout<<"GetDevMonItem is error --------------------------------the database is interrupt"<<std::endl;
        return false;
    }

    QSqlQuery itemschquery(db);//
    QString strSql=QString("select a.MonitoringIndex,a.MonitoringName,a.Ratio,a.ItemType,a.ItemValueType,a.AlarmEnable,a.IsUpload,c.name,a.alarmtype,a.type,a.moduletype,a.moduleid \
                           ,a.snmpoid from Monitoring_Device_Item a left join data_dictionary c on c.code=a.unitstring and c.type='CompanyType'\
            where a.DeviceNumber='%1'").arg(QString::fromStdString(strDevnum));
            itemschquery.prepare(strSql);
            if(itemschquery.exec()) {
                while(itemschquery.next()) {
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
                    GetItemAlarmConfig(db,strDevnum,item.iItemIndex,iAlarmid,item.vItemAlarm);

                    item.iTargetId = itemschquery.value(9).toInt();
                    item.iModTypeId = itemschquery.value(10).toInt();
                    item.iModDevId = itemschquery.value(11).toInt();
                    item.cmdSnmpOid = itemschquery.value(12).toString().toStdString();
                    map_item[item.iItemIndex] = item;
            //cout<<"iTargetId =" <<item.iTargetId<<endl;
             }
        } else{
                cout<<itemschquery.lastError().text().toStdString()<<"GetDevMonItem---itemschquery---error!"<<endl;
                return false;
        }
        return true;
}

//获取设备属性信息
bool DataBaseOperation::GetDevProperty(QSqlDatabase &db, string strDevnum,map<string,DevProperty>& map_property ,int nChnlId)
{
    //boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    if(!db.isOpen() || !db.isValid()) {
        std::cout<<"GetDevProperty is error -------------------------------- the database is interrupt"<<std::endl;
        return false;
    }

    QSqlQuery itemschquery(db);
    QString strSql=QString("select a.BasePropertyNumber,a.PropertyValueType,a.PropertyValue,b.PropertyName,a.channelid,a.propertycompany from Device_Property_Role_Bind a,Base_Property b \
                           where a.DeviceNumber='%1' and a.channelid='%2' and b.BasePropertyNumber=a.BasePropertyNumber").arg(QString::fromStdString(strDevnum)).arg(nChnlId);
    itemschquery.prepare(strSql);
        if(itemschquery.exec()) {
            while(itemschquery.next()) {
                DevProperty dp;
                dp.property_num = itemschquery.value(0).toString().toStdString();
                dp.property_type = itemschquery.value(1).toInt();
                dp.property_value = itemschquery.value(2).toString().toStdString();
                dp.property_name = itemschquery.value(3).toString().toStdString();
                dp.property_value_unit = itemschquery.value(5).toInt();
                map_property[dp.property_name] = dp;
            }
        }else{
            cout<<itemschquery.lastError().text().toStdString()<<"GetDevProperty---itemschquery---error!"<<endl;
            return false;
        }
    return true;
}

//获取网络配置属性
//bool DataBaseOperation::GetNetProperty(QSqlDatabase &db, string strConTypeNumber,NetCommunicationMode& nmode )
//{
//    //QSqlDatabase db = ConnectionPool::openConnection();
//    if(!db.isOpen() || !db.isValid()) {//IsOpen()
//        std::cout<<"GetNetProperty is error --------------------------------- the database is interrupt"<<std::endl;
//        return false;
//    }
//    boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
//    QSqlQuery netquery(db);
//    QString strSql=QString("select NetType,IpAddress,LocalPort,PeerPort,ConnectType from Net_Communication_Mode \
//                           where CommTypeNumber='%1'").arg(QString::fromStdString(strConTypeNumber));
//            netquery.prepare(strSql);
//            if(netquery.exec()) {
//            if(netquery.next()) {
//            nmode.inet_type = netquery.value(0).toInt();
//            nmode.strIp = netquery.value(1).toString().toStdString();
//            nmode.ilocal_port = netquery.value(2).toInt();
//            nmode.iremote_port = netquery.value(3).toInt();
//            nmode.ilink_type = netquery.value(4).toInt();
//}  else
//            return false;
//}
//            else {
//            cout<<netquery.lastError().text().toStdString()<<"GetNetProperty---netquery---error!"<<endl;
//            return false;
//}
//            return true;
//}

//获取串口配置属性
//bool DataBaseOperation::GetComProperty(QSqlDatabase &db, string strConTypeNumber,ComCommunicationMode& cmode )
//{

//    //boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
//    if(!db.isOpen() || !db.isValid()) {
//        std::cout<<"GetComProperty is error --------------------------------- the database is interrupt"<<std::endl;
//        return false;
//    }

//    QSqlQuery comquery(db);
//    QString strSql=QString("select Com,Baudrate,Databit,Stopbit,Parity from Com_Communication_Mode \
//                           where CommTypeNumber='%1'").arg(QString::fromStdString(strConTypeNumber));
//            comquery.prepare(strSql);
//            if(comquery.exec()) {
//            if(comquery.next()) {
//            cmode.icomport = comquery.value(0).toInt();
//            cmode.irate = comquery.value(1).toInt();
//            cmode.idata_bit = comquery.value(2).toInt();
//            cmode.istop_bit = comquery.value(3).toInt();
//            cmode.iparity_bit = comquery.value(4).toInt();
//}
//            else
//            return false;
//} else  {
//            cout<<comquery.lastError().text().toStdString()<<"GetComProperty---comquery---error!"<<endl;
//            return false;
//}
//            return true;
//}

//获取联动动作参数
bool DataBaseOperation::GetLinkActionParam(QSqlDatabase &db, string strParamnum,map<int,vector<ActionParam> >& map_Params)
{
    //boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    //QSqlDatabase db = ConnectionPool::openConnection();
    if(!db.isOpen() || !db.isValid()) {
        std::cout<<"GetLinkActionParam is error --------------------------------- the database is interrupt"<<std::endl;
        return false;
    }

    QSqlQuery actpquery(db);
    QString strSql=QString("select a.contentindex,a.content,a.contenttype from Action_Parameter_Bind_Content a \
                           where a.ParameterNumber='%1' order by a.contentindex"
            ).arg(QString::fromStdString(strParamnum));
    actpquery.prepare(strSql);
    if(actpquery.exec()){
        while(actpquery.next()){
            ActionParam aparam;
            aparam.strParamValue = actpquery.value(1).toString().toStdString();
            aparam.iParamType = actpquery.value(2).toInt();
            map_Params[actpquery.value(0).toInt()].push_back(aparam);
        }
    }
    else{
        cout<<actpquery.lastError().text().toStdString()<<"GetLinkActionParam---actpquery---error!"<<endl;
        return false;
    }
    return true;
}

//获取联动动作配置
bool DataBaseOperation::GetLinkAction(QSqlDatabase &db, string strLinkRolenum,vector<LinkAction>& vLinkAction )
{

    //boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    if(!db.isOpen() || !db.isValid()) {
        std::cout<<"GetLinkAction is error ---------------------------------- the database is interrupt"<<std::endl;
        return false;
    }

    QSqlQuery actquery(db);
    QString strSql=QString("select b.ActionNumber,b.ActionName,b.ActionType,b.IsParam,b.ParameterNumber from Linkage_Role_Bind_Action a,Action b \
                           where a.LinkageRoleNumber='%1' and b.ActionNumber=a.ActionNumber").arg(QString::fromStdString(strLinkRolenum));
    actquery.prepare(strSql);
    if(actquery.exec()) {
        while(actquery.next())  {
            LinkAction laction;
            laction.strActionNum = actquery.value(0).toString().toStdString();
            laction.strActionNam = actquery.value(1).toString().toStdString();
            laction.iActionType = actquery.value(2).toInt();
            laction.iIshaveParam = actquery.value(3).toInt();
            if(!GetLinkActionParam(db,actquery.value(4).toString().toStdString(),laction.map_Params))
                laction.iIshaveParam = 0;
            vLinkAction.push_back(laction);
        }
    }
    else  {
            cout<<actquery.lastError().text().toStdString()<<"GetLinkAction---actquery---error!"<<endl;
            return false;
    }
    return true;
}

//获得关联设备信息
bool DataBaseOperation::GetAssDevChan(QSqlDatabase &db, string strDevNum,map<int,vector<AssDevChan> >& mapAssDev )
{
    //boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    if(!db.isOpen() || !db.isValid()) {
        std::cout<<"GetAssDevChan is error ----------------------------- the database is interrupt"<<std::endl;
        return false;
    }

    QSqlQuery itemschquery(db);
    QString strSql=QString("select objectnumberb,channelnumberb,associatetype,channelnumbera from associate_object  where objectnumbera='%1'").arg(QString::fromStdString(strDevNum));
    itemschquery.prepare(strSql);
    if(itemschquery.exec()){
        while(itemschquery.next()){
            AssDevChan ac;
            ac.sAstNum = itemschquery.value(0).toString().toStdString();
            ac.iChannel = itemschquery.value(1).toInt();
            ac.iAssType = itemschquery.value(2).toInt();
            mapAssDev[itemschquery.value(3).toInt()].push_back(ac);
        }
    }
    else{
        cout<<itemschquery.lastError().text().toStdString()<<"GetAssDevChan---itemschquery---error!"<<endl;
        return false;
    }
    return true;
}

//获得设备整机告警配置
bool DataBaseOperation::GetAlarmConfig(QSqlDatabase &db, string strDevnum,map<int,Alarm_config>& map_Alarmconfig )
{
    //boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    if(!db.isOpen() || !db.isValid()) {
        std::cout<<"GetAlarmConfig is error ----------------------------- the database is interrupt"<<std::endl;
        return false;
    }

    QSqlQuery alarmconfigquery(db);
    QString strSql=QString("select a.MonitoringIndex,a.LimitValue,a.AlarmLevel,a.JumpLimitType,a.LinkageEnable,a.LinkageRoleNumber,a.delaytime,a.LinkageRoleNumber,a.alarmconfigtype \
                           ,a.resumeduration  from Alarm_Item_config a,device_alarm_switch b where a.DeviceNumber='%1' and b.alarmenable>0 and a.alarmconfigtype<>0 and b.devicenumber=a.DeviceNumber and \
            b.alarmswitchtype=a.MonitoringIndex and a.alarmenable>0 ").arg(QString::fromStdString(strDevnum));
    alarmconfigquery.prepare(strSql);
    if(alarmconfigquery.exec()) {
            while(alarmconfigquery.next()){
                Alarm_config acfig;
                int iItemid = alarmconfigquery.value(0).toInt();
                acfig.fLimitvalue = alarmconfigquery.value(1).toDouble();
                acfig.iAlarmlevel = alarmconfigquery.value(2).toInt();
                acfig.iLimittype = alarmconfigquery.value(3).toInt();
                acfig.iLinkageEnable = alarmconfigquery.value(4).toInt();
                if(acfig.iLinkageEnable>0)
                GetLinkAction(db,alarmconfigquery.value(5).toString().toStdString(),acfig.vLinkAction);

                acfig.iDelaytime = alarmconfigquery.value(6).toInt();
                acfig.strLinkageRoleNumber = alarmconfigquery.value(7).toString().toStdString();
                acfig.iAlarmtype = alarmconfigquery.value(8).toInt();//0:监控量 1:整机
                acfig.iResumetime = alarmconfigquery.value(9).toInt();
                acfig.iAlarmid = iItemid;
                map_Alarmconfig[iItemid] = acfig;
        }
    } else {
            cout<<alarmconfigquery.lastError().text().toStdString()<<"GetAlarmConfig---alarmconfigquery---error!"<<endl;
            return false;
    }
    return true;
}

//获得监控量告警配置信息
bool DataBaseOperation::GetItemAlarmConfig(QSqlDatabase &db, string strDevnum,int iIndex,int iThid,vector<Alarm_config>& vAlarmconfig)
{

    //boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    if(!db.isOpen() || !db.isValid()) {
        std::cout<<"GetItemAlarmConfig is error ------------------------- the database is interrupt"<<std::endl;
        return false;
    }

    QSqlQuery alarmconfigquery(db);
    QString strSql=QString("select a.LimitValue,a.AlarmLevel,a.JumpLimitType,a.LinkageEnable,a.LinkageRoleNumber,a.delaytime,a.LinkageRoleNumber,a.alarmconfigtype \
                           ,a.resumeduration  from Alarm_Item_config a ,device_alarm_switch b where a.DeviceNumber='%1' and a.monitoringindex=%2 and a.alarmconfigtype=0 and b.alarmenable>0 \
            and b.devicenumber=a.DeviceNumber and b.alarmswitchtype=a.MonitoringIndex and a.alarmenable>0").arg(QString::fromStdString(strDevnum)).arg(iIndex);
    alarmconfigquery.prepare(strSql);
    if(alarmconfigquery.exec()){
        while(alarmconfigquery.next()) {
            Alarm_config acfig;
            acfig.fLimitvalue = alarmconfigquery.value(0).toDouble();
            acfig.iAlarmlevel = alarmconfigquery.value(1).toInt();
            acfig.iLimittype = alarmconfigquery.value(2).toInt();
            acfig.iLinkageEnable = alarmconfigquery.value(3).toInt();
            if(acfig.iLinkageEnable>0)
                GetLinkAction(db,alarmconfigquery.value(4).toString().toStdString(),acfig.vLinkAction);
            acfig.iDelaytime = alarmconfigquery.value(5).toInt();
            acfig.strLinkageRoleNumber = alarmconfigquery.value(6).toString().toStdString();
            acfig.iAlarmtype = alarmconfigquery.value(7).toInt();//0:监控量 1:整机
            acfig.iAlarmid = iThid;
            acfig.iResumetime = alarmconfigquery.value(8).toInt();
            vAlarmconfig.push_back(acfig);
        }
    } else {
            cout<<alarmconfigquery.lastError().text().toStdString()<<"GetItemAlarmConfig---alarmconfigquery---error!"<<endl;
            return false;
    }

    return true;
}

//设置监控使能
bool DataBaseOperation::SetEnableMonitor( string strDevnum,int iItemIndex,bool bEnabled/*=true*/ )
{
    boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);

    QSqlDatabase db = ConnectionPool::openConnection();
    if(!db.isOpen() || !db.isValid()) {//IsOpen()
        std::cout<<"SetEnableMonitor is error -------------------------------- the database is interrupt"<<std::endl;
        return false;
    }

    QSqlQuery qquery(db);
    QString strSql=QString("update alarm_item_config set alarmenable=%1 where DeviceNumber='%2' and MonitoringIndex=%3").arg(bEnabled).arg(QString::fromStdString(strDevnum)).arg(iItemIndex);
    qquery.prepare(strSql);
    if(!qquery.exec()){
        cout<<qquery.lastError().text().toStdString()<<"SetEnableMonitor---qquery---error!"<<endl;
        ConnectionPool::closeConnection(db);
        return false;
    }
    ConnectionPool::closeConnection(db);
    return true;
}

//更新某个监控量
bool DataBaseOperation::UpdateMonitorItem( string strDevnum,DeviceMonitorItem ditem )
{
    boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);

    QSqlDatabase db = ConnectionPool::openConnection();
    if(!db.isOpen() || !db.isValid()) {//IsOpen()
        std::cout<<"UpdateMonitorItem is error -------------------------------- the database is interrupt"<<std::endl;
        return false;
    }

    QSqlQuery qquery(db);
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
    if(!qquery.exec()){
        cout<<qquery.lastError().text().toStdString()<<"UpdateMonitorItem---qquery---error!"<<endl;
        return false;
    }
    return true;
}

//更新所有监控量
bool DataBaseOperation::UpdateMonitorItems( string strDevnum,vector<DeviceMonitorItem> v_ditem )
{

    boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    QSqlDatabase db = ConnectionPool::openConnection();
    if(!db.isOpen() || !db.isValid()) {
        std::cout<<"UpdateMonitorItems is error -------------------------------- the database is interrupt"<<std::endl;
        return false;
    }


    QSqlQuery qquery(db);
    QString strSql=QString("update Monitoring_Device_Item set MonitoringName=:MonitoringName,Ratio=:Ratio,ItemType=:ItemType,ItemValueType=:ItemValueType,\
                           AlarmEnable=:AlarmEnable,IsUpload=:IsUpload,UnitString=:UnitString where DeviceNumber=:DeviceNumber and MonitoringIndex=:MonitoringIndex");
            qquery.prepare(strSql);

            vector<DeviceMonitorItem>::iterator iter = v_ditem.begin();
    db.transaction();
    for(;iter!=v_ditem.end();++iter){
        qquery.bindValue(":MonitoringName",QString::fromStdString((*iter).sItemName));
        qquery.bindValue(":Ratio",(*iter).dRatio);
        qquery.bindValue(":ItemType",(*iter).iItemType);
        qquery.bindValue(":ItemValueType",(*iter).iItemvalueType);
        qquery.bindValue(":AlarmEnable",(*iter).bAlarmEnable);
        qquery.bindValue(":IsUpload",(*iter).bUpload);
        qquery.bindValue(":UnitString",QString::fromStdString((*iter).sUnit));
        qquery.bindValue(":DeviceNumber",QString::fromStdString(strDevnum));
        qquery.bindValue(":MonitoringIndex",(*iter).iItemIndex);
        if(!qquery.exec()) {
            cout<<qquery.lastError().text().toStdString()<<"UpdateMonitorItems---qquery---error!"<<endl;
            db.rollback();
            ConnectionPool::closeConnection(db);
            return false;
        }
    }

    db.commit();
    ConnectionPool::closeConnection(db);
    return true;
}

//更新告警配置项
bool DataBaseOperation::UpdateItemAlarmConfig( string strDevnum,int iIndex,Alarm_config alarm_config )
{
    boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    QSqlDatabase db = ConnectionPool::openConnection();
    if(!db.isOpen() || !db.isValid()) {
        std::cout<<"UpdateItemAlarmConfig is error -------------------------------- the database is interrupt"<<std::endl;
        return false;
    }

    QSqlQuery qquery(db);
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
    if(!qquery.exec()){
        cout<<qquery.lastError().text().toStdString()<<"UpdateItemAlarmConfig---qquery---error!"<<endl;
        ConnectionPool::closeConnection(db);
        return false;
    }
    ConnectionPool::closeConnection(db);
    return true;
}

//更新告警项配置
bool DataBaseOperation::UpdateItemAlarmConfigs( string strDevnum,map<int,Alarm_config> mapAlarmConfig )
{

    boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    QSqlDatabase db = ConnectionPool::openConnection();
    if(!db.isOpen() || !db.isValid()) {
        std::cout<<"UpdateItemAlarmConfigs is error ---------------------------- the database is interrupt"<<std::endl;
        return false;
    }

    QSqlQuery qquery(db);
    QString strSql=QString("update Monitoring_Device_Item set LimitValue=:LimitValue,AlarmLevel=:AlarmLevel,JumpLimitType=:JumpLimitType,LinkageEnable=:LinkageEnable,\
                           LinkageRoleNumber=:LinkageRoleNumber where DeviceNumber=:DeviceNumber and MonitoringIndex=:MonitoringIndex");
            qquery.prepare(strSql);
            map<int,Alarm_config>::iterator iter = mapAlarmConfig.begin();
    db.transaction();
    for(;iter!=mapAlarmConfig.end();++iter){
        qquery.bindValue(":LimitValue",(*iter).second.fLimitvalue);
        qquery.bindValue(":AlarmLevel",(*iter).second.iAlarmlevel);
        qquery.bindValue(":JumpLimitType",(*iter).second.iLimittype);
        qquery.bindValue(":LinkageEnable",(*iter).second.iLinkageEnable);
        qquery.bindValue(":LinkageRoleNumber",QString::fromStdString((*iter).second.strLinkageRoleNumber));
        qquery.bindValue(":DeviceNumber",QString::fromStdString(strDevnum));
        qquery.bindValue(":MonitoringIndex",(*iter).first);
        if(!qquery.exec()){
            cout<<qquery.lastError().text().toStdString()<<"UpdateItemAlarmConfigs---qquery---error!"<<endl;
            db.rollback();
            ConnectionPool::closeConnection(db);
            return false;
        }
    }
    db.commit();
    ConnectionPool::closeConnection(db);
    return true;
}

//添加告警开始记录
bool DataBaseOperation::AddItemAlarmRecord( string strDevnum,time_t startTime,int nMonitoringIndex,int nlimitType,int nalarmTypeId,double dValue,
                                            const string &sreason,unsigned long long& irecordid,int alarmlevel )
{

    boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    QSqlDatabase db = ConnectionPool::openConnection();
    if(!db.isOpen() || !db.isValid()) {//IsOpen()
        std::cout<<"AddItemAlarmRecord is error ---------------------- the database is interrupt"<<std::endl;
        return false;
    }

    QSqlQuery inquery(db);
    QString strSql=QString("insert into device_alarm_record(devicenumber,monitoringindex,alarmstarttime,limittype,alarmvalue,alarmtypeid,alarmreason,alarmlevel) values(:devicenumber,:monitoringindex,\
                           :alarmstarttime,:limittype,:alarmvalue,:alarmtypeid,:alarmreason,:alarmlevel)");
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
    inquery.bindValue(":alarmlevel",alarmlevel);
    if(!inquery.exec()){
        cout<<inquery.lastError().text().toStdString()<<"AddItemAlarmRecord---inquery---error!"<<endl;
        ConnectionPool::closeConnection(db);
        return false;
    }
    strSql=QString("select max(id) from device_alarm_record");
    inquery.prepare(strSql);
    if(inquery.exec()){
        if(inquery.next()){
            irecordid = inquery.value(0).toULongLong();
        }
    }else{
        cout<<inquery.lastError().text().toStdString()<<"AddItemAlarmRecord---inquery2---error!"<<endl;
    }
    ConnectionPool::closeConnection(db);
    return  true;
}
//添加告警结束记录
bool DataBaseOperation::AddItemEndAlarmRecord( time_t endTime,unsigned long long irecordid )
{
    boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    QSqlDatabase db = ConnectionPool::openConnection();
    if(!db.isOpen() || !db.isValid()) {
        std::cout<<"AddItemEndAlarmRecord is error ------------------the database is interrupt"<<std::endl;
        return false;
    }

    QSqlQuery inquery(db);
    QString strSql=QString("update device_alarm_record set alarmendtime=:alarmendtime where id=:id ");//:alarmendtime,
    inquery.prepare(strSql);
    inquery.bindValue(":id",irecordid);

    QDateTime qdt;
    tm *ltime = localtime(&endTime);
    qdt.setDate(QDate(ltime->tm_year+1900,ltime->tm_mon+1,ltime->tm_mday));
    qdt.setTime(QTime(ltime->tm_hour,ltime->tm_min,ltime->tm_sec));
    inquery.bindValue(":alarmendtime",qdt);
    if(!inquery.exec()){
        cout<<inquery.lastError().text().toStdString()<<"AddItemEndAlarmRecord---inquery---error!"<<endl;
        ConnectionPool::closeConnection(db);
        return false;
    }

    ConnectionPool::closeConnection(db);
    return  true;
}

//添加抄表记录
bool DataBaseOperation::AddItemMonitorRecord( string strDevnum,time_t savetime,DevMonitorDataPtr pdata,const map<int,DeviceMonitorItem> &mapMonitorItem)
{
    //QTime startTime = QTime::currentTime();
    //string stempTm = startTime.toString("hh:mm:ss").toStdString();
    //cout<<stempTm<<"-------AddItemMonitorRecord  enter deviceid = "<<strDevnum<<endl;
    boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    QSqlDatabase db = ConnectionPool::openConnection();
    if(!db.isOpen() || !db.isValid()) {
        std::cout<<"AddItemMonitorRecord is error  ------------ the database is interrupt"<<std::endl;
        return false;
    }

    //cout<<stempTm<<"%%%%%%%  get db object ok------------deviceid = "<<strDevnum<<endl;


    QSqlQuery inquery(db);
    QString strSql=QString("insert into device_monitoring_record(devicenumber,monitoringindex,monitoringtime,monitoringvalue) values(:devicenumber,:monitoringindex,\
                           :monitoringtime,:monitoringvalue)");
            inquery.prepare(strSql);
    QString qstrNum = QString::fromStdString(strDevnum);
    inquery.bindValue(":devicenumber",qstrNum);
    QDateTime qdt;
    tm *ltime = localtime(&savetime);
    qdt.setDate(QDate(ltime->tm_year+1900,ltime->tm_mon+1,ltime->tm_mday));
    qdt.setTime(QTime(ltime->tm_hour,ltime->tm_min,ltime->tm_sec));

    map<int,DataInfo>::iterator iter = pdata->mValues.begin();
    db.transaction();

    for(;iter!=pdata->mValues.end();++iter){

        if(mapMonitorItem.find(iter->first)==mapMonitorItem.end())
            continue;

        inquery.bindValue(":monitoringindex",(*iter).first);

        inquery.bindValue(":monitoringtime",qdt);

        inquery.bindValue(":monitoringvalue",(*iter).second.fValue);
        if(!inquery.exec()){
            cout<<inquery.lastError().text().toStdString()<<"AddItemMonitorRecord---inquery---error!"<<endl;
            db.rollback();
            ConnectionPool::closeConnection(db);
            return false;
        }
    }
    // cout<<stempTm<<"^^^^^^^  start commit------------deviceid = "<<strDevnum<<endl;
    db.commit();
    ConnectionPool::closeConnection(db);
    //QTime stopTime = QTime::currentTime();
    //stempTm = stopTime.toString("hh:mm:ss").toStdString();
    //int elapsed = startTime.msecsTo(stopTime);

    //cout<<stempTm<<"+++++++AddItemMonitorRecord  leave deviceid = "<<strDevnum<<"--used time="<<elapsed<<"ms"<<endl;
    return true;
}

//改完待测2016-03-30 16:53
bool DataBaseOperation::SetEnableAlarm(map<string,vector<Alarm_Switch_Set> > &mapAlarmSwitchSet,int& resValue)
{
    boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    QSqlDatabase db = ConnectionPool::openConnection();
    if(!db.isOpen() || !db.isValid()) {
        resValue = 3;
        std::cout<<"SetEnableAlarm is error ----------------------- the database is interrupt"<<std::endl;
        return false;
    }

    map<string,vector<Alarm_Switch_Set> >::iterator iter = mapAlarmSwitchSet.begin();
    for(;iter!=mapAlarmSwitchSet.end();++iter){

        QString qsTransNum = QString::fromStdString(iter->first);
        db.transaction();
        QString strDel = QString("delete from device_alarm_switch where devicenumber='%1'").arg(qsTransNum);
        QSqlQuery qsDel(db);
        if(!qsDel.exec(strDel)) {
            cout<<"SetEnableAlarm---qsDel---error!"<<endl;
            db.rollback();
            resValue = 3;
            ConnectionPool::closeConnection(db);
            return false;
        }
        QString strResetItemAlarmEnable = QString("update monitoring_device_item set alarmenable=0 where devicenumber='%1'").arg(qsTransNum);
        QSqlQuery qsResetItem(db);
        if(!qsResetItem.exec(strResetItemAlarmEnable)) {
            cout<<"SetEnableAlarm---qsResetItem---error!"<<endl;
            db.rollback();
            resValue = 3;
            ConnectionPool::closeConnection(db);
            return false;
        }


        QSqlQuery qsInsert(db);
        QString strSql = QString("insert into device_alarm_switch(devicenumber,alarmswitchtype,alarmenable,description) values(:devicenumber,:alarmswitchtype,:alarmenable,:description)");
        qsInsert.prepare(strSql);
        qsInsert.bindValue(":devicenumber",qsTransNum);


        QString strSetItemAlarmEnable = QString("update monitoring_device_item set alarmenable=:alarmenable where devicenumber='%1'and monitoringindex=:monitoringindex").arg(qsTransNum);
        QSqlQuery qsSetItem(db);
        qsSetItem.prepare(strSetItemAlarmEnable);




        for(int i=0;i<iter->second.size();++i){
            QSqlQuery qsIect(db);
            QString qssql=QString("select monitoringindex from monitoring_device_item where devicenumber=:devicenumber and alarmtype=:alarmtype");
            qsIect.prepare(qssql);
            int itype=iter->second[i].iAlarmid;
            qsIect.bindValue(":devicenumber",qsTransNum);
            qsIect.bindValue(":alarmtype",itype);
            if(!qsIect.exec())
            {
                cout<<"SetEnableAlarm---qsInsert3---error!-----itype="<<itype<<endl;
                db.rollback();
                resValue = 3;
                ConnectionPool::closeConnection(db);
                return false;
            }

            while(qsIect.next())
            {
                itype = qsIect.value(0).toInt();
                qsInsert.bindValue(":alarmswitchtype",itype);
                qsInsert.bindValue(":alarmenable",iter->second[i].iSwtich);
                if(iter->second[i].sDes.empty()==false)
                    qsInsert.bindValue(":description",iter->second[i].sDes.empty());
                if(!qsInsert.exec()) {
                    db.rollback();
                    resValue = 3;
                    ConnectionPool::closeConnection(db);
                    return false;
                }

                qsSetItem.bindValue(":alarmenable",iter->second[i].iSwtich);
                qsSetItem.bindValue(":monitoringindex",itype);
                if(!qsSetItem.exec()) {
                    db.rollback();
                    resValue = 3;
                    ConnectionPool::closeConnection(db);
                    return false;
                }

            }

        }
        db.commit();
    }


    ConnectionPool::closeConnection(db);
    return true;
}

//改完待测2016-03-30 16:40
bool DataBaseOperation::SetAlarmLimit(map<string,vector<Alarm_config> > &mapAlarmSet,int& resValue)
{
    boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    QSqlDatabase db = ConnectionPool::openConnection();
    if(!db.isOpen() || !db.isValid()) {
        resValue = 3;
        std::cout<<"SetAlarmLimit is error --------------- the database is interrupt"<<std::endl;
        return false;
    }

    map<string,vector<Alarm_config> >::iterator iter=mapAlarmSet.begin();
    for(;iter!=mapAlarmSet.end();++iter) {
        QString qsTransNum = QString::fromStdString(iter->first);
        db.transaction();
        QSqlQuery qsDel(db);
        QString strSql=QString("delete from alarm_item_config where devicenumber=:devicenumber");
        qsDel.prepare(strSql);
        qsDel.bindValue(":devicenumber",qsTransNum);
        if(!qsDel.exec()) {
            cout<<qsDel.lastError().text().toStdString()<<"SetEnableAlarm---qsDel2---error!"<<endl;
            db.rollback();
            resValue = 3;
            ConnectionPool::closeConnection(db);
            return false;
        }

        QSqlQuery qsInsert(db);
        strSql=QString("insert into alarm_item_config(devicenumber,monitoringindex,limitvalue,jumplimittype,delaytime,resumeduration,alarmconfigtype,alarmenable) \
                       values(:devicenumber,:monitoringindex,:limitvalue,:jumplimittype,:delaytime,:resumeduration,:alarmconfigtype,1)");
                       qsInsert.prepare(strSql);
                qsInsert.bindValue(":devicenumber",qsTransNum);

        for(int i=0;i<iter->second.size();++i){

            int itype = iter->second[i].iAlarmid;
            QSqlQuery qsIect(db);
            QString qssql=QString("select monitoringindex from monitoring_device_item where devicenumber=:devicenumber and alarmtype=:alarmtype");
            qsIect.prepare(qssql);
            qsIect.bindValue(":devicenumber",qsTransNum);
            qsIect.bindValue(":alarmtype",itype);
            if(!qsIect.exec())
            {
                cout<<qsIect.lastError().text().toStdString()<<"SetEnableAlarm---qsInsert3---error!"<<endl;
                db.rollback();
                resValue = 3;
                ConnectionPool::closeConnection(db);
                return false;
            }

            if(itype!=512 && itype!=511)
                qsInsert.bindValue(":alarmconfigtype",0);
            else
                qsInsert.bindValue(":alarmconfigtype",1);


            while(qsIect.next())
            {
                itype = qsIect.value(0).toInt();

                qsInsert.bindValue(":monitoringindex",itype);
                qsInsert.bindValue(":delaytime", iter->second[i].iDelaytime);
                qsInsert.bindValue(":resumeduration", iter->second[i].iResumetime);
                qsInsert.bindValue(":jumplimittype",iter->second[i].iLimittype);
                //查找是否有Power与RePower属性(适应博汇发射机入射与反射门限百分比设定)
                string sLimitValue = "";
                if(itype==0 &&  GetInst(StationConfig).get_dev_propery(iter->first,"Power",sLimitValue)  ){
                    double dbLimit = (atof(sLimitValue.c_str()) * iter->second[i].fLimitvalue)/double(100000.0f);
                    qsInsert.bindValue(":limitvalue",dbLimit);
                }else  if(itype == 1 && GetInst(StationConfig).get_dev_propery(iter->first,"RePower",sLimitValue)){
                    double dbLimit = atof(sLimitValue.c_str()) * iter->second[i].fLimitvalue;
                    qsInsert.bindValue(":limitvalue",dbLimit);
                }else
                    qsInsert.bindValue(":limitvalue",iter->second[i].fLimitvalue);

                if(!qsInsert.exec())  {
                    cout<<qsInsert.lastError().text().toStdString()<<"SetEnableAlarm---qsInsert3---error!"<<endl;
                    db.rollback();
                    resValue = 3;
                    ConnectionPool::closeConnection(db);
                    return false;
                }
            }

        }
        db.commit();
    }
    resValue = 0;
    ConnectionPool::closeConnection(db);
    return true;
}

//设置发射机运行图
bool DataBaseOperation::SetAlarmTime( map<string,vector<Monitoring_Scheduler> > &mapSch,int& resValue)
{
    boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    QSqlDatabase db = ConnectionPool::openConnection();
    if(!db.isOpen() || !db.isValid()) {
        resValue = 3;
        std::cout<<"SetAlarmTime is error ------------------the database is interrupt"<<std::endl;
        return false;
    }

    db.transaction();
    //clear run time scheduler
    if(mapSch.size()<=0){

        QString strGetTsmt=QString("select devicenumber from device where devicetype=0");
        QSqlQuery sqlGetTsmt(db);
        if(!sqlGetTsmt.exec(strGetTsmt)) {
            cout<<sqlGetTsmt.lastError().text().toStdString()<<"SetAlarmTime---sqlGetTsmt---error!"<<endl;
            db.rollback();
            resValue = 3;
            ConnectionPool::closeConnection(db);
            return false;
        }

        while(sqlGetTsmt.next())
        {
            vector<Monitoring_Scheduler> vecSch;
            QString sDevNum = sqlGetTsmt.value(0).toString();
            QSqlQuery qsDel(db);
            QString strSql=QString("delete from monitoring_scheduler where objectnumber=:objectnumber");
            qsDel.prepare(strSql);
            qsDel.bindValue(":objectnumber",sDevNum);
            if(!qsDel.exec()){
                cout<<qsDel.lastError().text().toStdString()<<"SetAlarmTime---qsDel---error!"<<endl;
                db.rollback();
                resValue = 3;
                ConnectionPool::closeConnection(db);
                return false;
            }

            strSql = QString("delete from command_scheduler where objectnumber=:objectnumber and (commandtype=15 or commandtype=19)");
            qsDel.prepare(strSql);
            qsDel.bindValue(":objectnumber",sDevNum);
            if(!qsDel.exec()) {
                cout<<qsDel.lastError().text().toStdString()<<"SetAlarmTime---qsDel2---error!"<<endl;
                db.rollback();
                resValue = 3;
                ConnectionPool::closeConnection(db);
                return false;
            }

            mapSch[sDevNum.toStdString()] = vecSch;
        }

        db.commit();
        ConnectionPool::closeConnection(db);
        return true;
    }

    map<string,vector<Monitoring_Scheduler> >::iterator iter= mapSch.begin();
    for(;iter!=mapSch.end();++iter){
        bool needClearCtlRuntime = false;
        bool needClearMonitorRuntime = false;
        //统计是否需要清除
        vector<Monitoring_Scheduler>::iterator iter_need_clear = iter->second.begin();
        for(;iter_need_clear!=iter->second.end();++iter_need_clear){
            if((*iter_need_clear).iCtlType==0 || (*iter_need_clear).iCtlType==2)
                needClearCtlRuntime = true;
            if((*iter_need_clear).iCtlType==0 || (*iter_need_clear).iCtlType==1)
                needClearMonitorRuntime = true;
        }

        QString sDevNum = QString::fromStdString(iter->first);
        QSqlQuery qsDel(db);
        QString strSql=QString("delete from monitoring_scheduler where objectnumber=:objectnumber");
        //清理监测运行图记录
        if(needClearMonitorRuntime == true){
            qsDel.prepare(strSql);
            qsDel.bindValue(":objectnumber",sDevNum);
            if(!qsDel.exec()){
                cout<<qsDel.lastError().text().toStdString()<<"SetAlarmTime---qsDel---error!"<<endl;
                db.rollback();
                resValue = 3;
                ConnectionPool::closeConnection(db);
                return false;
            }
        }
        //清理控制运行图记录
        if(needClearCtlRuntime == true){
            strSql = QString("delete from command_scheduler where objectnumber=:objectnumber and (commandtype=15 or commandtype=19)");
            qsDel.prepare(strSql);
            qsDel.bindValue(":objectnumber",sDevNum);
            if(!qsDel.exec()) {
                cout<<qsDel.lastError().text().toStdString()<<"SetAlarmTime---qsDel2---error!"<<endl;
                db.rollback();
                resValue = 3;
                ConnectionPool::closeConnection(db);
                return false;
            }
        }


        QSqlQuery insertQuery(db);//0:星期 1:月 2:天
        strSql = QString("insert into monitoring_scheduler(objectnumber,weekday,enable,starttime,endtime,datetype,month,day,alarmendtime,mode) \
                         values(:objectnumber,:weekday,:enable,:starttime,:endtime,:datetype,:month,:day,:alarmendtime,:mode)");
        insertQuery.prepare(strSql);
        insertQuery.bindValue(":objectnumber",sDevNum);

        QString strCmd = QString("insert into command_scheduler(objectnumber,paramnumber,enable,weekday,starttime,commandtype,hasparam,datetype,month,day,commandendtime) \
                                 values(:objectnumber,'',1,:weekday,:starttime,:commandtype,0,:datetype,:month,:day,:commandendtime)");
        QSqlQuery insertCmdopenQuery(db);
        insertCmdopenQuery.prepare(strCmd);
        insertCmdopenQuery.bindValue(":objectnumber",sDevNum);
        insertCmdopenQuery.bindValue(":commandtype",15);
        QSqlQuery insertCmdcloseQuery(db);
        insertCmdcloseQuery.prepare(strCmd);
        insertCmdcloseQuery.bindValue(":objectnumber",sDevNum);
        insertCmdcloseQuery.bindValue(":commandtype",19);

        for(int i=0;i<iter->second.size();++i){
            //Command_Scheduler cmmdSch;
            int shutype = iter->second[i].iMonitorType;
            insertQuery.bindValue(":datetype",shutype);

            insertCmdopenQuery.bindValue(":datetype",shutype);
            insertCmdcloseQuery.bindValue(":datetype",shutype);

            int nEnable = (iter->second[i].bMonitorFlag==false)?0:1;
            insertQuery.bindValue(":enable",nEnable);

            int nMode = (iter->second[i].bRunModeFlag==false)?0:1;
            insertQuery.bindValue(":mode",nMode);
            QDateTime qdt = QDateTime::fromTime_t(iter->second[i].tStartTime);
            insertQuery.bindValue(":starttime",qdt);

            if(iter->second[i].bRunModeFlag==false)//bMonitorFlag
                insertCmdcloseQuery.bindValue(":starttime",qdt.addSecs(20));//不监测关机时间退后20秒
            else
                insertCmdopenQuery.bindValue(":starttime",qdt.addSecs(-20));//监测开机时间退后20秒

            qdt = QDateTime::fromTime_t(iter->second[i].tEndTime);
            insertQuery.bindValue(":endtime",qdt);
            if(iter->second[i].bRunModeFlag==true)//bMonitorFlag
                insertCmdcloseQuery.bindValue(":starttime",qdt.addSecs(20));//监测关机时间退后20秒
            else
                insertCmdopenQuery.bindValue(":starttime",qdt.addSecs(-20));//监测开机时间退后20秒

            QDateTime qAlarmendDt = QDateTime::fromTime_t(iter->second[i].tAlarmEndTime);

            insertQuery.bindValue(":weekday",iter->second[i].iMonitorWeek);
            insertQuery.bindValue(":month",iter->second[i].iMonitorMonth);
            insertQuery.bindValue(":day",iter->second[i].iMonitorDay);
            insertQuery.bindValue(":alarmendtime",qAlarmendDt);

            insertCmdcloseQuery.bindValue(":weekday",iter->second[i].iMonitorWeek);
            insertCmdcloseQuery.bindValue(":month",iter->second[i].iMonitorMonth);
            insertCmdcloseQuery.bindValue(":day",iter->second[i].iMonitorDay);
            insertCmdcloseQuery.bindValue(":commandendtime",qAlarmendDt);

            insertCmdopenQuery.bindValue(":weekday",iter->second[i].iMonitorWeek);
            insertCmdopenQuery.bindValue(":month",iter->second[i].iMonitorMonth);
            insertCmdopenQuery.bindValue(":day",iter->second[i].iMonitorDay);
            insertCmdopenQuery.bindValue(":commandendtime",qAlarmendDt);

            if(iter->second[i].iCtlType == 0 || iter->second[i].iCtlType == 1){
                if(!insertQuery.exec())  {
                    cout<<insertQuery.lastError().text().toStdString()<<"SetAlarmTime---insertQuery---error!"<<endl;
                    db.rollback();
                    resValue = 3;
                    ConnectionPool::closeConnection(db);
                    return false;
                }
            }

            if(iter->second[i].iCtlType == 0 || iter->second[i].iCtlType == 2){
                if(!insertCmdopenQuery.exec()) {
                    cout<<insertCmdopenQuery.lastError().text().toStdString()<<"SetAlarmTime---insertCmdopenQuery---error!"<<endl;
                    db.rollback();
                    resValue = 3;
                    ConnectionPool::closeConnection(db);
                    return false;
                }
                if(!insertCmdcloseQuery.exec()) {
                    cout<<insertCmdcloseQuery.lastError().text().toStdString()<<"SetAlarmTime---insertCmdcloseQuery---error!"<<endl;
                    db.rollback();
                    resValue = 3;
                    ConnectionPool::closeConnection(db);
                    return false;
                }
            }

        }

    }
    db.commit();
    ConnectionPool::closeConnection(db);
    return true;
}

bool DataBaseOperation::GetUpdateDevTimeScheduleInfo( string strDevnum,map<int,vector<Monitoring_Scheduler> >& monitorScheduler,
                                                      vector<Command_Scheduler> &cmmdScheduler  )
{
    boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    QSqlDatabase db = ConnectionPool::openConnection();
    if(!db.isOpen() || !db.isValid()) {
        std::cout<<"GetUpdateDevTimeScheduleInfo is error ------------------the database is interrupt"<<std::endl;
        return false;
    }
    GetDevMonitorSch(db,strDevnum,monitorScheduler);
    GetCmd(db,strDevnum,cmmdScheduler);

    ConnectionPool::closeConnection(db);
    return true;
}

bool DataBaseOperation::GetUpdateDevAlarmInfo( string strDevnum,DeviceInfo& device )
{
    boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    QSqlDatabase db = ConnectionPool::openConnection();
    if(!db.isOpen() || !db.isValid()) {
        std::cout<<"GetUpdateDevAlarmInfo error  ------ the database is interrupt"<<std::endl;
        return false;
    }

    QSqlQuery devquery(db);
    QString strSql=QString("select a.DeviceNumber,a.ProtocolNumber \
                           from Device a,Device_Map_Protocol b where a.DeviceNumber='%1' and b.ProtocolNumber=a.ProtocolNumber").arg(QString::fromStdString(strDevnum));
    devquery.prepare(strSql);
    if(devquery.exec())   {
        if(devquery.next())  {
            device.sDevNum = devquery.value(0).toString().toStdString();
            QString sprotoclnum = devquery.value(1).toString();
            if(GetDevMonItem(db,strDevnum,sprotoclnum,device.map_MonitorItem)==false){
                ConnectionPool::closeConnection(db);
                return false;
            }
            if(GetAlarmConfig(db,strDevnum,device.map_AlarmConfig)==false){
                        ConnectionPool::closeConnection(db);
                        return false;
            }
        }
    }else {
        std::cout<<devquery.lastError().text().toStdString()<<"GetUpdateDevAlarmInfo---query---error!"<<std::endl;
        ConnectionPool::closeConnection(db);
        return false;
    }

    ConnectionPool::closeConnection(db);

    return true;
}

//根据用户编号获取用户信息
bool DataBaseOperation::GetUserInfoByNumber(const string sNumber,UserInformation &user)
{

    boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    QSqlDatabase db = ConnectionPool::openConnection();
    if(!db.isOpen() || !db.isValid()) {
        std::cout<<"GetUserInfoByNumber error  ------ the database is interrupt"<<std::endl;
        return false;
    }

    try
    {

        QSqlQuery userquery(db);
        QString strSql=QString("select Number,Password,controllevel,Headship,JobNumber,telephone from Users where Number='%1'").arg(QString::fromStdString(sNumber));
        if(!userquery.exec(strSql)){
            cout<<userquery.lastError().text().toStdString()<<"GetUserInfoByNumber---userquery---error!"<<endl;
            ConnectionPool::closeConnection(db);
            return false;
        }
        if(userquery.next()){

            user.sNumber = userquery.value(0).toString().toStdString();
            user.sPassword = userquery.value(1).toString().toStdString();
            user.nControlLevel = userquery.value(2).toInt();
            user.sHeadship = userquery.value(3).toString().toStdString();
            user.sJobNumber = userquery.value(4).toString().toStdString();
            user.sTelephone = userquery.value(5).toString().toStdString();
        }
    }catch(...){
        ConnectionPool::closeConnection(db);
        return false;
    }
    ConnectionPool::closeConnection(db);
    return true;

}

//获取用户信息
bool DataBaseOperation::GetUserInfo( const string sName,UserInformation &user )
{
    boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    QSqlDatabase db = ConnectionPool::openConnection();
    if(!db.isOpen() || !db.isValid()) {
        std::cout<<"GetUserInfo error  ------ the database is interrupt"<<std::endl;
        return false;
    }

    try
    {
        QSqlQuery userquery(db);
        QString strSql=QString("select Number,Password,controllevel,Headship,JobNumber,rolenumber,rolename from Users,user_role where Users.rolenumber=user_role.roleid and Name='%1'").arg(QString::fromStdString(sName));
        if(!userquery.exec(strSql)){
            cout<<userquery.lastError().text().toStdString()<<"GetUserInfo---userquery---error!"<<endl;
            ConnectionPool::closeConnection(db);
            return false;
        }
        if(userquery.next()){
            user.sName = sName;
            user.sNumber = userquery.value(0).toString().toStdString();
            user.sPassword = userquery.value(1).toString().toStdString();
            user.nControlLevel = userquery.value(2).toInt();
            user.sHeadship = userquery.value(3).toString().toStdString();
            user.sJobNumber = userquery.value(4).toString().toStdString();
            user.sRoleNumber = userquery.value(5).toString().toStdString();
            user.sRoleName = userquery.value(6).toString().toStdString();
        }
    }catch(...){
        ConnectionPool::closeConnection(db);
        return false;
    }
    ConnectionPool::closeConnection(db);
    return true;
}

bool DataBaseOperation::GetAllAuthorizeDevByUser( const string sUserId,vector<string> &vDevice ,string sClientId)
{
    boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    QSqlDatabase db = ConnectionPool::openConnection();
    if(!db.isOpen() || !db.isValid()) {
        std::cout<<"GetAllAuthorizeDevByUser error  ------ the database is interrupt"<<std::endl;
        return false;
    }

    QSqlQuery query(db);
    QString strSql=QString("select a.objectnumber from user_role_object a,users b where b.number='%1' and a.rolenumber=b.rolenumber").arg(QString::fromStdString(sUserId));
    if(sClientId.compare("00000000") != 0){
        strSql = QString("select c.objectnumber from user_role_object a,users b,platform_server_purview c where c.servernumber='%1'"
                         " and b.number='%2' and a.rolenumber=b.rolenumber and a.objectnumber=c.objectnumber").arg(QString::fromStdString(sClientId)).arg(QString::fromStdString(sUserId));
    }
    if(!query.exec(strSql)){
        cout<<query.lastError().text().toStdString()<<"GetAllAuthorizeDevByUser---query---error!"<<endl;
        ConnectionPool::closeConnection(db);
        return false;
    }
    while(query.next()){
        string sdevnum;
        sdevnum = query.value(0).toString().toStdString();
        vDevice.push_back(sdevnum);
    }
    ConnectionPool::closeConnection(db);
    return true;
}


//节目信号告警/恢复
bool DataBaseOperation::AddProgramSignalAlarmRecord(string strDevNum, string strFrqName,time_t startTime,int nlimitType,
                                                    int nalarmTypeId,unsigned long long& irecordid )
{
    boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    QSqlDatabase db = ConnectionPool::openConnection();
    if(!db.isOpen() || !db.isValid()) {
        std::cout<<"AddProgramSignalAlarmRecord error  ------ the database is interrupt"<<std::endl;
        return false;
    }

    QSqlQuery inquery(db);
    QString strSql;
    if(nlimitType==ALARM_SWITCH)
    {
        strSql = QString("insert into dtmb_alarm_record(devicenumber,freq,alarmtype,alarmstarttime,alarmid) values(:devicenumer,:freq,:alarmtype,:alarmstarttime,:alarmid)");
        inquery.prepare(strSql);
        inquery.bindValue(":freq",atoi(strFrqName.c_str()));//
        inquery.bindValue(":alarmtype",nalarmTypeId);
        inquery.bindValue(":devicenumer",QString::fromStdString(strDevNum));
        inquery.bindValue(":alarmid",0);
        QDateTime qdt;
        //  tm *ltime = localtime(&startTime);
        qdt=QDateTime::fromTime_t(startTime);
        //qdt.setDate(QDate(ltime->tm_year+1900,ltime->tm_mon+1,ltime->tm_mday));
        // qdt.setTime(QTime(ltime->tm_hour,ltime->tm_min,ltime->tm_sec));
        inquery.bindValue(":alarmstarttime",qdt);
        if(!inquery.exec()){
            cout<<inquery.lastError().text().toStdString()<<"AddItemAlarmRecord---inquery---error!"<<endl;
            ConnectionPool::closeConnection(db);
            return false;
        }
        strSql=QString("select max(id) from dtmb_alarm_record");
        inquery.prepare(strSql);
        if(inquery.exec()){
            if(inquery.next()){
                irecordid = inquery.value(0).toULongLong();
            }
        }else{
            cout<<inquery.lastError().text().toStdString()<<"AddItemAlarmRecord---inquery2---error!"<<endl;
        }
        ConnectionPool::closeConnection(db);
        return  true;
    }
    else if(nlimitType==ALARM_RESUME)
    {
        strSql = QString("update dtmb_alarm_record set alarmendtime=:alarmendtime where alarmid=:id");
        inquery.prepare(strSql);
        QDateTime qdt;
        tm *ltime = localtime(&startTime);
        qdt.setDate(QDate(ltime->tm_year+1900,ltime->tm_mon+1,ltime->tm_mday));
        qdt.setTime(QTime(ltime->tm_hour,ltime->tm_min,ltime->tm_sec));
        inquery.bindValue(":alarmendtime",qdt);
        inquery.bindValue(":id",irecordid);
        if(!inquery.exec()){
            cout<<inquery.lastError().text().toStdString()<<"AddItemAlarmRecord---inquery---error!"<<endl;
            ConnectionPool::closeConnection(db);
            return false;
        }
    }

    ConnectionPool::closeConnection(db);
    return true;

}

bool DataBaseOperation::GetGSMInfo(bool &bhave, ComCommunicationMode &mdInfo)
{
    boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    QSqlDatabase db = ConnectionPool::openConnection();
    if(!db.isOpen() || !db.isValid()) {
        std::cout<<"GetGSMInfo error  ------ the database is interrupt"<<std::endl;
        return false;
    }

    QSqlQuery selectquery(db);
    QString strSql;
    strSql = QString("select devicenumber from device where devicetype=300");
    if(!selectquery.exec(strSql))
    {
        ConnectionPool::closeConnection(db);
        return false;
    }
    if(selectquery.next())
    {

        QString strDevNum = selectquery.value(0).toString();
        QSqlQuery infoquery(db);
        strSql = QString("select * from com_communication_mode where commtypenumber in (select commtypenumber from device_bind_comm where devicenumber='%1')").arg(strDevNum);
        if(!infoquery.exec(strSql))
        {
            bhave = false;
        }
        if(infoquery.next())
        {
            mdInfo.icomport = infoquery.value(2).toInt();
            mdInfo.irate = infoquery.value(4).toInt();
            mdInfo.idata_bit = infoquery.value(5).toInt();
            mdInfo.istop_bit = infoquery.value(6).toInt();
            mdInfo.iparity_bit = infoquery.value(7).toInt();
            bhave = true;
        }
        else
            bhave = false;
    }
    else
    {
        bhave = false;
    }
    ConnectionPool::closeConnection(db);
    return true;
}


bool DataBaseOperation::GetAllUserInfoByStation(const string sStationNumber, LoginAck &users)
{
    boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    QSqlDatabase db = ConnectionPool::openConnection();
    if(!db.isOpen() || !db.isValid()) {
        std::cout<<"GetAllUserInfoByStation error  ------ the database is interrupt"<<std::endl;
        return false;
    }

    QSqlQuery selectquery(db);
    QString strSql;
    strSql = QString("select number,name,headship,jobnumber,controllevel,rolenumber,rolename from users,user_role where users.rolenumber=user_role.roleid and number in(select objectnumber from station_bind_object where objecttype=1 and stationnumber='%1')").arg(QString::fromStdString(sStationNumber));
    if(!selectquery.exec(strSql))
    {
        ConnectionPool::closeConnection(db);
        return false;
    }
    while(selectquery.next())
    {
        string sNumber,sName,sHeadship,sJobNumber,sRoleNumber,sRoleName;
        int nLevel;
        sNumber = selectquery.value(0).toString().toStdString();
        sName = selectquery.value(1).toString().toStdString();
        sHeadship = selectquery.value(2).toString().toStdString();

        sJobNumber = selectquery.value(3).toString().toStdString();
        nLevel = selectquery.value(4).toInt();
        sRoleNumber = selectquery.value(5).toString().toStdString();
        sRoleName = selectquery.value(6).toString().toStdString();

        UserInfo *pUser = users.add_alluserinfo();
        pUser->set_usrname(sName.c_str());
        pUser->set_usrnumber(sNumber.c_str());
        pUser->set_eusrlevel(nLevel);
        pUser->set_usrheadship(sHeadship.c_str());
        pUser->set_usrjobnumber(sJobNumber.c_str());
        pUser->set_usrrolenumber(sRoleNumber);
        pUser->set_usrrolename(sRoleName);
    }
    ConnectionPool::closeConnection(db);
    return true;
}

bool DataBaseOperation::AddDutyLog(const string sUserNumber, const string sContent, int nType)
{
    boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    QSqlDatabase db = ConnectionPool::openConnection();
    if(!db.isOpen() || !db.isValid()) {
        std::cout<<"AddDutyLog error  ------ the database is interrupt"<<std::endl;
        return false;
    }

    QSqlQuery insertquery(db);
    QString strSql;
    strSql = QString("insert into duty_log(usernumber,time,content,dutylogtype) values(:usernumber,:time,:content,:dutylogtype)");
    QDateTime qdt=QDateTime::currentDateTime();
    insertquery.prepare(strSql);
    insertquery.bindValue(":usernumber",QString::fromStdString(sUserNumber));
    insertquery.bindValue(":time",qdt);
    insertquery.bindValue(":content",QString::fromStdString(sContent));
    insertquery.bindValue(":dutylogtype",nType);
    if(!insertquery.exec()){
        cout<<insertquery.lastError().text().toStdString()<<"AddDutyLog---inquery---error!"<<endl;
        ConnectionPool::closeConnection(db);
        return false;
    }
    ConnectionPool::closeConnection(db);
    return true;
}

bool DataBaseOperation::AddHandove(const string sHandoveNumber, const string sSuccessorNumber, const string &sLogContents, const time_t ttime)
{
    boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    QSqlDatabase db = ConnectionPool::openConnection();
    if(!db.isOpen() || !db.isValid()) {
        std::cout<<"AddHandove error  ------ the database is interrupt"<<std::endl;
        return false;
    }

    QSqlQuery insertquery(db);
    QString strSql = QString("insert into user_handove(handoveperson,successors,successiontime,handovelog) values(:handoveperson,:successors,:successiontime,:handovelog)");
    insertquery.prepare(strSql);
    insertquery.bindValue(":handoveperson",QString::fromStdString(sHandoveNumber));
    insertquery.bindValue(":successors",QString::fromStdString(sSuccessorNumber));
    insertquery.bindValue(":successiontime",QDateTime::fromTime_t(ttime));
    insertquery.bindValue(":handovelog",QString::fromStdString(sLogContents));
    if(!insertquery.exec()){
        cout<<insertquery.lastError().text().toStdString()<<"AddHandove---inquery---error!"<<endl;
        ConnectionPool::closeConnection(db);
        return false;
    }
    ConnectionPool::closeConnection(db);
    return true;
}

bool DataBaseOperation::AddSignin(const string sSignerNumber, const time_t ttime,int nSigntype)
{
    boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    QSqlDatabase db = ConnectionPool::openConnection();
    if(!db.isOpen() || !db.isValid()) {
        std::cout<<"AddSignin error  ------ the database is interrupt"<<std::endl;
        return false;
    }

    QSqlQuery insertquery(db);
    QString strSql;

    strSql = QString("insert into signstatus(userid,signinstatus,signintime,signintype) values(:userid,:signinstatus,:signintime,:signintype)");
    insertquery.prepare(strSql);
    insertquery.bindValue(":userid",QString::fromStdString(sSignerNumber));
    insertquery.bindValue(":signinstatus",1);
    insertquery.bindValue(":signintime",QDateTime::fromTime_t(ttime));
    insertquery.bindValue(":signintype",nSigntype);
    if(!insertquery.exec()){
        cout<<insertquery.lastError().text().toStdString()<<"AddSignin---inquery---error!"<<endl;
        ConnectionPool::closeConnection(db);
        return false;
    }
    ConnectionPool::closeConnection(db);
    return true;
}

bool DataBaseOperation::AddSignout(const string sSignerNumber, const time_t tintime, const time_t touttime)
{
    boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    QSqlDatabase db = ConnectionPool::openConnection();
    if(!db.isOpen() || !db.isValid()) {//IsOpen()
        std::cout<<"AddSignout error  ------ the database is interrupt"<<std::endl;
        return false;
    }

    QSqlQuery updatequery(db);
    QString strSql;
    strSql = QString("update signstatus set sgnouttime=:sgnouttime where userid=:userid and signintime=:signintime");
    updatequery.prepare(strSql);
    updatequery.bindValue(":sgnouttime",QDateTime::fromTime_t(touttime));
    updatequery.bindValue(":signintime",QDateTime::fromTime_t(tintime));
    updatequery.bindValue(":userid",QString::fromStdString(sSignerNumber));
    if(!updatequery.exec())
    {
        cout<<updatequery.lastError().text().toStdString()<<"AddSignout---update---error!"<<endl;
        ConnectionPool::closeConnection(db);
        return false;
    }
    ConnectionPool::closeConnection(db);
    return true;
}

//添加命令执行日志
bool DataBaseOperation::AddExcuteCommandLog(const string sDevNum,int nCommandType,
                                            const string sResult,const string sUser)
{
    boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    QSqlDatabase db = ConnectionPool::openConnection();
    if(!db.isOpen() || !db.isValid()) {
        std::cout<<"AddExcuteCommandLog error  ------ the database is interrupt"<<std::endl;
        return false;
    }

    time_t tmTime = time(0);

    QSqlQuery insertquery(db);
    QString strSql;

    int  nMode = 0;
    if(sUser == "timer")
        nMode = 1;
    else if(sUser == "auto")
        nMode = 2;


    strSql = QString("insert into device_operation_record(devicenumber,operation,operationtime,"
                     "operationresult,operationuser,operationcommand) values(:devid,:opermod,:opertm,:operrslt,:operuser,:opercmmd)");
    insertquery.prepare(strSql);
    insertquery.bindValue(":devid",QString::fromStdString(sDevNum));
    insertquery.bindValue(":opermod",nMode);
    insertquery.bindValue(":opertm",QDateTime::fromTime_t(tmTime));
    insertquery.bindValue(":operrslt",QString::fromStdString(sResult));
    insertquery.bindValue(":operuser",QString::fromStdString(sUser));
    insertquery.bindValue(":opercmmd",nCommandType);
    if(!insertquery.exec()){
        cout<<insertquery.lastError().text().toStdString()<<"AddExcuteCommandLog---inquery---error!"<<endl;
        ConnectionPool::closeConnection(db);
        return false;
    }

    ConnectionPool::closeConnection(db);
    return true;

}

//获取提醒事件配置信息
bool DataBaseOperation::GetRemindInfoByServer(const string sServerNumber,map<string,Remind_Scheduler> &remindSch)
{
    boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    QSqlDatabase db = ConnectionPool::openConnection();
    if(!db.isOpen() || !db.isValid()) {
        std::cout<<"GetRemindInfoByServer error  ------ the database is interrupt"<<std::endl;
        return false;
    }
    QSqlQuery remindSchquery(db);

    QString strSql=QString("select remindnumber,remindtype,datetype,weekday,time,month,day,agentserver,originator,targetobject"
                            ",remindcontent,needconfirm,confirmtimeout,advanceseconds from remind_notify_scheduler where agentserver='%1' union all"
                            " select remindnumber,remindtype,datetype,weekday,time,month,day,agentserver,originator,targetobject"
                            ",remindcontent,needconfirm,confirmtimeout,advanceseconds from remind_notify_scheduler where remindnumber in(select remindnumber "
                            "from command_scheduler where objectnumber in(select objectnumber from platform_server_purview"
                            " where servernumber='%2'))").arg(QString::fromStdString(sServerNumber)).arg(QString::fromStdString(sServerNumber));
    remindSchquery.prepare(strSql);
    if(remindSchquery.exec()){
        while(remindSchquery.next()){
            Remind_Scheduler  remind_sch;
            remind_sch.sRemindNumber = remindSchquery.value(0).toString().toStdString();
            remind_sch.iRemindType = remindSchquery.value(1).toInt();
            remind_sch.iDateType = remindSchquery.value(2).toInt();
            QString weekDay =  remindSchquery.value(3).toString();
            QStringList weekList = weekDay.replace("{","").replace("}","").split(",");
            for(int i = 0; i< weekList.size();++i)
               remind_sch.vWeek.push_back(weekList.at(i).toInt());
            remind_sch.tExecuteTime = remindSchquery.value(4).toDateTime().toTime_t();
            remind_sch.iMonth = remindSchquery.value(5).toInt();
            remind_sch.iDay = remindSchquery.value(6).toInt();
            remind_sch.sAgentServerNumber = remindSchquery.value(7).toString().toStdString();
            remind_sch.iOriginator = remindSchquery.value(8).toInt();
            remind_sch.iTargetObject = remindSchquery.value(9).toInt();
            remind_sch.sRemindContent = remindSchquery.value(10).toString().toStdString();
            remind_sch.bNeedConfirm = remindSchquery.value(11).toBool();
            remind_sch.iConfirmTimeout = remindSchquery.value(12).toInt();
            remind_sch.iAdvanceSeconds = remindSchquery.value(13).toInt();

            remindSch[remind_sch.sRemindNumber] = remind_sch;
        }
    }
    else{
        cout<<remindSchquery.lastError().text().toStdString()<<"GetRemindInfoByServer---remindSchquery---error!"<<endl;
        return false;
    }
    return true;
}

//添加提醒时间触发日志
bool DataBaseOperation::AddRemindItemLog(const string sRemindNumber,int remindtype,const string sConfirmUser,string sConfirmMessage,
                      int nConfirmState,const time_t notifyTime ,int &newId)
{
    boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    QSqlDatabase db = ConnectionPool::openConnection();
    if(!db.isOpen() || !db.isValid()) {
        std::cout<<"AddRemindItemLog error  ------ the database is interrupt"<<std::endl;
        return false;
    }
    QSqlQuery insertRemindQuery(db);


    QString strSql = QString("insert into remind_notify_record(remindnumber,remindtime,"
                     "confirmuser,confirmstate,remindtype,confirmcontent) values(:remindnumber,"
                     ":remindtime,:confirmuser,:confirmstate,:remindtype,:confirmcontent)");
    insertRemindQuery.prepare(strSql);
    insertRemindQuery.bindValue(":remindnumber",QString::fromStdString(sRemindNumber));
    insertRemindQuery.bindValue(":remindtype",remindtype);
    insertRemindQuery.bindValue(":confirmstate",nConfirmState);
    insertRemindQuery.bindValue(":remindtime",QDateTime::fromTime_t(notifyTime));
    //insertRemindQuery.bindValue(":confirmtime",QDateTime::fromTime_t(confirmTime));//confirmtime,
    insertRemindQuery.bindValue(":confirmuser",QString::fromStdString(sConfirmUser));
    insertRemindQuery.bindValue(":confirmcontent",QString::fromStdString(sConfirmMessage));

    if(!insertRemindQuery.exec()){
        cout<<insertRemindQuery.lastError().text().toStdString()<<"AddRemindItemLog---inquery---error!"<<endl;
        ConnectionPool::closeConnection(db);
        return false;
    }


    newId = insertRemindQuery.lastInsertId().toInt();
    QString queryMaxId = QString("select currval('remind_notify_record_seq')");
    insertRemindQuery.prepare(queryMaxId);//nextval
    if(insertRemindQuery.exec()){
        while(insertRemindQuery.next()){

            newId = insertRemindQuery.value(0).toInt();
        }
    }

    ConnectionPool::closeConnection(db);
    return true;
}

//更新提醒时间触发日志
bool DataBaseOperation::UpdateRemindItemLog(int sRemindRecordId,const string sConfirmUser,
                                            string sConfirmMessage,int nConfirmState)
{
    boost::recursive_mutex::scoped_lock lock(db_connect_mutex_);
    QSqlDatabase db = ConnectionPool::openConnection();
    if(!db.isOpen() || !db.isValid()) {
        std::cout<<"UpdateRemindItemLog error  ------ the database is interrupt"<<std::endl;
        return false;
    }
    QSqlQuery updateRemindQuery(db);

    QString strSql=QString("update remind_notify_record set confirmtime=:time,confirmuser= :user,confirmstate=:state"
                           ",confirmcontent=:content where id=:id");
    updateRemindQuery.prepare(strSql);
    updateRemindQuery.bindValue(":time",QDateTime::currentDateTime());
    updateRemindQuery.bindValue(":user",QString::fromStdString(sConfirmUser));
    updateRemindQuery.bindValue(":state",nConfirmState);
    updateRemindQuery.bindValue(":content",QString::fromStdString(sConfirmMessage));
    updateRemindQuery.bindValue(":id",sRemindRecordId);


    if(!updateRemindQuery.exec()){
        cout<<updateRemindQuery.lastError().text().toStdString()<<"UpdateRemindItemLog---updateRemindQuery---error!"<<endl;
        ConnectionPool::closeConnection(db);
        return false;
    }
    ConnectionPool::closeConnection(db);


}


}
