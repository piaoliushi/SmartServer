#ifndef PARSE_ASS_DEVICE_H
#define PARSE_ASS_DEVICE_H

#include "../../DataTypeDefine.h"
namespace hx_net
{
class Parse_Ass_Device
{
public:
    Parse_Ass_Device(DeviceInfo &devInfo);
protected:
    void _parse_dev_associate_info();
private:
    DeviceInfo    &d_dev_info_;
};
typedef boost::shared_ptr<hx_net::Parse_Ass_Device> parse_ass_dev_ptr;
typedef boost::weak_ptr<hx_net::Parse_Ass_Device>    parse_ass_dev_weak_ptr;
}
#endif // PARSE_ASS_DEVICE_H
