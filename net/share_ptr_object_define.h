#ifndef SHARE_PTR_OBJECT_DEFINE
#define SHARE_PTR_OBJECT_DEFINE
#include <boost/shared_ptr.hpp>
#include "../protocol/protocol.pb.h"
#include "../DataType.h"
#include <vector>
using namespace std;
namespace net
{
	typedef boost::shared_ptr<google::protobuf::Message>  googleMsgPtr;
	typedef boost::weak_ptr<google::protobuf::Message>    googleMsgWeakPtr;

	
	typedef boost::shared_ptr<LoginReq>              loginReqMsgPtr;
	typedef boost::weak_ptr<LoginReq>                loginReqMsgWeakPtr;

	typedef boost::shared_ptr<LoginAck>              loginAckMsgPtr;
	typedef boost::weak_ptr<LoginAck>                loginAckMsgWeakPtr;

	typedef boost::shared_ptr<SignInOutAck>          signInOutAckMsgPtr;
	typedef boost::weak_ptr<SignInOutAck>            signInOutAckMsgWeakPtr;

	
	typedef boost::shared_ptr<DutyLogAck>              dutyLogAckMsgPtr;
	typedef boost::weak_ptr<DutyLogAck>                dutyLogAckMsgWeakPtr;

	typedef boost::shared_ptr<CheckStationWorkingReq>  checkWorkingReqMsgPtr;
	typedef boost::weak_ptr<CheckStationWorkingReq>    checkWorkingReqWeakPtr;

	typedef boost::shared_ptr<CheckStationWorkingAck>  checkWorkingAckMsgPtr;
	typedef boost::weak_ptr<CheckStationWorkingAck>    checkWorkingAckWeakPtr;

	typedef boost::shared_ptr<CheackStationWorkingNotify>  checkWorkingNotifyMsgPtr;
	typedef boost::weak_ptr<CheackStationWorkingNotify>    checkWorkingNotifyWeakPtr;

	

	typedef boost::shared_ptr<LogoutReq>              logoutReqMsgPtr;
	typedef boost::weak_ptr<LogoutReq>                logoutReqMsgWeakPtr;

	typedef boost::shared_ptr<LogoutAck>              logoutAckMsgPtr;
	typedef boost::weak_ptr<LogoutAck>                logoutAckMsgWeakPtr;

	typedef boost::shared_ptr<DevDataNotify>         devDataNfyMsgPtr;
	typedef boost::weak_ptr<DevDataNotify>           devDataNfyMsgWeakPtr;
	typedef boost::shared_ptr<DevAlarmStatusNotify>  devAlarmNfyMsgPtr;
	typedef boost::weak_ptr<DevAlarmStatusNotify>    devAlarmNfyMsgWeakPtr;
	typedef boost::shared_ptr<DevWorkStatusNotify>   devWorkNfyMsgPtr;
	typedef boost::weak_ptr<DevWorkStatusNotify>     devWorkNfyMsgWeakPtr;
	typedef boost::shared_ptr<DevNetStatusNotify>    devNetNfyMsgPtr;
	typedef boost::weak_ptr<DevNetStatusNotify>      devNetNfyMsgWeakPtr;
	typedef boost::shared_ptr<DeviceCommandResultNotify> devCommdRsltPtr;
	typedef boost::weak_ptr<DeviceCommandResultNotify>   devCommdRsltWeakPtr;

	typedef boost::shared_ptr<DeviceCommandMsg> devCommdMsgPtr;
	typedef boost::weak_ptr<DeviceCommandMsg>   devCommdMsgWeakPtr;

	typedef boost::shared_ptr<CommandParam>             commandParamPtr;
	typedef boost::weak_ptr<CommandParam>               commandParamWeakPtr;

    //typedef boost::shared_ptr<vector<commandParamPtr>>             ;
    //typedef boost::weak_ptr<vector<commandParamPtr>>               vCommandParamWeakPtr;

    //typedef boost::shared_ptr<TransmitterPropertyEx> pTransmitterPropertyExPtr;
    //typedef boost::shared_ptr<Data> DevMonitorDataPtr;
    //typedef boost::shared_ptr<MoxaPropertyEx> pMoxaPropertyExPtr;
}
#endif
