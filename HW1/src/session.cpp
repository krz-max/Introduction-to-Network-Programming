#include "stdlib.h"
#include "socketwrapper.cpp"


struct NameInfo
{
    std::string nickname="";
    std::string username="";
    std::string hostname="";
    std::string servername="";
    std::string realname="";
};
struct UserInfo
{
    int fd;
    NameInfo UserID;
    socklen_t CLen;
    sockaddr_in CAddr;
    bool isLogin = false;

    UserInfo(int connfd, socklen_t clilen, sockaddr_in cliaddr) : fd(connfd), CLen(clilen), CAddr(cliaddr) {}
    std::string getIP()
    {
        char ip[INET_ADDRSTRLEN];
        Inet_ntop(AF_INET, (SA *)&CAddr, ip, INET_ADDRSTRLEN);
        return ip;
    }
};
struct Channel{
    int nusers;
    std::string topic;
    std::string name;
};
