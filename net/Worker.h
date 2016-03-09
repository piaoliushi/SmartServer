#ifndef WORKER_H
#define	WORKER_H
#pragma once

///////////////////////////////////////
//模板类：创建多个线程用来维护一个任务队列，具体的任务由派生类重载work方法
//        完成，直到stop方法被调用，工作线程结束
//
#include "taskqueue.h"
namespace net
{
	template <typename Task>
	class Worker
	{
	public:
		Worker(TaskQueue<Task>& _taskqueue,size_t _maxthreads=2):
			taskqueue_(_taskqueue)
			,maxthreads_(_maxthreads)
			,exitthread(false)
		{

		}
		~Worker(void)
		{}
	public:
		void run()
		{
			try
			{
				for(size_t i=0;i<maxthreads_;++i)
				{
					//创建线程并绑定成员函数workloop
					boost::shared_ptr<boost::thread> _thread(
						new boost::thread(boost::bind(&Worker::workloop,this)));

					threads_.push_back(_thread);
				}

				for(size_t i=0;i<maxthreads_;++i)
				{
					threads_[i]->join();//等待所有线程结束后继续执行
				}
			}
			catch (std::exception& e)
			{
				std::cout << "ERROR INFO:" << e.what() << std::endl;
			}
		}
		void stop()
		{
			
			exitthread = true;
			//taskqueue_.ClearTaskList();
			taskqueue_.ExitNotifyAll();
		}
		virtual bool work(Task& task)=0;//派生类需要重载此虚函数，以完成工作
	protected:
		//工作循环
		void workloop()
		{
			do 
			{
				//从队列中取任务进行处理
				Task task_ = taskqueue_.GetTask();
				//调用子类work，处理具体任务
				if(work(task_))
					continue;
				else
					break;
			} while (!exitthread);
		}
	private:
        std::vector<boost::shared_ptr<boost::thread> > threads_;
		//boost::mutex mutex_;//没有用到
		size_t maxthreads_;
		TaskQueue<Task>& taskqueue_;
		bool exitthread;
	};

}

#endif
