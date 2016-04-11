#include "ntp_session.h"
using namespace db;
namespace hx_net
{
ntp_session::ntp_session(boost::asio::io_service& io_service)
    :query_timer_(io_service)
    ,receive_msg_ptr_(new othdev_message(2048))
    ,io_service_(io_service)
{

}

ntp_session::~ntp_session()
{

}

//udp连接
void ntp_session::udp_connect(std::string hostname,unsigned short port)
{
    boost::system::error_code ec;
    udp::resolver::query query(hostname,boost::lexical_cast<std::string, unsigned short>(port));
    udp::resolver::iterator iter = uresolver_.resolve(query,ec);
    if(iter!=udp::resolver::iterator())
    {
        uendpoint_ = (*iter).endpoint();
        usocket().open(udp::v4());
        const udp::endpoint local_endpoint = udp::endpoint(udp::v4(),port);
        usocket().bind(local_endpoint);
        usocket().open(udp::v4());
    }
}

void ntp_session::start_query_timer(unsigned long nSeconds/* =3 */)
{
    query_timer_.expires_from_now(boost::posix_time::millisec(nSeconds));
    query_timer_.async_wait(
            #ifdef USE_STRAND
                strand_.wrap(
                #endif
                    boost::bind(&device_session::query_send_time_event,this,boost::asio::placeholders::error)
                #ifdef USE_STRAND
                    )
            #endif
                );
}

void  ntp_session::query_send_time_event(const boost::system::error_code& error)
{

}


void ntp_session::start_write(unsigned char* commStr,int commLen)
{
    if(commLen<=0)
        return;
    usocket().async_send_to(boost::asio::buffer(commStr,commLen),uendpoint_,
                        #ifdef USE_STRAND
                            strand_.wrap(
                            #endif
                                boost::bind(&net_session::handle_write,shared_from_this(),
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred)
                            #ifdef USE_STRAND
                                )
                        #endif
                            );
}

void ntp_session::close_all()
{
    close_i();   //关闭socket
 }

void ntp_session::handle_read_head(const boost::system::error_code& error, size_t bytes_transferred)
{
    if (!error)
    {
        int nResult = receive_msg_ptr_->check_normal_msg_header(dev_agent_and_com[cur_dev_id_].second,bytes_transferred,CMD_QUERY,cur_msg_q_id_);
        if(nResult>0)//检查消息头
            start_read_body(nResult);
        else{
            close_all();
            start_connect_timer(moxa_config_ptr->connect_timer_interval);
        }
    }
    else{
        close_all();
        start_connect_timer(moxa_config_ptr->connect_timer_interval);
    }
}

void ntp_session::start_read_head(int msgLen)
{
    udp::endpoint sender_endpoint;
    usocket().async_receive_from(boost::asio::buffer(receive_msg_ptr_->w_ptr(),receive_msg_ptr_->space()),sender_endpoint,
                                                boost::bind(&device_session::handle_udp_read,this,
                                                 boost::asio::placeholders::error,
                                                 boost::asio::placeholders::bytes_transferred)
                                              );

}

void ntp_session::handle_udp_read(const boost::system::error_code& error,size_t bytes_transferred)
{
    if (!error || error == boost::asio::error::message_size)
    {

        start_read_head(bytes_transferred);
    }
    else{

        start_connect_timer(moxa_config_ptr->connect_timer_interval);
    }
}



void device_session::start_read_body(int msgLen)
{
        udp::endpoint sender_endpoint;
        usocket().async_receive_from(boost::asio::buffer(receive_msg_ptr_->w_ptr(),
                                                         msgLen),
                                     sender_endpoint,
                             #ifdef USE_STRAND
                                     strand_.wrap(
                                 #endif
                                         boost::bind(&device_session::handle_read_body,this,
                                                     boost::asio::placeholders::error,
                                                     boost::asio::placeholders::bytes_transferred)
                                 #ifdef USE_STRAND
                                         )
                             #endif
                                     );

}


void ntp_session::handle_read_body(const boost::system::error_code& error, size_t bytes_transferred)
{
    if (!error)
    {

     }
}

}
