#include "stdlib.h"
#include "parser.cpp"
#include "socketwrapper.cpp"
#include "unixwrapper.cpp"

#define MAXUSER 1024
#define MAXLINE 2000
#define NAMELEN 20

const int kBufSize = 1024;

class Server
{
public:
    Server(in_port_t port, in_addr_t addr);
    void setup();
    void start();

private:
    struct sockaddr_in servaddr;
    int listenfd;
    int maxi;
    struct pollfd client[1024];
    socklen_t CLen[1024];
    struct sockaddr_in CAddr[1024];
    char nameStr[1024][100];
    int maxconn;
    int howmanyusers;
    int nready;
    fd_set rset, allset;

    void MyPoll(int &);
    void IncommingReq(int &);
    void ConnectedReq(int &);
    void CloseConnection(int &, int &);
};

