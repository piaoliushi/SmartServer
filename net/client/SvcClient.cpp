#include "SvcClient.h"
#include "client_session.h"
//#include "Transmitter_session.h"
//#include "Antenna_session.h"
#include "StationConfig.h"
//#include "othdev_session.h"
namespace net
{
	SvcClient::SvcClient(TaskQueue<msgPointer>& taskwork,size_t io_service_pool_size/* =4 */)
		:io_service_pool_(io_service_pool_size)
		,taskwork_(taskwork)
	{

	}

	SvcClient::~SvcClient()
	{

	}

	void SvcClient::connect_all()
	{

	}

	void SvcClient::disconnect_all()
	{

	}

	void SvcClient::run()
	{
		io_service_pool_.run();
	}
	void SvcClient::stop()
	{
		io_service_pool_.stop();
	}

	//发射机是否在线
	bool SvcClient::transmitter_is_online(const string sTransmitterNumber)
	{
		return false;
	}

	//发射机是否正在运行
	bool SvcClient::transmitter_is_offline(const string sTransmitterNumber)
	{
		return false;
	}

	//发射机是否正在运行
	bool SvcClient::transmitter_is_running(const string sTransmitterNumber)
	{

		return false;
	}

	//发射机是否已经关机
	bool SvcClient::transmitter_is_shutdown(const string sTransmitterNumber)
	{

		return false;
	}

	//开发射机
	bool SvcClient::turn_on_transmitter(const string sTransmitterNumber)
	{
		return false;
	}

	//关发射机
	bool SvcClient::turn_off_transmitter(const string sTransmitterNumber)
	{
		return false;
	}

	//升功率
	bool SvcClient::upper_transmitter_power(const string sTransmitterNumber)
	{
		return false;
	}

	//降功率
	bool SvcClient::lower_transmitter_power(const string sTransmitterNumber)
	{
		return false;
	}

	//切换天线到主机
	bool SvcClient::switch_antenna_pos(const string sAntennaNumber,bool bHost)
	{
		return false;
	}

	//获得发射机在线数
	int SvcClient::get_transmitter_online_count()
	{

		return 0;
	}

	//获得发射机运行数
	int SvcClient::get_transmitter_running_count()
	{

		return 0;
	}

	//获得天线在线数
	int SvcClient::get_antenna_online_count()
	{
		return 0;
	}


	int SvcClient::get_modle_online_count()
	{

		return 0;
	}

	int SvcClient::get_antenna_running_count()
	{

		return 0;
	}
}
