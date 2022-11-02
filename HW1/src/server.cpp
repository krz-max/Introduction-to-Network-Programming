#include "server.h"

Server::Server(in_port_t port, in_addr_t addr)
{
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = port;
    servaddr.sin_addr.s_addr = addr;
    maxconn = -1;
    howmanyusers = 0;
    bzero(CLen, sizeof(CLen));
    bzero(CAddr, sizeof(CAddr));
    bzero(nameStr, sizeof(nameStr));
}
void Server::setup()
{
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    // wrapper required
    int flag = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int));

    client[0].fd = listenfd;
    client[0].events = POLLRDNORM;
    for (int i = 1; i < MAXUSER; i++)
        client[i].fd = -1; /* -1 indicates available entry */
    maxi = 0;
}
void Server::start()
{
    Bind(listenfd, (SA *)&servaddr, sizeof(servaddr));
    Listen(listenfd, 1024);
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
    nready = poll(client, maxi+1, INFTIM);
}

void Server::IncommingReq(int &nready)
{
    if(nready == 0)
        return ;
    char ip[INET_ADDRSTRLEN];
    int connfd, i;
    socklen_t clilen;
    struct sockaddr_in cliaddr;
    if (client[0].revents & POLLRDNORM)
    {
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


        client[i].events = POLLRDNORM;
        CLen[i] = clilen;
        CAddr[i] = cliaddr;
        if (i > maxi)
            maxi = i; /* max index in client[] .fdarray */

        if (--nready <= 0)
            return ; /* no more readable descriptors */
    }
}
void Server::ConnectedReq(int &nready)
{
    if(nready <= 0)
        return ;
    int nowfd, n, i;
    char buf[kBufSize];
    std::stringstream ss;
    std::list<std::string> cmd;
    for (i = 1; i <= maxi; i++)
    { /* check all clients for data */
        if ((nowfd = client[i].fd) < 0)
            continue;
        if (client[i].revents & (POLLRDNORM | POLLERR))
        {
            memset(buf, 0, kBufSize);
            if ((n = read(nowfd, buf, MAXLINE)) < 0)
            {
                if (errno == ECONNRESET)
                    CloseConnection(nowfd, i);
                else
                    err_sys("read error");
            }
            else if (n == 0)
                CloseConnection(nowfd, i);
            else
            {
                ss.str("");
                ss.clear();
                /* 
                buf is received from weechat client
                example messages are
                USER
                USERS
                JOIN ...
                 */                
                ss.str(buf);

                if (ParseCommand(ss, &client[i], cmd))
                    CloseConnection(client[i].fd, i);
            }

            if (--nready <= 0)
                break; /* no more readable descriptors */
        }
    }
}
void Server::CloseConnection(int &sockfd, int &connidx)
{
    Close(sockfd);
    client[connidx].fd = -1;
}