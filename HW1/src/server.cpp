#include "server.h"
inline std::string &rtrim(std::string &s)
{
    if (s.empty())
        return s;

    s.erase(s.find_last_not_of(" \n\r") + 1);
    return s;
}
void Parser(std::stringstream &ss, std::list<std::string> &args)
{
    std::string str;
    while (std::getline(ss, str, ' '))
    {
        if (str[0] == ':') // last argument (could contain spaces)
        {
            std::string remainder;
            if (std::getline(ss, remainder))
                str = str.substr(1) + ' ' + remainder;
            args.push_back(rtrim(str));
            return;
        }
        else
        {
            args.push_back(rtrim(str));
        }
    }
}
// parse command and do action
// return :
//      true: close connection
//      false: keep reading
bool Server::ParseCommand(std::stringstream &ss, int &uid, std::list<std::string> &cmd)
{
    Parser(ss, cmd);
    fprintf(stdout, "cmd size: %ld\n", cmd.size());
    for (std::string it : cmd)
        fprintf(stdout, "parsed cmd: %s\n", it.c_str());

    if (cmd.empty())
        return false;
    std::string identifier = cmd.front();
    cmd.pop_front();
    if (identifier == CmdType::IRC_QUIT)
    {
        quit(cmd, uid);
        return true;
    }
    else if (identifier == CmdType::IRC_NICK)
    {
        nick(cmd, uid);
    }
    else if (identifier == CmdType::IRC_USER)
    {
        user(cmd, uid);
    }
    else if (identifier == CmdType::IRC_USERS)
    {
        users(cmd, uid);
    }
    else if (identifier == CmdType::IRC_JOIN)
    {
        join(cmd, uid);
    }
    else if (identifier == CmdType::IRC_LIST)
    {
        list(cmd, uid);
    }
    else if (identifier == CmdType::IRC_TOPIC)
    {
        topic(cmd, uid);
    }
    return false;
}

Server::Server(in_port_t port, in_addr_t addr)
{
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = port;
    servaddr.sin_addr.s_addr = addr;
    maxconn = -1;
}
void Server::setup()
{
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    // wrapper required
    int flag = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int));

    maxfd = listenfd;
    kchannel = 0;
    kusers = 0;
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    for (int i = 0; i < MAXUSER; i++)
        client[i] = nullptr; /* -1 indicates available entry */
    for (int i = 0; i < MAXCHAN; ++i)
        channels[i] = nullptr;
}
void Server::start()
{
    Bind(listenfd, (SA *)&servaddr, sizeof(servaddr));
    Listen(listenfd, SOMAXCONN);
    for (;;)
    {
        int nready;
        MySelect(nready);

        if (nready == -1 && errno == EINTR)
            continue;

        IncommingReq(nready);
        ConnectedReq(nready);
    }
}
/**
 * @brief setup select bits and select
 *
 * @param nready number of descriptors that are ready to I/O
 */
void Server::MySelect(int &nready)
{
    rset = allset;
    nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
}
void Server::Connection(int &uid)
{
    std::stringstream ss;
    std::list<std::string> cmd;
    char buf[MAXLINE];
    Readline(client[uid]->fd, buf, sizeof(buf)); // NICK <nickname>/r/n
    ss.str(buf);
    ParseCommand(ss, uid, cmd);
    int n = Readline(client[uid]->fd, buf, sizeof(buf));
    if (n > 0)
    { // USER <username> <hostname> <servername> :<realname>
        ss.str("");
        ss.clear();
        ss.str(buf);
        ParseCommand(ss, uid, cmd);
    }
}
/**
 * @brief handle incomming requests, and accept connections
 *
 * @param nready number of descriptors that are ready to I/O
 */
void Server::Server::IncommingReq(int &nready)
{
    if (nready == 0)
        return;
    if (FD_ISSET(listenfd, &rset))
    {
        int i;
        int connfd;
        struct sockaddr_in cliaddr;
        socklen_t len = sizeof(cliaddr);

        connfd = Accept(listenfd, (SA *)&cliaddr, &len);

        for (i = 0; i < FD_SETSIZE; ++i)
        {
            if (client[i] == nullptr)
            {
                client[i] = new UserInfo(connfd, len, cliaddr);
                break;
            }
        }
        if (i == FD_SETSIZE)
            fprintf(stderr, "too many clients\r\n");

        FD_SET(connfd, &allset);

        // ParseCommand(ss, client[i], cmd);
        Connection(i);

        maxfd = std::max(maxfd, connfd);
        maxconn = std::max(maxconn, i);
        --nready;
    }
}

/**
 * @brief handle requests from connected socket
 *
 * @param nready number of descriptors that are ready to I/O
 */
