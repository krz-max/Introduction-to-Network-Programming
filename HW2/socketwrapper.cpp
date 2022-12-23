#include "socketwrapper.h"

#define err_quit(m) \
    {               \
        perror(m);  \
        exit(-1);   \
    }
/*
Create a Socket
Prototype:
    int Socket(int family, int type, int protocol);
Example:
    Socket(AF_INET, SOCK_STREAM, 0);
*/
int Socket(int family, int type, int protocol)
{
    int n;
    if ((n = socket(family, type, protocol)) < 0)
        fprintf(stderr, "socket error\n");
    return n;
}
/*
For Server, bind the server to the IP address and port
Prototype:
    int Bind(int sockfd, struct SA *myaddr, socklen_t addrlen);
Example:
    SAddr.sin_family      = AF_INET
    SAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    SAddr.sin_port        = htons(SERV_PORT);
    Bind(sockfd, (SA*)&SAddr, sizeof(SAddr));
*/
void Bind(int sockfd, struct SA *myaddr, socklen_t addrlen)
{
    if (bind(sockfd, myaddr, addrlen) < 0)
        fprintf(stderr, "bind error\n");
    return ;
}
/*
Convert Network Byte Order to "xxx.xxx.xxx.xxx" IP
Prototype:
    const char *Inet_ntop(int family, const void *addrptr, char *str, size_t len)
Example:
    Inet_ntop(AF_INET, (sockaddr*)&CAddr[k].sin_addr, ip, INET_ADDRSTRLEN);
*/
const char *Inet_ntop(int family, const void *addrptr, char *str, size_t len)
{
    const char *result;
    if ((result = inet_ntop(family, addrptr, str, len)) == NULL)
    {
        fprintf(stderr, "inet_ntop error for %ld\n", (size_t)addrptr);
        return NULL;
    }
    return result;
}

int Inet_pton(int family, const char *addr, void *buf)
{
    ssize_t n;
    if ((n = inet_pton(family, addr, buf)) < 0)
    {
        fprintf(stderr, "inet_pton error for %s\n", addr);
        return n;
    }
    return n;
}