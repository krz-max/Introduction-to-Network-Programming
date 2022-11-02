#include "stdlib.h"
#include "command.cpp"
inline std::string &rtrim(std::string &s);
void Parser(std::stringstream &ss, std::list<std::string> &args);
bool ParseCommand(std::stringstream &ss, struct pollfd *client, std::list<std::string> &cmd);
