#ifndef STRING_HEADER
#define STRING_HEADER

#include <cstring>
#include <string>
#include <sstream>
#endif

#ifndef QUEUE_HEADER
#define QUEUE_HEADER

#include <queue>
#endif

#ifndef SOCKET_HEADER
#define SOCKET_HEADER

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#endif

#ifndef MAP_HEADER
#define MAP_HEADER

#include <map>
#include <unordered_map>
#endif

#ifndef FORWARD_LIST_HEADER
#define FORWARD_LIST_HEADER

#include <forward_list>
#endif

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

void Register(std::vector<std::string> &arg_str, int fd);
void Login(std::vector<std::string> &arg_str, int fd);
void Logout(std::vector<std::string> &arg_str, int fd);
void Whoami(std::vector<std::string> &arg_str, int fd);
void ListUser(std::vector<std::string> &arg_str, int fd);
void Exit(std::vector<std::string> &arg_str, int fd);
void Send(std::vector<std::string> &arg_str, int fd);
void ListMsg(std::vector<std::string> &arg_str, int fd);
void Receive(std::vector<std::string> &arg_str, int fd);

typedef void (*FuncPtr)(std::vector<std::string> &arg_str, int fd);

std::unordered_map<std::string, FuncPtr> kCommandFuntions = {
    {"register", Register},
    {"login", Login},
    {"logout", Logout},
    {"whoami", Whoami},
    {"list-user", ListUser},
    {"exit", Exit},
    {"send", Send},
    {"list-msg", ListMsg},
    {"receive", Receive}};