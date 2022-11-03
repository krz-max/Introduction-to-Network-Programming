#include "stdlib.h"
#include "server.h"
#define LIST 10003
#define NAMES 10006
#define USERS 10008
#define NICK 10000
#define USER 10001
#define PING 10002
#define JOIN 10004
#define TOPIC 10005
#define PART 10007
#define PRIVMSG 10009
#define LEAVE 10010
#define UNKNOWN 10011


void nick(std::list<std::string> &arg_str, UserInfo *fd);
void user(std::list<std::string> &arg_str, UserInfo *fd);
void ping(std::list<std::string> &arg_str, UserInfo *fd);
void list(std::list<std::string> &arg_str, UserInfo *fd);
void join(std::list<std::string> &arg_str, UserInfo *fd);
void topic(std::list<std::string> &arg_str, UserInfo *fd);
void names(std::list<std::string> &arg_str, UserInfo *fd);
void part(std::list<std::string> &arg_str, UserInfo *fd);
void users(std::list<std::string> &arg_str, UserInfo *fd);
void privmsg(std::list<std::string> &arg_str, UserInfo *fd);
void quit(std::list<std::string> &arg_str, UserInfo *fd);

typedef void (*FuncPtr)(std::list<std::string> &arg_str, UserInfo *fd);

std::unordered_map<std::string, FuncPtr> kCommandFuntions = {
    {"NICK", nick},
    {"USER", user},
    {"PING", ping},
    {"LIST", list},
    {"JOIN", join},
    {"TOPIC", topic},
    {"NAMES", names},
    {"PART", part},
    {"USERS", users},
    {"PRIVMSG", privmsg},
    {"QUIT", quit}
};