#ifndef NETTOOL_H
#define NETTOOL_H



class netTool
{
public:
    netTool();
    static int get_gw_ip(char *eth, char *ipaddr);
    static int set_If_addr(char *ifname, char *Ipaddr, char *mask,char *gateway);
};

#endif // NETTOOL_H
