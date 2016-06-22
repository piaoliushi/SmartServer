#include "parse_ass_device.h"

namespace hx_net
{


Parse_Ass_Device::Parse_Ass_Device(DeviceInfo &devInfo)
    :d_dev_info_(devInfo)
{
    _parse_dev_associate_info();
}

void Parse_Ass_Device::_parse_dev_associate_info()
{

}

bool Parse_Ass_Device::get_parent_device_id(string &parentId)
{
    if(d_dev_info_.bMulChannel)
        return false;
   map<int,vector<AssDevChan> >::iterator iter =  d_dev_info_.map_AssDevChan.find(0);
   if(iter!=d_dev_info_.map_AssDevChan.end()){
       vector<AssDevChan>::iterator iter_ass = iter->second.begin();
       for(;iter_ass!=iter->second.end();++iter_ass){
           if((*iter_ass).iAssType == MOUDLE_TO_DEV){
               parentId = (*iter_ass).sAstNum;
               return true;
           }
       }
   }

}

bool Parse_Ass_Device::get_parent_device_id_by_channel(const int nChannel,string &parentId)
{
    if(!d_dev_info_.bMulChannel)
        return false;
    map<int,vector<AssDevChan> >::iterator iter =  d_dev_info_.map_AssDevChan.find(nChannel);
    if(iter!=d_dev_info_.map_AssDevChan.end()){
        vector<AssDevChan>::iterator iter_ass = iter->second.begin();
        for(;iter_ass!=iter->second.end();++iter_ass){
            if((*iter_ass).iAssType == MOUDLE_TO_DEV){
                parentId = (*iter_ass).sAstNum;
                return true;
            }
        }
    }
}

}
