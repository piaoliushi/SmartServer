#ifndef DBMANAGER_H
#define DBMANAGER_H
#include <QObject>
#include <QTimer>
#include <vector>
#include <map>
#include <string>
#include <boost/thread.hpp>
#include "../DataType.h"
#include "../net/share_ptr_object_define.h"
using namespace std;
using namespace net;
class DbManager
{
public:
    DbManager();
public:
    bool OpenDb(const std::string& serveraddress,
                const std::string& database,
                const std::string& uid,
                const std::string& pwd,
                int timeout,
                std::string driverName="SQL Native Client");

    bool CloseDb();
    bool IsOpen();
    bool ReOpen();
    void StartReOpen();//开始重连数据库
public:
    //获取某一台站信息(已测)
    bool GetStationInfo(const string sNumber,StationInformation &Info);
    //获取本平台所所属一级平台服务信息
    bool GetChildStationDevServer(const string sNumber,vector<DevSvcInfo> &svcInfo);
    //获得用户信息
    bool GetUserInfo(const string sName,const string sStationNumber,UserInformation &user);
    //获得发射机信息
    bool GetTransmitters(const string sStationNumber,const string sServerNumber,vector<TransmitterInformation> &vecTransmitters);
    bool GetTransmitters(const string sStationNumber,vector<TransmitterInformation>&vecTransmitters);
    //读取发射机与天线代理模块信息
    bool GetThransmitterAgentInfo(const string sStationNumber,const string sServerNumber,vector<ModleInfo> &vecModle);
    //获得天线信息
    bool GetAntennaInfo(const string sStationNumber,const string sServerNumber,vector<AntennaInformation> &antenna);
    bool GetAntennaInfo(const string sAntennaNumber,AntennaInformation &antenna);
    bool GetAntennaAgentInfo(const string sStationNumber,const string sServerNumber,vector<ModleInfo> &vecModle);
    //获得台站天线信息
    bool GetAntennaInfo(const string sStationNumber,vector<AntennaInformation> &antenna);
    //获得发射机与天线关联信息
    bool GetAssociateInfo(const string sStationNumber,const string sServerNumber,vector<AssociateInfo> &vecAssociateInfo);
    bool GetAssociateInfo(const string sStationNumber,vector<AssociateInfo> &vecAssociateInfo);
    // 获取发射机开关机计划
    bool GetTransmitterSchedule(const string sTransmitterNumber, vector<TransmitterSchedule> &schedules);
    // 获得发射机监控量配置信息
    bool GetTransmitterMonitorItem(const string sStationNumber, const string sTransmitterNumber,
                                   std::map<int, DevParamerMonitorItem> &mapMonitorItems);
    // 保存发射机监控数据
    bool SaveTransmitterMonitoringData(const string sStationNumber,const string sTransmitterNumber,
                                       map<int,DevParamerMonitorItem>& items,const DevMonitorDataPtr dataPtr);
    // 保存发射机监控数据(google message)
    bool SaveTransmitterMonitoringData(devDataNfyMsgPtr &dataPtr);
    // 保存报警开始记录
    bool AddTransmitterAlarm(tm * curTime,const string sStationNumber,const string sTransmitterNumber,
                             int nMonitoringIndex,int alarmType,double dValue=0.00f);//const string &sAlarmContent
    // 保存报警结束记录
    bool UpdateTransmitterAlarmEndTime(tm *curTime,const tm &stime,const string sStationNumber,
                                       const string sTransmitterNumber,const int nMonitoringIndex);
    //由天线获得主机id
    bool GetHostTransmitterIdFromAntenna(const string sStationNumber,const string sAntennaId,string &sHostId);
    //由天线获得备机id
    bool GetBackupTransmitterIdFromAntenna(const string sStationNumber,const string sAntennaId,string &sBackupId);
    //根据服务号获取模块信息
    bool GetModleInfo(const string sStationNumber,const string sServerNumber,vector<ModleInfo> &vecModle);
    //根据服务编号获取主动上传设备信息
    bool GetUploadDevInfo( const string sStationNumber,const string sServerNumber,
                           vector<MediaDeviceParamInfo> &vecUpload );//DevParamerInfo

    bool SaveOthDevMonitoringData(const string sStationNumber,const string sOtherDevNumber,
                                  map<int,DevParamerMonitorItem>& items,const DevMonitorDataPtr dataPtr);

    bool AddOthDevAlarm(tm *stime,const string sStationNumber,const string sDevNum,
                         int nMonitoringIndex,int alarmType);
    bool UpdateOthDevAlarm(tm *curTime,const tm &stime,const string sStationNumber,const string sDevNum,
                           int nMonitoringIndex);

    //获取用户的授权设备
    bool GetAllAuthorizeDevByUser(const string sUserId,vector<pair<string,string> > &vDevice);
    //获取所拥有的服务信息
    bool GetServerInfo(const string sStationNumber,const string sServerNumber,vector<ServerInfo>&vecServerInfo);
    //添加操作记录
    bool AddOperationRecord(const string sStationNumber,const string sDevNumber,int optType
                            ,const string optResult,const string oprUser);
    //添加工作记录
    bool AddDevWorkRecord(const string sStationNumber,const string sDevNumber,const tm *stime);
    //值班日志
    bool AddDutyLog(const string sStationNumber,const string sUserNumber,const string sContent,int nType);
    //交班记录
    bool AddHandove(const string sStationNumber,const string sHandoveNumber,const string sSuccessorNumber,const string &sLogContents,const tm* ttime);
    //用户签到
    bool AddSignin(const string sStationNumber,const string sSignerNumber,const tm* ttime);
    //用户签退
    bool AddSignout(const string sStationNumber,const string sSignerNumber,const tm* tintime,const tm* touttime);

    //获得用户电话配置
    bool GetTelInfoByStation(const string sStationNumber,vector<SendMSInfo> &vecSendMsInfo);
    //获得当前台站某服务所有授权设备
    bool GetAllAuthorizeDevByDevServer(const string sServerNumber,vector<pair<string,string> > &vDevice);
    //写入电话告警内容（第三方程序支持）
    bool WriteCallTask( string sPhone,string sVoice_text,int nType=SMS);

    //获得设备监控量相关的联动角色ID
    bool GetAllLinkageRoleNumber(const string &sStationNumber,vector<LinkageInfo> &vLinkages);

    //获得媒体设备参数信息
    bool GetMediaDevEncoderInfo(const string sSationNumber,const string sDevNumver,DeviceEncoderInfo &info);

    //获得主动连接媒体设备
    bool GetConnectMediaDevInfo( const string sStationNumber,const string sServerNumber,vector<MediaDeviceParamInfo> &vecDevice );

    //添加一条查岗记录
    bool AddCheckWorking( const string sStationNumber,const tm* asktime);
    //更新查岗确认
    bool AddCheckAck( const string sStationNumber,const string sUserNumber,const tm* asktime,const tm* acktime );
    //获得台站所有用户信息
    bool GetAllUserInfoByStation(const string sStationNumber,LoginAck &users);
protected:
    //获取发射机对应的协议信息
    bool GetTransmitterProtocolInfo(int nProtocolNumber, TransmitterProtocol &protocol);
    bool GetNetDevInfo(const string sStationNumber,const string sServerNumber,vector<ModleInfo> &vecModle);
};

#endif // DBMANAGER_H
