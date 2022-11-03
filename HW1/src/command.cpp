#include "command.h"

void nick(std::list<std::string> &arg_str, UserInfo *client)
{
    std::string CMD, nickname;
    CMD = arg_str.front();
    arg_str.pop_front();
    nickname = arg_str.front();
    arg_str.pop_front();
    client->UserID.nickname = nickname;
    return;
}
void user(std::list<std::string> &arg_str, UserInfo *client)
{
    std::string CMD, username, servername, hostname, realname;
    CMD = arg_str.front();
    arg_str.pop_front();
    servername = arg_str.front();
    arg_str.pop_front();
    username = arg_str.front();
    arg_str.pop_front();
    hostname = arg_str.front();
    arg_str.pop_front();
    realname = arg_str.front();
    arg_str.pop_front();
    client->UserID.servername = servername;
    client->UserID.hostname = hostname;
    client->UserID.realname = realname;
    client->UserID.username = username;
    client->isLogin = true;
    dprintf(client->fd, ":mircd 001 %s :Welcome to the minimized IRC daemon!\r\n", client->UserID.nickname.c_str());
    dprintf(client->fd, ":mircd 251 %s :There are 1 users and 0 invisible on 1 server\r\n", client->UserID.nickname.c_str());
    dprintf(client->fd, ":mircd 375 %s :- mircd Message of the day -\r\n", client->UserID.nickname.c_str());
    dprintf(client->fd, ":mircd 372 %s :-  Hello, World!\r\n", client->UserID.nickname.c_str());
    dprintf(client->fd, ":mircd 372 %s :-               @                    _ \r\n", client->UserID.nickname.c_str());
    dprintf(client->fd, ":mircd 372 %s :-   ____  ___   _   _ _   ____.     | |\r\n", client->UserID.nickname.c_str());
    dprintf(client->fd, ":mircd 372 %s :-  /  _ `'_  \\ | | | '_/ /  __|  ___| |\r\n", client->UserID.nickname.c_str());
    dprintf(client->fd, ":mircd 372 %s :-  | | | | | | | | | |   | |    /  _  |\r\n", client->UserID.nickname.c_str());
    dprintf(client->fd, ":mircd 372 %s :-  | | | | | | | | | |   | |__  | |_| |\r\n", client->UserID.nickname.c_str());
    dprintf(client->fd, ":mircd 372 %s :-  |_| |_| |_| |_| |_|   \\____| \\___,_|\r\n", client->UserID.nickname.c_str());
    dprintf(client->fd, ":mircd 372 %s :-  minimized internet relay chat daemon\r\n", client->UserID.nickname.c_str());
    dprintf(client->fd, ":mircd 372 %s :-\r\n", client->UserID.nickname.c_str());
    dprintf(client->fd, ":mircd 376 %s :End of message of the day\r\n", client->UserID.nickname.c_str());
    return;
}
void ping(std::list<std::string> &arg_str, UserInfo *client)
{
    return;
}
void list(std::list<std::string> &arg_str, UserInfo *client, Channel *chan, int nchannel)
{
    dprintf(client->fd, ":mircd 321 %s Channel  :Users  Name\r\n", client->UserID.nickname.c_str());
    dprintf(client->fd, ":mircd 322 %s #%s %d :%s\r\n", "chan1", 10, "Hello, World");
    dprintf(client->fd, ":mircd 323 %s :End of /LIST\r\n", client->UserID.nickname.c_str());
    return;
}
void join(std::list<std::string> &arg_str, UserInfo *client)
{
    dprintf(client->fd, ":%s JOIN #chal1\r\n", client->UserID.nickname.c_str());
    dprintf(client->fd, ":mircd 331 %s #chal1 :No topic is set\r\n", client->UserID.nickname.c_str());
    dprintf(client->fd, ":mircd 353 %s #chal1 :%s\r\n", client->UserID.nickname.c_str(), client->UserID.nickname.c_str());
    dprintf(client->fd, ":mircd 366 %s #chal1 :End of Names List\r\n", client->UserID.nickname.c_str());
    return;
}
void topic(std::list<std::string> &arg_str, UserInfo *client)
{
    return;
}
void names(std::list<std::string> &arg_str, UserInfo *client)
{
    return;
}
void part(std::list<std::string> &arg_str, UserInfo *client)
{
    return;
}
void users(std::list<std::string> &arg_str, UserInfo *client)
{
    dprintf(client->fd, ":mircd 392 %s :UserID   Terminal   Host\r\n", client->UserID.nickname.c_str());
    dprintf(client->fd, ":mircd 393 %s :%-8s %-9s %-8s\r\n", "%s", "-", "localhost", client->UserID.nickname.c_str());
    dprintf(client->fd, ":mircd 393 %s :%-8s %-9s %-8s\r\n", "%s", "-", "localhost", client->UserID.nickname.c_str());
    dprintf(client->fd, ":mircd 393 %s :%-8s %-9s %-8s\r\n", "%s", "-", "localhost", client->UserID.nickname.c_str());
    dprintf(client->fd, ":mircd 394 %s :End of users\r\n", client->UserID.nickname.c_str());
    return;
}
void privmsg(std::list<std::string> &arg_str, UserInfo *client)
{
    return;
}
void quit(std::list<std::string> &arg_str, UserInfo *client)
{
    return;
}
