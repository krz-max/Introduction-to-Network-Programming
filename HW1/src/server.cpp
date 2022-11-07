#include "server.h"
/*
Reply Message :
[x] (321) RPL_LISTSTART
[x] (322) RPL_LIST
[x] (323) RPL_LISTEND
[x] (331) RPL_NOTOPIC
[x] (332) RPL_TOPIC
[x] (353) RPL_NAMREPLY
[x] (366) RPL_ENDOFNAMES
[x] (372) RPL_MOTD
[x] (375) RPL_MOTDSTART
[x] (376) RPL_ENDOFMOTD
[x] (392) RPL_USERSSTART
[x] (393) RPL_USERS
[x] (394) RPL_ENDOFUSERS

Error Message :
[x](403) ERR_NOSUCHCHANNEL // names, part, topic, [x]list.  join & privmsg is always right
[x](421) ERR_UNKNOWNCOMMAND // ~~
[x](436) ERR_NICKCOLLISION // nick
[x](442) ERR_NOTONCHANNEL // topic, part
[*](431) ERR_NONICKNAMEGIVEN // NICK
[*](401) ERR_NOSUCHNICK // PRIVMSG
[*](411) ERR_NORECIPIENT // PRIVMSG
[*](412) ERR_NOTEXTTOSEND // PRIVMSG
[*](451) ERR_NOTREGISTERED // ??
[*](461) ERR_NEEDMOREPARAMS // JOIN, PART, TOPIC
 */

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
            else
                str = str.substr(1);
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
    else if (identifier == CmdType::IRC_PING)
    {
        ping(cmd, uid);
    }
    else if (identifier == CmdType::IRC_NAMES)
    {
        names(cmd, uid);
    }
    else if (identifier == CmdType::IRC_PART)
    {
        part(cmd, uid);
    }
    else if (identifier == CmdType::IRC_PRIVMSG)
    {
        privmsg(cmd, uid);
    }
    else
    {
        dprintf(client[uid]->fd, ":mircd %s %s %s :Unknown command\r\n",
                ResponseCode::ERR_UNKNOWNCOMMAND.c_str(),
                client[uid]->UserID.nickname.c_str(),
                identifier.c_str());
        return false;
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
                while (!cmd.empty())
                    cmd.pop_front();
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
    close(sockfd);
}

