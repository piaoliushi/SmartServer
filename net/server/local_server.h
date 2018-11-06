#ifndef LOCALSERVER_H
#define LOCALSERVER_H

#pragma once
#include "../io_service_pool.h"
#include "../share_ptr_object_define.h"

using boost::asio::ip::tcp;

namespace hx_net
{
struct HandlerKey
{
    tcp::endpoint  endPoint_;//登陆客户端地址
    string         usr_;     //登陆用户
    string         psw_;     //登陆密码
    string         usr_number_;//用户编号
    string         client_id_;//客户端编号
};

struct UserSignInInfo
{
public:
    UserSignInInfo();
    ~UserSignInInfo();
public:
    UserInformation UsrInfo;//用户名
    time_t SignInTime;//签到时间
};
    class LocalServer
	{
	public:
		//初始化一个服务，该服务维护一个任务队列，并创建一个4尺寸io_service
        LocalServer(short port,TaskQueue<msgPointer>& taskwork,size_t io_service_pool_size=4);
        ~LocalServer();
		void run();
		void stop();
		void start_accept();
		void handle_accept(session_ptr new_session_ptr,const boost::system::error_code& error);

        //连接注册
		void register_connection_handler(session_ptr ch_ptr);
		//连接移除
		int remove_connection_handler(session_ptr ch_ptr);
		//用户登陆
		void user_login(session_ptr ch_ptr,string sUser,string sPassword,LoginAck &loginAck);
		//用户注销
		void user_logout(session_ptr ch_ptr,string sUser,string sPassword,LogoutAck &logoutAck);

		int remove_all();

		//发送设备数据通知
		void send_dev_data(string sStationid,string sDevid,devDataNfyMsgPtr &dataPtr);
		//发送设备网络状态数据
		void send_dev_net_state_data(string sStationid,string sDevid,devNetNfyMsgPtr &netPtr);
		//发送设备运行状态数据
		void send_dev_work_state_data(string sStationid,string sDevid,devWorkNfyMsgPtr &workPtr);
		//发送设备报警状态数据
		void send_dev_alarm_state_data(string sStationid,string sDevid,devAlarmNfyMsgPtr &alarmPtr);
		//发送控制执行结果通知
		void send_command_execute_result(string sStationid,string sDevid,e_MsgType nMsgType,devCommdRsltPtr &commdRsltPtr);

		void BroadcastMessage(e_MsgType _type,googleMsgPtr gMsgPtr);
		//获得子台站设备状态信息
		loginAckMsgPtr  get_child_station_dev_status();
		//收集本平台所有设备状态信息
		void  get_local_station_dev_status(loginAckMsgPtr &statusMsgPtr);


        //用户交接班
        void user_handover(session_ptr ch_ptr,string sCurUsr,string sNewUser,
                           string sNewPassword,const string &sContents,LoginAck &loginAck);
        //用户签到
        //void user_signin_out(session_ptr ch_ptr,string sSignInUsr,string sSignInPsw,signInOutAckMsgPtr signinPtr,int bIn);

        //收集当前签到人信息
        //void addSignInUsersToLoginAck(session_ptr ch_ptr,const UserInformation &sUser,LoginAck &loginAck);
        //签到签退操作
        //void handSignInAndOut(session_ptr ch_ptr,bool bIsIn,time_t &curTm,const UserInformation &sUser
        //                      ,e_ErrorCode &eError);
        //通知客户端签到结果
        //void notify_other_client_signin_result(session_ptr ch_ptr,int bIn,const UserSignInInfo &sUser);
    protected:
        void user_switch_login(session_ptr ch_ptr,string sNewUser,string sPassword,LoginAck &loginAck,UserInformation &tmpUser);
        //获取用户与当前客户端id下的设备授权信息
        void get_authorize_info_by_user(session_ptr ch_ptr,string sUserNum,string sClientNum,LoginAck &loginAck);

	private:	
		
		TaskQueue<msgPointer>& taskwork_;//引用一个任务队列
		io_service_pool io_service_pool_;//定义一个io池
		tcp::acceptor acceptor_;//定义一个接受器（客户端服务接受器）

		boost::recursive_mutex mutex_;
		std::map<session_ptr,HandlerKey> session_pool_;//用户连接handler

        std::map<string,vector<session_ptr> > devToUser_;//通过Dev获得对应的连接

        boost::recursive_mutex sign_mutex_;
        std::vector<UserSignInInfo> sign_in_users_;//当前签到用户
		
	};
}

#endif
