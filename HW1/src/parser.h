#include "stdlib.h"
#include "session.cpp"
inline std::string &rtrim(std::string &s);
void Parser(std::stringstream &ss, std::list<std::string> &args);
bool ParseCommand(std::stringstream &ss, UserInfo *client, std::list<std::string> &cmd);
