#ifndef RELAY_SERVER_H
#define RELAY_SERVER_H

#pragma once

#include "../io_service_pool.h"
#include "../../qnotifyhandler.h"
using boost::asio::ip::tcp;



namespace net
{
	class SvcClient
	{
	public:
		//初始化一个客户端，该客户端维护一个任务队列，并创建一个io_service
		SvcClient(TaskQueue<msgPointer>& taskwork,size_t io_service_pool_size=2);
		~SvcClient();
	public:
		void run();//启动io_service pool
		void stop();
		void connect_all();
		void disconnect_all();
		
		//发射机是否已关闭
		bool transmitter_is_online(const string sTransmitterNumber);

		//发射机是否正在运行
		bool transmitter_is_offline(const string sTransmitterNumber);

		//发射机是否已关闭
		bool transmitter_is_shutdown(const string sTransmitterNumber);

		//发射机是否正在运行
		bool transmitter_is_running(const string sTransmitterNumber);

		//开发射机
		bool turn_on_transmitter(const string sTransmitterNumber);

		//关发射机
		bool turn_off_transmitter(const string sTransmitterNumber);

		//升功率
		bool upper_transmitter_power(const string sTransmitterNumber);

		//降功率
		bool lower_transmitter_power(const string sTransmitterNumber);

		//切换天线
		bool switch_antenna_pos(const string sAntennaNumber,bool bHost);

		//获得发射机在线数
		int get_transmitter_online_count();

		//获得发射机运行数
		int get_transmitter_running_count();

		//获得天线在线数
		int get_antenna_online_count();

		//获得天线运行数
		int get_antenna_running_count();
		//获取转换模块在线数
		int get_modle_online_count();

	private:
		TaskQueue<msgPointer>& taskwork_;
		boost::mutex client_session_mutex;
		std::map<string,session_ptr> client_pool_;
		io_service_pool io_service_pool_;

	};
}

#endif//RELAY_SERVER_H