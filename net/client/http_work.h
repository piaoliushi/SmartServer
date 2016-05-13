#ifndef CLIENT_WORK
#define CLIENT_WORK
#pragma once
#include "../Worker.h"
#include "../taskqueue.h"
using namespace std;
using namespace hx_net;
class http_work:public Worker<msgPointer>
{
public:
    http_work(TaskQueue< pair<string,string> >& _taskqueue,std::size_t _maxthreads = 4) :
      Worker< pair<string,string> >(_taskqueue, _maxthreads){}

    ~http_work(void){};
public:
    bool work(pair<string,string>& task)
    {

        return true;
	}
};
#endif
