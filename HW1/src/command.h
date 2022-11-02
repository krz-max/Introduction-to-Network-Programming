#include "stdlib.h"

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

namespace PromptMsg
{
    const std::string kRegSuccess = "Register successfully.\n";
    const std::string kLoginSuccess = "Welcome, ";
    const std::string kLogoutSuccess = "Bye, ";
    const std::string kEmptyMailBox = "Your message box is empty.\n";
};

namespace ErrorMsg
{
    const std::string kRegUsageError = "Usage: register <username> <password>\n";
    const std::string kRegDupUserError = "Username is already used.\n";
    const std::string kLoginUsageError = "Usage: login <username> <password>\n";
    const std::string kNoLogoutError = "Please logout first.\n";
    const std::string kLoginFailError = "Login failed.\n";
    const std::string kNoLoginError = "Please Login first.\n";
    const std::string kSendUsageError = "Usage: send <username> <message>\n";
    const std::string kNoUserError = "User not existed.\n";
    const std::string kReceiveUsageError = "Usage: receive <username>\n";
};

void Register(std::list<std::string> &arg_str, int fd);
void Login(std::list<std::string> &arg_str, int fd);
void Logout(std::list<std::string> &arg_str, int fd);
void Whoami(std::list<std::string> &arg_str, int fd);
void ListUser(std::list<std::string> &arg_str, int fd);
void Exit(std::list<std::string> &arg_str, int fd);
void Send(std::list<std::string> &arg_str, int fd);
void ListMsg(std::list<std::string> &arg_str, int fd);
void Receive(std::list<std::string> &arg_str, int fd);

typedef void (*FuncPtr)(std::list<std::string> &arg_str, int fd);

std::unordered_map<std::string, FuncPtr> kCommandFuntions = {
    {"register", Register},
    {"login", Login},
    {"logout", Logout},
    {"whoami", Whoami},
    {"list-user", ListUser},
    {"exit", Exit},
    {"send", Send},
    {"list-msg", ListMsg},
    {"receive", Receive}
};