#ifndef GPSCHECKTIME_H
#define GPSCHECKTIME_H
//#include "./net/io_service_pool.h"
#include <boost/shared_ptr.hpp>
#include <QString>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QObject>
#include <vector>
//using namespace hx_net;
using namespace std;
class GpsCheckTime : public QObject
{
    Q_OBJECT
public:
    GpsCheckTime(int icom,int ibaud=9600,QObject *parent=0);
    ~GpsCheckTime();

    char* com_rd_data(){return &(com_data_[0])+i_read_pos_;}
    char* com_wr_data(){return &(com_data_[0])+i_write_pos_;}
    bool com_open();
    bool com_close();
    void init_gps();
public slots:
    void handle_com_read();
signals:
    void S_Gpsdatetime(QString qsDatetime);
    void S_Gpsgll(bool block);
protected:
    void handle_write(const boost::system::error_code& error, size_t bytes_transferred){}
    void paseGpsdata(char* qData);
    int get_data_len(){return (i_write_pos_-i_read_pos_);}
    void gpzda_data(QString strdata);
    void gpgll_data(QString strdata);
private:
    boost::recursive_mutex         Datalist_mutex_;
    vector<char>    com_data_;
    int i_read_pos_,i_write_pos_;
    boost::shared_ptr<QSerialPort> pQtSerialport_ptr_;
    int icom_num_;
    int icom_baud_;
    bool b_gps_init_;
    QString qs_lock_state_;
     boost::recursive_mutex      com_data_mutex_;
};

#endif // GPSCHECKTIME_H