void Server::Server::ConnectedReq(int &nready)
{
    if (nready <= 0)
        return;
    UserInfo *s;
    int n;
    char buf[MAXLINE];
    std::stringstream ss;
    std::list<std::string> cmd;
    for (int i = 0; i <= maxconn; i++)
    { /* check all clients for data */
        if ((s = client[i]) == nullptr)
            continue;
        if (FD_ISSET(s->fd, &rset))
        {
            bzero(buf, MAXLINE);
            if ((n = read(s->fd, buf, MAXLINE)) == 0)
            {
                fprintf(stdout, "Disconnected\r\n");
                CloseConnection(client[i]->fd, i);
            }
            else
            {
                ss.str("");
                ss.clear();
                // dprintf(sockfd, "%s\r\n", buf);
                fprintf(stdout, "%s\r\n", buf);
                ss.str(buf);
                if (ParseCommand(ss, i, cmd))
                    CloseConnection(s->fd, i);
            }

            if (--nready <= 0)
                break; /* no more readable descriptors */
        }
    }
}

/**
 * @brief close existed socket connection
 *
 * @param sockfd the socket descriptor to close
 * @param connidx index in "client" array
 */
void Server::Server::CloseConnection(int &sockfd, int &connidx)
{
    FD_CLR(sockfd, &allset);
    client[connidx] = nullptr;
    // shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
}

