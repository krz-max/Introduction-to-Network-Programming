#include "parser.h"

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
        if(str[0] == '#')
        {
            // JOIN, TOPIC, PART, PRIVMSG, NAMES has <channel> as argument
            // #channel
            std::string temp = str.substr(1);
            args.push_back(rtrim(str));
        }
        else if(str[0] == ':') // last argument
        {
            std::string temp = str.substr(1);
            args.push_back(rtrim(str));
            return ;
        }
        else{
            args.push_back(rtrim(str));
        }
    }
}
// parse command and do action
// return :
//      true: close connection
//      false: keep reading
bool ParseCommand(std::stringstream &ss, UserInfo *client, std::list<std::string> &cmd)
{
    Parser(ss, cmd);
    fprintf(stdout, "cmd size: %ld\n", cmd.size());
    for(std::string it:cmd)
        fprintf(stdout, "parsed cmd: %s\n", it.c_str());

    if (cmd.empty())
        return false;
    auto func = kCommandFuntions.find(cmd.front());
    // if(cmd.front() == "USERS") fprintf(stdout, "???\n");
    if (func != kCommandFuntions.end())
    {
        fprintf(stdout, "cmd: %s\n", func->first.c_str());
        (*func->second)(cmd, client);

        if (func->first == "exit")
            return true;
    }
    return false;
}