bool Server::checkNickExist(const std::string &target, int &uid)
{
    for (int i = 0; i <= maxconn; i++)
    {
        if (client[i]->UserID.nickname == target)
        { // collision
            dprintf(client[uid]->fd, ":mircd %s %s %s :Nickname collision\r\n",
                    ResponseCode::ERR_NICKCOLLISION.c_str(),
                    client[uid]->UserID.nickname.c_str(),
                    target.c_str());
            return true;
        }
    }
    return false;
}
// command function
void Server::nick(std::list<std::string> &arg_str, int &uid)
{
    std::string nickname;
    nickname = arg_str.front();
    arg_str.pop_front();
    if (checkNickExist(nickname, uid))
        return;
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
    kusers = (client[uid]->isLogin) ? kusers : kusers + 1;
    client[uid]->isLogin = true;
    dprintf(client[uid]->fd, ":mircd %s %s :Welcome to the minimized IRC daemon!\r\n",
            ResponseCode::RPL_WELCOME.c_str(),
            client[uid]->UserID.nickname.c_str());
    dprintf(client[uid]->fd, ":mircd %s %s :There are %d users and 0 invisible on 1 server\r\n",
            ResponseCode::RPL_SERVMSG.c_str(),
            client[uid]->UserID.nickname.c_str(),
            kusers);
    dprintf(client[uid]->fd, ":mircd %s %s :- mircd Message of the day -\r\n",
            ResponseCode::RPL_MOTDSTART.c_str(),
            client[uid]->UserID.nickname.c_str());
    dprintf(client[uid]->fd, ":mircd %s %s :-  Hello, World!\r\n", ResponseCode::RPL_MOTD.c_str(), client[uid]->UserID.nickname.c_str());
    dprintf(client[uid]->fd, ":mircd %s %s :-               @                    _ \r\n", ResponseCode::RPL_MOTD.c_str(), client[uid]->UserID.nickname.c_str());
    dprintf(client[uid]->fd, ":mircd %s %s :-   ____  ___   _   _ _   ____.     | |\r\n", ResponseCode::RPL_MOTD.c_str(), client[uid]->UserID.nickname.c_str());
    dprintf(client[uid]->fd, ":mircd %s %s :-  /  _ `'_  \\ | | | '_/ /  __|  ___| |\r\n", ResponseCode::RPL_MOTD.c_str(), client[uid]->UserID.nickname.c_str());
    dprintf(client[uid]->fd, ":mircd %s %s :-  | | | | | | | | | |   | |    /  _  |\r\n", ResponseCode::RPL_MOTD.c_str(), client[uid]->UserID.nickname.c_str());
    dprintf(client[uid]->fd, ":mircd %s %s :-  | | | | | | | | | |   | |__  | |_| |\r\n", ResponseCode::RPL_MOTD.c_str(), client[uid]->UserID.nickname.c_str());
    dprintf(client[uid]->fd, ":mircd %s %s :-  |_| |_| |_| |_| |_|   \\____| \\___,_|\r\n", ResponseCode::RPL_MOTD.c_str(), client[uid]->UserID.nickname.c_str());
    dprintf(client[uid]->fd, ":mircd %s %s :-  minimized internet relay chat daemon\r\n", ResponseCode::RPL_MOTD.c_str(), client[uid]->UserID.nickname.c_str());
    dprintf(client[uid]->fd, ":mircd %s %s :-\r\n", ResponseCode::RPL_MOTD.c_str(), client[uid]->UserID.nickname.c_str());
    dprintf(client[uid]->fd, ":mircd %s %s :End of message of the day\r\n",
            ResponseCode::RPL_ENDOFMOTD.c_str(),
            client[uid]->UserID.nickname.c_str());
    return;
}
void Server::list(std::list<std::string> &arg_str, int &uid)
{
    if (kchannel == 0)
        return;
    dprintf(client[uid]->fd, ":mircd %s %s Channel  :Users  Name\r\n",
            ResponseCode::RPL_LISTSTART.c_str(),
            client[uid]->UserID.nickname.c_str());
    if (arg_str.size() == 1) // list a specific channel
    {
        std::string target = arg_str.front();
        arg_str.pop_front();
        int idx = FindChannel(target, uid);
        dprintf(client[uid]->fd, ":mircd %s %s %s %d :%s\r\n",
                ResponseCode::RPL_LIST.c_str(),
                client[uid]->UserID.nickname.c_str(),
                target.c_str(),
                channels[idx]->nusers,
                channels[idx]->topic.c_str());
        dprintf(client[uid]->fd, ":mircd %s %s :End of /LIST\r\n",
                ResponseCode::RPL_LISTEND.c_str(),
                client[uid]->UserID.nickname.c_str());
        return;
    }
    for (int i = 0; i < kchannel; i++)
    {
        dprintf(client[uid]->fd, ":mircd %s %s %s %d :%s\r\n",
                ResponseCode::RPL_LIST.c_str(),
                client[uid]->UserID.nickname.c_str(),
                channels[i]->name.c_str(),
                channels[i]->nusers,
                channels[i]->topic.c_str());
    }
    dprintf(client[uid]->fd, ":mircd %s %s :End of /LIST\r\n",
            ResponseCode::RPL_LISTEND.c_str(),
            client[uid]->UserID.nickname.c_str());
    return;
}
int Server::FindChannel(const std::string &target, int &uid)
{
    for (int i = 0; i < kchannel; i++)
    {
        if (channels[i]->name == target)
            return i;
    }
    // (403) ERR_NOSUCHCHANNEL
    dprintf(client[uid]->fd, ":mircd %s %s %s :No such channel\r\n",
            ResponseCode::ERR_NOSUCHCHANNEL.c_str(),
            client[uid]->UserID.nickname.c_str(),
            target.c_str());
    return -1;
}
void Server::join(std::list<std::string> &arg_str, int &uid)
{
    std::string channelname = arg_str.front();
    arg_str.pop_front();
    int idx;
    for (idx = 0; idx < kchannel; idx++)
        if (channels[idx]->name == channelname)
            break;
    client[uid]->chanID = idx;
    if (idx == kchannel) // new channel created
        channels[kchannel++] = new Channel(channelname);
    else // channel already exist
        ++channels[idx]->nusers;
    dprintf(client[uid]->fd, ":%s JOIN %s\r\n", client[uid]->UserID.nickname.c_str(), channelname.c_str());
    if (channels[idx]->topic == "")
        dprintf(client[uid]->fd, ":mircd %s %s %s :No topic is set\r\n",
                ResponseCode::RPL_NOTOPIC.c_str(),
                client[uid]->UserID.nickname.c_str(),
                channelname.c_str());
    else
        dprintf(client[uid]->fd, ":mircd %s %s %s :%s\r\n",
                ResponseCode::RPL_TOPIC.c_str(),
                client[uid]->UserID.nickname.c_str(),
                channelname.c_str(),
                channels[idx]->topic.c_str());
    for (int i = 0; i <= maxconn; ++i)
        if (client[i]->chanID == idx)
            dprintf(client[uid]->fd, ":mircd %s %s %s :%s\r\n",
                    ResponseCode::RPL_NAMREPLY.c_str(),
                    client[uid]->UserID.nickname.c_str(),
                    channelname.c_str(),
                    client[i]->UserID.nickname.c_str());
    dprintf(client[uid]->fd, ":mircd %s %s %s :End of Names List\r\n",
            ResponseCode::RPL_ENDOFNAMES.c_str(),
            client[uid]->UserID.nickname.c_str(),
            channelname.c_str());
    return;
}
void Server::users(std::list<std::string> &arg_str, int &uid)
{
    dprintf(client[uid]->fd, ":mircd %s %s :UserID   Terminal   Host\r\n",
            ResponseCode::RPL_USERSSTART.c_str(),
            client[uid]->UserID.nickname.c_str());
    for (int i = 0; i <= maxconn; ++i)
        if (client[i] != nullptr)
            dprintf(client[uid]->fd, ":mircd %s %s :%-8s %-9s %-8s\r\n",
                    ResponseCode::RPL_USERS.c_str(),
                    client[uid]->UserID.nickname.c_str(),
                    client[i]->UserID.nickname.c_str(),
                    DefaultMsg::dash.c_str(),
                    client[i]->getIP().c_str());
    dprintf(client[uid]->fd, ":mircd %s %s :End of users\r\n",
            ResponseCode::RPL_ENDOFUSERS.c_str(),
            client[uid]->UserID.nickname.c_str());
    return;
}