// command function
void Server::nick(std::list<std::string> &arg_str, int &uid)
{
    std::string nickname;
    nickname = arg_str.front();
    arg_str.pop_front();
    client[uid]->UserID.nickname = nickname;
    return;
}
void Server::user(std::list<std::string> &arg_str, int &uid)
{
    std::string username, servername, hostname, realname;
    servername = arg_str.front();
    arg_str.pop_front();
    username = arg_str.front();
    arg_str.pop_front();
    hostname = arg_str.front();
    arg_str.pop_front();
    realname = arg_str.front();
    arg_str.pop_front();
    client[uid]->UserID.servername = servername;
    client[uid]->UserID.hostname = hostname;
    client[uid]->UserID.realname = realname;
    client[uid]->UserID.username = username;
    kusers = (client[uid]->isLogin) ? kusers + 1 : kusers;
    client[uid]->isLogin = true;
    dprintf(client[uid]->fd, ":mircd 001 %s :Welcome to the minimized IRC daemon!\r\n", client[uid]->UserID.nickname.c_str());
    dprintf(client[uid]->fd, ":mircd 251 %s :There are 1 users and 0 invisible on 1 server\r\n", client[uid]->UserID.nickname.c_str());
    dprintf(client[uid]->fd, ":mircd 375 %s :- mircd Message of the day -\r\n", client[uid]->UserID.nickname.c_str());
    dprintf(client[uid]->fd, ":mircd 372 %s :-  Hello, World!\r\n", client[uid]->UserID.nickname.c_str());
    dprintf(client[uid]->fd, ":mircd 372 %s :-               @                    _ \r\n", client[uid]->UserID.nickname.c_str());
    dprintf(client[uid]->fd, ":mircd 372 %s :-   ____  ___   _   _ _   ____.     | |\r\n", client[uid]->UserID.nickname.c_str());
    dprintf(client[uid]->fd, ":mircd 372 %s :-  /  _ `'_  \\ | | | '_/ /  __|  ___| |\r\n", client[uid]->UserID.nickname.c_str());
    dprintf(client[uid]->fd, ":mircd 372 %s :-  | | | | | | | | | |   | |    /  _  |\r\n", client[uid]->UserID.nickname.c_str());
    dprintf(client[uid]->fd, ":mircd 372 %s :-  | | | | | | | | | |   | |__  | |_| |\r\n", client[uid]->UserID.nickname.c_str());
    dprintf(client[uid]->fd, ":mircd 372 %s :-  |_| |_| |_| |_| |_|   \\____| \\___,_|\r\n", client[uid]->UserID.nickname.c_str());
    dprintf(client[uid]->fd, ":mircd 372 %s :-  minimized internet relay chat daemon\r\n", client[uid]->UserID.nickname.c_str());
    dprintf(client[uid]->fd, ":mircd 372 %s :-\r\n", client[uid]->UserID.nickname.c_str());
    dprintf(client[uid]->fd, ":mircd 376 %s :End of message of the day\r\n", client[uid]->UserID.nickname.c_str());
    return;
}
void Server::list(std::list<std::string> &arg_str, int &uid)
{
    if (kchannel == 0)
        return;
    dprintf(client[uid]->fd, ":mircd 321 %s Channel  :Users  Name\r\n", client[uid]->UserID.nickname.c_str());
    for (int i = 0; i < kchannel; i++)
    {
        dprintf(client[uid]->fd, ":mircd 322 %s %s %d :%s\r\n", client[uid]->UserID.nickname.c_str(), channels[i]->name.c_str(), channels[i]->nusers, channels[i]->topic.c_str());
    }
    dprintf(client[uid]->fd, ":mircd 323 %s :End of /LIST\r\n", client[uid]->UserID.nickname.c_str());
    return;
}
int Server::findChannel(const std::string &target)
{
    for (int i = 0; i < kchannel; i++)
    {
        if (channels[i]->name == target)
            return i;
    }
    return -1;
}
void Server::join(std::list<std::string> &arg_str, int &uid)
{
    std::string channelname = arg_str.front();
    arg_str.pop_front();
    int idx = findChannel(channelname);
    if (idx == -1)
    {
        // new channel created
        client[uid]->chanID = kchannel;
        channels[kchannel++] = new Channel(channelname);
        dprintf(client[uid]->fd, ":%s JOIN #%s\r\n", client[uid]->UserID.nickname.c_str(), channelname.c_str());
        dprintf(client[uid]->fd, ":mircd 331 %s %s :No topic is set\r\n", client[uid]->UserID.nickname.c_str(), channelname.c_str());
        dprintf(client[uid]->fd, ":mircd 353 %s %s :%s\r\n", client[uid]->UserID.nickname.c_str(), channelname.c_str(), client[uid]->UserID.nickname.c_str());
        dprintf(client[uid]->fd, ":mircd 366 %s %s :End of Names List\r\n", client[uid]->UserID.nickname.c_str(), channelname.c_str());
    }
    else
    {
        // channel already exist
        client[uid]->chanID = idx;
        dprintf(client[uid]->fd, ":%s JOIN %s\r\n", client[uid]->UserID.nickname.c_str(), channelname.c_str());
        if (channels[idx]->topic == "")
            dprintf(client[uid]->fd, ":mircd 331 %s %s :No topic is set\r\n", client[uid]->UserID.nickname.c_str(), channelname.c_str());
        else
            dprintf(client[uid]->fd, ":mircd 332 %s %s :%s\r\n", client[uid]->UserID.nickname.c_str(), channelname.c_str(), channels[idx]->topic.c_str());
        for (int i = 0; i <= maxconn; ++i)
            if (client[i]->chanID == idx)
                dprintf(client[uid]->fd, ":mircd 353 %s %s :%s\r\n", client[uid]->UserID.nickname.c_str(), channelname.c_str(), client[i]->UserID.nickname.c_str());
        dprintf(client[uid]->fd, ":mircd 366 %s %s :End of Names List\r\n", client[uid]->UserID.nickname.c_str(), channelname.c_str());
    }
    return;
}
void Server::users(std::list<std::string> &arg_str, int &uid)
{
    dprintf(client[uid]->fd, ":mircd 392 %s :UserID   Terminal   Host\r\n", client[uid]->UserID.nickname.c_str());
    for (int i = 0; i <= maxconn; ++i)
    {
        if (client[i] != nullptr)
            dprintf(client[uid]->fd, ":mircd 393 %s :%-8s %-9s %-8s\r\n", client[uid]->UserID.nickname.c_str(), client[i]->UserID.nickname.c_str(), DefaultMsg::dash.c_str(), client[i]->getIP().c_str());
    }
    dprintf(client[uid]->fd, ":mircd 394 %s :End of users\r\n", client[uid]->UserID.nickname.c_str());
    return;
}

// not done yet
void Server::ping(std::list<std::string> &arg_str, int &uid)
{
    return;
}
void Server::quit(std::list<std::string> &arg_str, int &uid)
{
    kusers--;
    while (!arg_str.empty())
        arg_str.pop_front();
    return;
}
void Server::topic(std::list<std::string> &arg_str, int &uid)
{
    // show the topic of a channel
    std::string channelname = arg_str.front();
    arg_str.pop_front();
    int channelidx = findChannel(channelname);
    if (arg_str.size() == 1)
    {
        dprintf(client[uid]->fd, ":mircd 331 %s %s :No topic is set", client[uid]->UserID.nickname.c_str(), channels[channelidx]->name.c_str());
    }
    // change the topic ~
    else
    {
        std::string newtopic = arg_str.front();
        arg_str.pop_front();
        dprintf(client[uid]->fd, ":mircd 332 %s %s :%s\r\n", client[uid]->UserID.nickname.c_str(), channels[channelidx]->name.c_str(), newtopic.c_str());
    }
    return;
}

void Server::names(std::list<std::string> &arg_str, int &uid)
{
    return;
}
void Server::part(std::list<std::string> &arg_str, int &uid)
{
    return;
}
void Server::privmsg(std::list<std::string> &arg_str, int &uid)
{
    return;
}