#include "nettool.h"
#ifdef Q_WS_X11
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <error.h>
#include <net/route.h>
#elif Q_WS_WIN

#endif
#include<string.h>

netTool::netTool()
{

}

int netTool::get_gw_ip(char *eth, char *ipaddr)
{
    #ifdef Q_WS_X11
    int sock_fd;
    struct  sockaddr_in my_addr;
    struct ifreq ifr;
    /**//* Get socket file descriptor */
    if ((sock_fd = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
     return -1;
    /**//* Get IP Address */
    strncpy(ifr.ifr_name, eth, IF_NAMESIZE);
    ifr.ifr_name[IFNAMSIZ-1]='/0';

    if (ioctl(sock_fd, SIOCGIFADDR, &ifr) < 0)
     return -1;

    memcpy(&my_addr, &ifr.ifr_addr, sizeof(my_addr));
    strcpy(ipaddr, inet_ntoa(my_addr.sin_addr));
    close(sock_fd);
#endif
    return 0;
}

int netTool::set_If_addr(char *ifname, char *Ipaddr, char *mask,char *gateway)
{

    int fd;
    int rc;

#ifdef Q_WS_X11
    struct ifreq ifr;
    struct sockaddr_in *sin;
    struct rtentry  rt;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd < 0)
           return -1;
    memset(&ifr,0,sizeof(ifr));
    strcpy(ifr.ifr_name,ifname);
    sin = (struct sockaddr_in*)&ifr.ifr_addr;
    sin->sin_family = AF_INET;
    //IP地址
    if(inet_aton(Ipaddr,&(sin->sin_addr)) < 0)
        return -2;
    if(ioctl(fd,SIOCSIFADDR,&ifr) < 0)
        return -3;
    //子网掩码
    if(inet_aton(mask,&(sin->sin_addr)) < 0)
        return -4;
    if(ioctl(fd, SIOCSIFNETMASK, &ifr) < 0)
        return -5;
    //网关
    memset(&rt, 0, sizeof(struct rtentry));
    memset(sin, 0, sizeof(struct sockaddr_in));
    sin->sin_family = AF_INET;
    sin->sin_port = 0;
    if(inet_aton(gateway, &sin->sin_addr)<0)
       return -6;
    memcpy ( &rt.rt_gateway, sin, sizeof(struct sockaddr_in));
    ((struct sockaddr_in *)&rt.rt_dst)->sin_family=AF_INET;
    ((struct sockaddr_in *)&rt.rt_genmask)->sin_family=AF_INET;
    rt.rt_flags = RTF_GATEWAY;
    if (ioctl(fd, SIOCADDRT, &rt)<0)
    {
        close(fd);
        return -1;
    }
    close(fd);
#endif
    return rc;
}
