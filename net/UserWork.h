#ifndef USER_WORK_H
#define USER_WORK_H
#pragma once

#include "include.h"
#include "message.h"
#include "Worker.h"
#include "taskqueue.h"
#include "net_session.h"
#include "./protocol/protocol.pb.h"

using namespace hx_net;

class UserWork: public Worker<msgPointer>
{
public:
	UserWork(TaskQueue<msgPointer>& _taskqueue,std::size_t _maxthreads = 4) :
	Worker<msgPointer>(_taskqueue, _maxthreads)
	{
	}
public:
	bool work(msgPointer& task)
	{
		/*if(task->isappendmsg())
		{	
			session_ptr psession;
			task->getsession(psession);	
			if (!psession) 
				return false;
			switch(task->getappendmsg())
			{
			case CONNECT_SUCCESS:
				{

					break;
				} 
			case CONNECT_TIMEOUT:
				{
					
				
					break;
				}
			}
		}
		else
		{
			session_ptr psession;
			task->getsession(psession);
			if (!psession) 
				return false;

			packHeadPtr MsgPtr = task->msg();

			// 用psession往客户端回发数据;
			switch (MsgPtr->type)
			{
			case MSG_HEARTBEAT_ACK://心跳回复
				// this->OnHeart(MsgPtr->heart);           // 处理心跳.
				break;
			case MSG_CONFIG_REQ://配置响应
				{
					AllStationConfigInfo _Info;
					for(int i=0;i<100;i++)
					{
						StationConfigInfo *pStation = _Info.add_cstationinfo();
						pStation->set_sstationid("1000");
						if(pStation!=NULL)
						{
							for(int j=0;j<50;j++)
							{
								ElcConfigInfo *pElc = pStation->add_celccfginfo();
								pElc->set_celcdevtype(EDA9033A);
								pElc->set_sdevid("10001001");

								pElc->set_sdevname("电力设备54456王升");

								EnvirConfigInfo *pEnvir = pStation->add_cenvircfginfo();
								pEnvir->set_sdevid(std::string("10002001"));
								
								pEnvir->set_sdevname("87878979");
								pEnvir->set_cenvirdevtype(HUMIDITY);
							}
							
						}
					}
					psession->sendMessage(MSG_CONFIG_ACK,&_Info);
				}
				
				break;
			default:
				break;
			}
		}*/


		return true;
	}

	//message *pMsg; = new message;
	//AllDevConfigInfo curCfg;//此对象用于保存当前设备配置
};

#endif