// not done yet
void Server::ping(std::list<std::string> &arg_str, int &uid)
{
    dprintf(client[uid]->fd, "PONG %s\r\n", client[uid]->getIP().c_str());
    return;
}
void Server::quit(std::list<std::string> &arg_str, int &uid)
{
    kusers--;
    if (client[uid]->chanID != -1)
    {
        --channels[client[uid]->chanID]->nusers;
    }
    while (!arg_str.empty())
        arg_str.pop_front();
    return;
}
void Server::topic(std::list<std::string> &arg_str, int &uid)
{
    std::string channelname = arg_str.front();
    arg_str.pop_front();
    int channelidx = FindChannel(channelname, uid);
    if (channelidx == -1)
        return;
    // show the topic of a channel
    if (arg_str.size() == 0)
    {
        if (channels[channelidx]->topic == "") // no topic
            dprintf(client[uid]->fd, ":mircd %s %s %s :No topic is set\r\n",
                    ResponseCode::RPL_NOTOPIC.c_str(),
                    client[uid]->UserID.nickname.c_str(),
                    channels[channelidx]->name.c_str());
        else
            dprintf(client[uid]->fd, ":mircd %s %s %s :%s\r\n",
                    ResponseCode::RPL_TOPIC.c_str(),
                    client[uid]->UserID.nickname.c_str(),
                    channels[channelidx]->name.c_str(),
                    channels[channelidx]->topic.c_str());
    }
    // change the topic ~
    else
    {
        std::string newtopic = arg_str.front();
        arg_str.pop_front();
        if (channelidx != client[uid]->chanID)
        {
            dprintf(client[uid]->fd, ":mircd %s %s :You're not on that channel\r\n",
                    ResponseCode::ERR_NOTONCHANNEL.c_str(),
                    channelname.c_str());
            return;
        }
        channels[channelidx]->topic = newtopic;
        dprintf(client[uid]->fd, ":mircd %s %s %s :%s\r\n",
                ResponseCode::RPL_TOPIC.c_str(),
                client[uid]->UserID.nickname.c_str(),
                channels[channelidx]->name.c_str(),
                channels[channelidx]->topic.c_str());
    }
    return;
}
// channel command
void Server::names(std::list<std::string> &arg_str, int &uid)
{
    std::string targetname = arg_str.front();
    arg_str.pop_front();
    int channelidx = FindChannel(targetname, uid);
    if (channelidx == -1)
        return;
    for (int i = 0; i <= maxconn; i++)
        if (client[i] != nullptr)
            if (client[i]->chanID == channelidx)
                dprintf(client[uid]->fd, ":mircd %s %s %s :%s\r\n",
                        ResponseCode::RPL_NAMREPLY.c_str(),
                        client[i]->UserID.nickname.c_str(),
                        channels[channelidx]->name.c_str(),
                        client[i]->UserID.nickname.c_str());
    dprintf(client[uid]->fd, ":mircd %s %s %s :End of Names List\r\n",
            ResponseCode::RPL_ENDOFNAMES.c_str(),
            client[uid]->UserID.nickname.c_str(),
            channels[channelidx]->name.c_str());
    return;
}
void Server::part(std::list<std::string> &arg_str, int &uid)
{
    std::string targetname = arg_str.front();
    arg_str.pop_front();
    arg_str.pop_front(); // discard the remaining args
    int channelidx = FindChannel(targetname, uid);
    if (channelidx == -1)
        return;
    if (client[uid]->chanID != channelidx)
    {
        dprintf(client[uid]->fd, ":mircd %s %s %s :You're not on that channel\r\n",
            ResponseCode::ERR_NOTONCHANNEL.c_str(),
            client[uid]->UserID.nickname.c_str(),
            targetname.c_str());
        return;
    }
    client[uid]->chanID = -1;
    --channels[channelidx]->nusers;
    dprintf(client[uid]->fd, ":%s PART :%s\r\n",
            client[uid]->UserID.nickname.c_str(),
            channels[channelidx]->name.c_str());
    return;
}
void Server::privmsg(std::list<std::string> &arg_str, int &uid)
{
    std::string targetname = arg_str.front();
    arg_str.pop_front();
    std::string textmsg = arg_str.front();
    arg_str.pop_front(); // get the user input
    int channelidx = FindChannel(targetname, uid);
    if (channelidx == -1) // no such channel
        return;
    else
        for (int i = 0; i <= maxconn; i++)
            if (client[i] != nullptr)
                if (client[i]->chanID == channelidx && i != uid)
                    dprintf(client[i]->fd, ":%s PRIVMSG %s :%s\r\n",
                            client[uid]->UserID.nickname.c_str(),
                            channels[channelidx]->name.c_str(),
                            textmsg.c_str());
    return;
}