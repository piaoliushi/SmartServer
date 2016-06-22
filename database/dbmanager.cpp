#include "dbmanager.h"

DbManager::DbManager()
{
}

bool DbManager::OpenDb(const std::string& serveraddress,
                                const std::string& database,
                                const std::string& uid,
                                const std::string& pwd,
                                int timeout,
                               std::string driverName )
{
    return true;
}

void DbManager::StartReOpen()
{
}

bool DbManager::ReOpen()
{
    return true;
}

bool DbManager::IsOpen()
{
    return true;
}

bool DbManager::CloseDb()
{
        return true;

}

//获取台站信息
bool DbManager::GetStationInfo(const string sNumber,StationInformation &Info)
{
return true;
}

//获取本平台所所属一级平台服务信息
bool DbManager::GetChildStationDevServer(const string sNumber,vector<DevSvcInfo> &svcInfo)
{
    return true;
}

//获得用户信息
bool DbManager::GetUserInfo(const string sName,const string sStationNumber,
                                  UserInformation &user)
{
    return true;
}

//获得台站所有用户信息
bool DbManager::GetAllUserInfoByStation(const string sStationNumber,LoginAck &users)
{
    return true;
}


//获取发射机对应的协议信息
bool DbManager::GetTransmitterProtocolInfo(const int nProtocolNumber, TransmitterProtocol &protocol)
{
    return true;
}

//获取发射机信息
bool DbManager::GetTransmitters(const string sStationNumber,const string sServerNumber,vector<TransmitterInformation> &vecTransmitters)
{
    return true;
}

//获取发射机信息
bool DbManager::GetTransmitters(const string sStationNumber,vector<TransmitterInformation> &vecTransmitters)
{
    return true;
}


//读取天线信息
bool DbManager::GetAntennaInfo(const string sStationNumber,const string sServerNumber,vector<AntennaInformation> &antenna)
{
    return true;
}

//获得台站天线信息
bool DbManager::GetAntennaInfo(const string sStationNumber,vector<AntennaInformation> &antenna)
{
    return true;
}
//获得天线信息
bool DbManager::GetAntennaInfo(const string sAntennaNumber, AntennaInformation &antenna)
{
    return true;

}
//获得发射机与天线关联信息
bool DbManager::GetAssociateInfo(const string sStationNumber,const string sServerNumber,vector<AssociateInfo> &vecAssociateInfo)
{
    return true;
}
bool DbManager::GetAssociateInfo(const string sStationNumber,vector<AssociateInfo> &vecAssociateInfo)
{
    return true;
}

// 获取发射机开关机计划
bool DbManager::GetTransmitterSchedule(const string sTransmitterNumber,
                                             std::vector<TransmitterSchedule> &schedules)
{
    return true;
}

// 获得发射机监控量配置信息
bool DbManager::GetTransmitterMonitorItem(const string sStationNumber, const string sTransmitterNumber,
                                                std::map<int, DevParamerMonitorItem> &mapMonitorItems)
{
    return true;
}

// 保存发射机监控数据
bool DbManager::SaveTransmitterMonitoringData(const string sStationNumber,const string sTransmitterNumber,
                                                    map<int,DevParamerMonitorItem>& items,const DevMonitorDataPtr dataPtr)
{
    return true;
}

// 保存发射机监控数据(google message)
bool DbManager::SaveTransmitterMonitoringData(devDataNfyMsgPtr &dataPtr)
{
    return true;
}

// 保存报警开始记录
// 说明:程序记录报警开始时间，并向报警记录表插入一条记录，当该项报警解除/程序正常退出，根据时间/监控项/发射机编号/台站编号
//      更新唯一记录
// 保存报警开始记录


bool DbManager::AddTransmitterAlarm(tm *curTime,const string sStationNumber,const string sTransmitterNumber,
                         int nMonitoringIndex,int alarmType,double dValue)// const string &sAlarmContent
{
    return true;
}
// 更新报警结束记录
//报警开始时间对于广播报警消息具有重要意义

bool DbManager::UpdateTransmitterAlarmEndTime(tm *curTime,const tm &stime,const string sStationNumber,
                                                    const string sTransmitterNumber,const int nMonitoringIndex)
{
    return true;
}

//获得主机id
bool DbManager::GetHostTransmitterIdFromAntenna(const string sStationNumber,
                                     const string sAntennaId,
                                     string &sHostId)
{
    return true;
}

