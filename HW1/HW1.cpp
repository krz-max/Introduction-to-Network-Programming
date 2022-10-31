#include "src/socketwrapper.h"
#include "src/server.h"
#include "src/command.h"
#include "src/unixwrapper.h"

#define MAXUSER     1024
#define MAXLINE     2000
#define NAMELEN     20
#define LIST        10003
#define NAMES       10006
#define USERS       10008
#define NICK        10000
#define USER        10001
#define PING        10002
#define JOIN        10004
#define TOPIC       10005
#define PART        10007
#define PRIVMSG     10009
#define LEAVE       10010
#define UNKNOWN     10011

char                nameStr[1024][NAMELEN], newname[NAMELEN];
int					i, maxi, maxfd, listenfd, connfd, nowfd;
int                 howManyUsers;
int					nready;
struct pollfd client[1024];
int                 writefd;
ssize_t				n;
fd_set				rset, allset;
char				buf[MAXLINE], ip[MAXLINE];
socklen_t			clilen;
struct sockaddr_in	cliaddr;
socklen_t           CLen[1024];
struct sockaddr_in  CAddr[1024];
void sig_hand(int signo) {}
void Notify(int sockfd, int userid, int MessageId){
	time_t raw_time = time(0);
	struct tm *TIME = localtime(&raw_time);
	char timeStr[40];
	sprintf(timeStr, "%04d-%02d-%02d %d:%d:%d ", TIME->tm_year+1900, TIME->tm_mon+1, TIME->tm_mday, TIME->tm_hour, TIME->tm_min, TIME->tm_sec);
	switch(MessageId){
		case NICK:
			break;
		case USER:
			break;
		case PING:
			break;
		case LIST:
			break;
		case JOIN:			
			break;
		case TOPIC:
			break;
		case NAMES:
            break;
		case PART:
			break;
        case USERS:
            break;
        case PRIVMSG:
            break;
        case LEAVE:
            break;
        default:
            break;
	}
}

int main(int argc, char** argv){
	Server Myserver(htons(strtol(argv[1], NULL, 10)), htonl(INADDR_ANY));

	client[0].fd = listenfd;
	client[0].events = POLLRDNORM;
	for (i = 1; i < 1024; i++)
		client[i].fd = -1;			/* -1 indicates available entry */
	maxi = 0;
	for ( ; ; ) {
		nready = Poll(client, maxi+1, INFTIM);

		if(client[0].revents & POLLRDNORM) {
			clilen = sizeof(cliaddr);
			connfd = Accept(listenfd, (sockaddr*)&cliaddr, &clilen);
			for (i = 1; i < 1024; i++)
				if (client[i].fd < 0) {
					client[i].fd = connfd;	/* save descriptor */
					break;
				}
			if (i == 1024)
				err_sys("too many clients");


			client[i].events = POLLRDNORM;
			CLen[i] = clilen;
			CAddr[i] = cliaddr;
			if (i > maxi)
				maxi = i;				/* max index in client[] .fdarray */
			
			if (--nready <= 0)
				continue;				/* no more readable descriptors */
		}
		
		for (i = 1; i <= maxi; i++) {	/* check all clients for data */
			if ( (nowfd = client[i].fd) < 0)
				continue;
			if (client[i].revents & (POLLRDNORM | POLLERR)) {
				if ( (n = read(nowfd, buf, MAXLINE)) < 0) {
					if (errno == ECONNRESET) {
							/*4connection reset by client */
						Close(nowfd);
						client[i].fd = -1;
					} else
						err_sys("read error");
				} else if (n == 0) {
					/*4connection closed by client */
				}
                else {
                    /* "/server" ..; "/xxx"-like command */
                    parseString(buf);
                }


				if (--nready <= 0)
					break;				/* no more readable descriptors */
			}
		}
	}
	return 0;
}