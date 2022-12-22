#include "header.h"

#define SA sockaddr

int Socket(int family, int type, int protocol);
void Bind(int sockfd, struct SA *myaddr, socklen_t addrlen);
const char *Inet_ntop(int family, const void *addrptr, char *str, size_t len);