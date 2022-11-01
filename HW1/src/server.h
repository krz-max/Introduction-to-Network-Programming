#ifndef VECTOR_HEADER
#define VECTOR_HEADER

#include <vector>
#endif

#ifndef STRING_HEADER
#define STRING_HEADER

#include <cstring>
#include <string>
#include <sstream>
#endif

#ifndef SOCKET_HEADER
#define SOCKET_HEADER

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#endif

const int kBufSize = 1024;
const std::string kShellTitle = "% ";

inline void WelcomeMsg(int connfd)
{
    const std::string msg[3] = {
        "********************************\n",
        "** Welcome to the BBS server. **\n",
        "********************************\n"};

    for (int i = 0; i < 3; ++i)
        send(connfd, msg[i].c_str(), msg[i].size(), 0);
}
class Server
{
public:
    Server(in_port_t Port, in_addr_t addr);
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
    fd_set rset, allset;

    void MyPoll(int &);
};
Server::Server(in_port_t port, in_addr_t addr)
{
    Bzero(&servaddr, sizeof(servaddr));
    SAddr.sin_family = AF_INET;
    SAddr.sin_port = port;
    SAddr.sin_addr.s_addr = addr;
    maxconn = -1;
    howmanyusers = 0;
    memset(CLen, 0, sizeof(CLen));
    memset(CAddr, 0, sizeof(CAddr));
    memset(nameStr, 0, sizeof(nameStr));
}
void Server::setup()
{
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    // wrapper required
    int flag = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int));

    client[0].fd = listenfd;
    client[0].events = POLLRDNORM;
    for (i = 1; i < MAXUSER; i++)
        client[i].fd = -1; /* -1 indicates available entry */
    maxi = 0;
}
void Server::start()
{
    Bind(listenfd, (SA *)&servaddr, sizeof(servaddr));
    Listen(listenfd, SOMAXCONN);

    for (;;)
    {
        int nready;
        MyPoll(nready);

        if (nready == -1 && errno == EINTR)
            continue;

        IncommingReq(nready);
        ConnectedReq(nready);
    }
}
void Server::MyPoll(int &nready)
{
    nready = Poll(client, maxi, INFTIM);
}

void Server::IncommingReq(int &nready)
{
    if (client[0].revents & POLLRDNORM)
    {
        int connfd;
        int i;
        socklen_t clilen;
        struct sockaddr_in cliaddr;
        clilen = sizeof(cliaddr);
        connfd = Accept(listenfd, (sockaddr *)&cliaddr, &clilen);
        for (i = 1; i < 1024; i++)
            if (client[i].fd < 0)
            {
                client[i].fd = connfd; /* save descriptor */
                break;
            }
        if (i == 1024)
            fprintf(stderr, "too many clients");

        Inet_ntop(AF_INET, (sockaddr *)&cliaddr.sin_addr, ip, INET_ADDRSTRLEN);
        printf("* client connected from %s:%d\n", ip, cliaddr.sin_port);
        howManyUsers++;

        client[i].events = POLLRDNORM;
        CLen[i] = clilen;
        CAddr[i] = cliaddr;
        if (i > maxi)
            maxi = i; /* max index in client[] .fdarray */

        strcpy(nameStr[i], "kkmelon");
        Notify(connfd, i, WELCOME);
        Notify(connfd, i, ARRIVE);

        if (--nready <= 0)
            continue; /* no more readable descriptors */
    }
}
void Server::ConnectedReq(int &nready)
{
    int nowfd;
    char buf[kBufSize];
    std::stringstream ss;
    std::list<std::string> cmd;
    for (i = 1; i <= maxi; i++)
    { /* check all clients for data */
        if ((nowfd = client[i].fd) < 0)
            continue;
        if (client[i].revents & (POLLRDNORM | POLLERR))
        {
            bzero(buf, kBufSize);
            if ((n = read(nowfd, buf, MAXLINE)) < 0)
            {
                if (errno == ECONNRESET)
                {
                    /*4connection reset by client */
                    Close(nowfd);
                    client[i].fd = -1;
                }
                else
                    err_sys("read error");
            }
            else if (n == 0)
            {
                /*4connection closed by client */
                Close(nowfd);
                client[i].fd = -1;
                Inet_ntop(AF_INET, (sockaddr *)&CAddr[i].sin_addr, ip, INET_ADDRSTRLEN);
                printf("* client %s:%d disconnected\n", ip, CAddr[i].sin_port);
                Notify(connfd, i, LEAVE);
                howManyUsers--;
            }
            else
            {
                ss.str("");
                ss.clear();

                ss << buf;

                if (ParseCommand(ss, s, cmd))
                    CloseConnection(s->fd, i);

                PrintShellTitle(s->fd);
            }

            if (--nready <= 0)
                break; /* no more readable descriptors */
        }
    }
}
void Server::CloseConnection(int &sockfd, int &connidx)
{
    FD_CLR(sockfd, &allset);
    client[connidx] = nullptr;
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
}