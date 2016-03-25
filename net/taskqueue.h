#ifndef TASKQUEUE_H
#define TASKQUEUE_H

#include "include.h"
namespace hx_net
{
	template<typename Task>
	class TaskQueue
	{
	public:
		TaskQueue():exitwait_(false)
		{}
		~TaskQueue()
		{
			list_.clear();
		}

		//提交任务
		void SubmitTask(const Task& task)
		{
			boost::mutex::scoped_lock lock(mutex_);
			list_.push_back(task);//将任务拷贝到list
			worktobedone_.notify_all();
		}
		//获取任务
		Task GetTask()
		{
			boost::mutex::scoped_lock lock(mutex_);
			while(list_.size()==0 && !exitwait_)
			{//线程没有通知退出且无任务，由condition完成同步等待
				worktobedone_.wait(lock);
			}

			Task tmp;//这个地方处理感觉有点草率
			if(exitwait_)
				return tmp;
			tmp = list_.front();
			list_.pop_front();
			return tmp;
		}
		//清空当前任务
		void ClearTaskList()
		{
			boost::mutex::scoped_lock lock(mutex_);
			list_.clear();
		}
		//由上层代码发起，退出所有任务处理线程
		void ExitNotifyAll()
		{
			exitwait_=true;
			worktobedone_.notify_all();
		}
	private:
		bool exitwait_;
		std::list<Task> list_;
		boost::mutex mutex_;
		boost::condition worktobedone_;
	};
}
#endif
