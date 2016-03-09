#ifndef NET_CONFIG_H
#define NET_CONFIG_H
#include <boost/thread/detail/singleton.hpp>
//是否使用ssl通信
//#define USE_SSL
//使用服务池
#define USE_POOL
//使用strand对象
#define USE_STRAND
//使用client strand 对象
#define USE_CLIENT_STRAND

//定义boost单体对象访问宏
#define GetInst(T) boost::detail::thread::singleton<T>::instance()



#endif
