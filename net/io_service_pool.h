#ifndef IO_SERVICE_POOL_H
#define IO_SERVICE_POOL_H

#pragma once

#include "include.h"
#include "message.h"
#include "taskqueue.h"
#include "session.h"
#include "Worker.h"
#include "config.h"

using boost::asio::ip::tcp;
using namespace std;

namespace net
{
	class io_service_pool:private boost::noncopyable//禁止拷贝构造
	{
	public:
		explicit io_service_pool(std::size_t pool_size);//显式默认构造函数
		void run();
		void stop();
		boost::asio::io_service& get_io_service();
	private:
		typedef boost::shared_ptr<boost::asio::io_service> io_service_ptr;
		typedef boost::shared_ptr<boost::asio::io_service::work> work_ptr;
		//io_services_池
		std::vector<io_service_ptr> io_services_;
		//io_service::work若不被析构可以保证io_service在没有io消息的情况下不自动退出
		//若需要退出可以通过io_service的stop方法
		std::vector<work_ptr> work_;
		std::size_t next_io_service_;
		boost::recursive_mutex io_mutex_;
	};
}

#endif