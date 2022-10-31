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
    int maxconn;
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
		client[i].fd = -1;			/* -1 indicates available entry */
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
    if (nready == 0)
        return;
    if (FD_ISSET(listenfd, &rset))
    {
        int i;
        int connfd;
        struct sockaddr_in cliaddr;
        socklen_t len = sizeof(cliaddr);

        connfd = accept(listenfd, (SA *)&cliaddr, &len);

        for (i = 0; i < FD_SETSIZE; ++i)
        {
            if (client[i] == nullptr)
            {
                client[i] = new Session(connfd);
                break;
            }
        }
        if (i == FD_SETSIZE)
            std::cerr << "too many clients\n";

        FD_SET(connfd, &allset);

        WelcomeMsg(connfd);
        PrintShellTitle(connfd);

        maxfd = std::max(maxfd, connfd);
        maxconn = std::max(maxconn, i);
        --nready;
    }
}
void Server::ConnectedReq(int &nready)
{
    if (nready <= 0)
        return;

    Session *s;
    char buf[kBufSize];
    std::stringstream ss;
    std::list<std::string> cmd;

    for (int i = 0; i <= maxconn; ++i)
    {
        if ((s = client[i]) == nullptr)
            continue;

        if (FD_ISSET(s->fd, &rset))
        {
            bzero(buf, kBufSize);

            if ((read(s->fd, buf, kBufSize)) == 0)
            {
                CloseConnection(s->fd, i);
            }
            else
            {
                // ! clear ss before read
                ss.str("");
                ss.clear();

                ss << buf;

                if (ParseCommand(ss, s, cmd))
                    CloseConnection(s->fd, i);

                PrintShellTitle(s->fd);
            }

            if (--nready <= 0)
                break;
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