#include "io_service_pool.h"


namespace hx_net
{
	io_service_pool::io_service_pool(size_t pool_size)
		:next_io_service_(0)
	{
		//io池尺寸不可为0
		if(pool_size==0)
			throw std::runtime_error("io_service_pool size is 0");
		//创建pool_size尺寸的io_service
		for(size_t i=0;i<pool_size;++i)
		{
			io_service_ptr io_service(new boost::asio::io_service);
			work_ptr work(new boost::asio::io_service::work(*io_service));
			io_services_.push_back(io_service);
			work_.push_back(work);
		}
	}

	//创建若干线程给io_service使用
	void io_service_pool::run()
	{
        std::vector<boost::shared_ptr<boost::thread> > threads;
		for(size_t i=0;i<io_services_.size();++i)
		{
			boost::shared_ptr<boost::thread> thread(new boost::thread(
				boost::bind(&boost::asio::io_service::run,io_services_[i])));
			threads.push_back(thread);

		}
		for(size_t i=0;i<threads.size();++i)
			threads[i]->join();
	}
	//停止io_service
	void io_service_pool::stop()
	{
		boost::recursive_mutex::scoped_lock lock(io_mutex_);
		for(size_t i=0;i<io_services_.size();++i)
			io_services_[i]->stop();
	}
	//获取一个io_service,平均分配方式
	boost::asio::io_service& io_service_pool::get_io_service()
	{
		boost::recursive_mutex::scoped_lock lock(io_mutex_);
		boost::asio::io_service& io_service=*io_services_[next_io_service_];
		++next_io_service_;
		if(next_io_service_==io_services_.size())
			next_io_service_=0;
		return io_service;
	}
}
