#include "stdlib.h"
#include "unixwrapper.cpp"
#include "socketwrapper.cpp"

#define MAXUSER FD_SETSIZE
#define MAXCHAN 1024
#define MAXLINE 2000
#define NAMELEN 20

namespace CmdType
{
    const std::string IRC_QUIT = "QUIT";
    const std::string IRC_NICK = "NICK";
    const std::string IRC_USER = "USER";
    const std::string IRC_PING = "PING";
    const std::string IRC_LIST = "LIST";
    const std::string IRC_JOIN = "JOIN";
    const std::string IRC_TOPIC = "TOPIC";
    const std::string IRC_NAMES = "NAMES";
    const std::string IRC_PART = "PART";
    const std::string IRC_USERS = "USERS";
    const std::string IRC_PRIVMSG = "PRIVMSG";
};
namespace ResponseCode
{
    // Reply Message :
    const std::string RPL_WELCOME = "001";
    const std::string RPL_SERVMSG = "251";
    const std::string RPL_LISTSTART = "321";
    const std::string RPL_LIST = "322";
    const std::string RPL_LISTEND = "323";
    const std::string RPL_NOTOPIC = "331";
    const std::string RPL_TOPIC = "332";
    const std::string RPL_NAMREPLY = "353";
    const std::string RPL_ENDOFNAMES = "366";
    const std::string RPL_MOTD = "372";
    const std::string RPL_MOTDSTART = "375";
    const std::string RPL_ENDOFMOTD = "376";
    const std::string RPL_USERSSTART = "392";
    const std::string RPL_USERS = "393";
    const std::string RPL_ENDOFUSERS = "394";

    // Error Message :
    const std::string ERR_NOSUCHCHANNEL = "403";
    const std::string ERR_UNKNOWNCOMMAND = "421";
    const std::string ERR_NICKCOLLISION = "436";
    const std::string ERR_NOTONCHANNEL = "442";
    // Not sure :
    const std::string ERR_NONICKNAMEGIVEN = "431";
    const std::string ERR_NOSUCHNICK = "401";
    const std::string ERR_NORECIPIENT = "411";
    const std::string ERR_NOTEXTTOSEND = "412";
    const std::string ERR_NOTREGISTERED = "451";
    const std::string ERR_NEEDMOREPARAMS = "461";
};
namespace DefaultMsg
{
    const std::string dash = "-";
};
struct NameInfo
{
    std::string nickname = "";
    std::string username = "";
    std::string hostname = "";
    std::string servername = "";
    std::string realname = "";
};
struct UserInfo
{
    int fd;
    int chanID = -1;
    NameInfo UserID;
    socklen_t CLen;
    sockaddr_in CAddr;
    bool isLogin = false;

    UserInfo(int connfd, socklen_t clilen, sockaddr_in cliaddr) : fd(connfd), CLen(clilen), CAddr(cliaddr) {}
    std::string getIP()
    {
        char ip[INET_ADDRSTRLEN];
        Inet_ntop(AF_INET, (SA *)&CAddr.sin_addr, ip, INET_ADDRSTRLEN);
        return (!strcmp(ip, "127.0.0.1")) ? "localhost" : ip;
    }
};
struct Channel
{
    int nusers;
    std::string name;
    std::string topic = "";
    Channel(std::string name) : nusers(1), name(name) {}
};

class Server
{
public:
    Server(in_port_t port, in_addr_t addr);
    void setup();
    void start();

private:
    struct sockaddr_in servaddr;
    int listenfd;
    int maxfd;
    int maxconn;
    int kchannel;
    int kusers;
    fd_set rset, allset;
    // Session *client[FD_SETSIZE];
    UserInfo *client[FD_SETSIZE];
    Channel *channels[MAXCHAN];
    // std::map<std::string, UserInfo> users;
    // int client[MAXUSER];
    // socklen_t CLen[MAXUSER];
    // struct sockaddr_in CAddr[MAXUSER];
    // char nameStr[MAXUSER][NAMELEN];

    void MySelect(int &);
    void IncommingReq(int &);
    void ConnectedReq(int &);
    void Connection(int &);
    void CloseConnection(int &, int &);

    // command function
    void nick(std::list<std::string> &arg_str, int &uid);
    void user(std::list<std::string> &arg_str, int &uid);
    void ping(std::list<std::string> &arg_str, int &uid);
    void list(std::list<std::string> &arg_str, int &uid);
    void join(std::list<std::string> &arg_str, int &uid);
    void topic(std::list<std::string> &arg_str, int &uid);
    void names(std::list<std::string> &arg_str, int &uid);
    void part(std::list<std::string> &arg_str, int &uid);
    void users(std::list<std::string> &arg_str, int &uid);
    void privmsg(std::list<std::string> &arg_str, int &uid);
    void quit(std::list<std::string> &arg_str, int &uid);
    // parse cmd
    bool ParseCommand(std::stringstream &ss, int &uid, std::list<std::string> &cmd);
    int FindChannel(const std::string &target, int &uid);
    // Error handling
    bool checkNickExist(const std::string &target, int &uid);
};