//获得备机id
bool DbManager::GetBackupTransmitterIdFromAntenna(const string sStationNumber,
                                                        const string sAntennaId,
                                                        string &sBackupId)
{
    return true;
}

bool DbManager::GetAntennaAgentInfo(const string sStationNumber,const string sServerNumber,vector<ModleInfo> &vecModle)
{
    return true;
}

//读取发射机与天线代理模块信息
bool DbManager::GetThransmitterAgentInfo(const string sStationNumber,const string sServerNumber,vector<ModleInfo> &vecModle)
{
    return true;
}

bool DbManager::GetNetDevInfo(const string sStationNumber,const string sServerNumber,vector<ModleInfo> &vecModle)
{
    return true;
}

//读取模块信息(主动连接设备)
bool DbManager::GetModleInfo(const string sStationNumber,const string sServerNumber,vector<ModleInfo> &vecModle)
{
    return GetNetDevInfo(sStationNumber,sServerNumber,vecModle);
}

//读取模块信息(主动上传媒体设备)
bool DbManager::GetUploadDevInfo( const string sStationNumber,const string sServerNumber,vector<MediaDeviceParamInfo> &vecUpload )
{
    //2015-10-29 修改将 and a.CommType=3，条件改为and a.CommType=1
    return true;
}


//读取主动连接媒体设备)
bool DbManager::GetConnectMediaDevInfo( const string sStationNumber,const string sServerNumber,vector<MediaDeviceParamInfo> &vecDevice )
{
    return true;
}

bool DbManager::SaveOthDevMonitoringData(const string sStationNumber,const string sOtherDevNumber,
                                               map<int,DevParamerMonitorItem>& items,const DevMonitorDataPtr dataPtr)
{
    return true;
}

bool DbManager::AddOthDevAlarm(tm *stime,const string sStationNumber,const string sDevNum,
                    int nMonitoringIndex,int alarmType)// const string &sAlarmContent
{
    return true;
}


bool DbManager::UpdateOthDevAlarm(tm *curTime,const tm &stime,const string sStationNumber,const string sDevNum,
                       int nMonitoringIndex)
{
    return true;
}
//获得服务授权设备
bool DbManager::GetAllAuthorizeDevByDevServer(const string sServerNumber,vector<pair<string,string> > &vDevice)
{
    return true;
}

//获取用户的授权设备
bool DbManager::GetAllAuthorizeDevByUser(const string sUserId,vector<pair<string,string> > &vDevice)
{
    return true;
}

//获取所拥有的服务信息(这个是新添加的)
bool DbManager::GetServerInfo(const string sStationNumber,const string sServerNumber,vector<ServerInfo>&vecServerInfo)
{
    return true;
}

//添加操作记录
bool DbManager::AddOperationRecord(const string sStationNumber,const string sDevNumber,int optType
                                        ,const string optResult,const string oprUser)
{
    return true;
}

//添加工作记录
bool DbManager::AddDevWorkRecord(const string sStationNumber,const string sDevNumber,const tm *stime)
{
    return true;
}

bool DbManager::GetTelInfoByStation(const string sStationNumber,vector<SendMSInfo> &vecSendMsInfo)
{
    return true;
}


bool DbManager::WriteCallTask( string sPhone,string sVoice_text,int nType )
{
    return true;
}

//获得设备监控量相关的联动角色ID
bool DbManager::GetAllLinkageRoleNumber(const string &sStationNumber,vector<LinkageInfo> &vLinkages)
{
    return true;
}


bool DbManager::GetMediaDevEncoderInfo( const string sSationNumber,const string sDevNumver,DeviceEncoderInfo &info )
{
    return true;
}

//添加值班日志
bool DbManager::AddDutyLog( const string sStationNumber,const string sUserNumber,const string sContent,int nType )
{
    return true;
}

bool DbManager::AddHandove( const string sStationNumber,const string sHandoveNumber,const string sSuccessorNumber,
                                  const string &sLogContents,const tm* ttime )
{
    return true;
}

bool DbManager::AddSignin( const string sStationNumber,const string sSignerNumber,const tm* ttime )
{
    return true;
}

bool DbManager::AddSignout( const string sStationNumber,const string sSignerNumber,const tm* tintime,const tm* touttime )
{
    return true;
}

bool DbManager::AddCheckWorking( const string sStationNumber,const tm* asktime )
{
    return true;
}

bool DbManager::AddCheckAck( const string sStationNumber,const string sUserNumber,const tm* asktime,const tm* acktime )
{
    return true;
}
