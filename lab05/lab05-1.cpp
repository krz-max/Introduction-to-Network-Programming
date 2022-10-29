#include "header.h"
#define MAXUSER    1024
#define MAXLINE    2000
#define NAMELEN    20
#define WELCOME    10000
#define ARRIVE     10001
#define LEAVE      10002
#define NAMECHANGE 10003
#define SELFCHANGE 10004
#define NAMELIST   10005
#define WRONGCMD   10006
#define CHAT       10007
#define SARRIVE    10000
#define SLEAVE     10001

using namespace std;

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
		case WELCOME:
			dprintf(sockfd, "%s *** Welcome to the simple CHAT server\n", timeStr);
			dprintf(sockfd, "%s *** Total %d users online now. Your name is %s\n", timeStr, howManyUsers, nameStr[userid]);
			break;
		case ARRIVE:
			for(int k = 1; k <= maxi; k++) {
				if(client[k].fd < 0) 
					continue;
				if(k != userid) {
					dprintf(client[k].fd, " *** User %s has just landed on the server\n", nameStr[userid]);
				}
			}
			break;
		case LEAVE:
			for(int k = 1; k <= maxi; k++) {
				if(client[k].fd < 0) 
					continue;
				if(k != userid) {
					dprintf(client[k].fd, " *** User %s has left the server\n", nameStr[userid]);
				}
			}
			break;
		case NAMECHANGE:
			for(int k = 1; k <= maxi; k++) {
				if(client[k].fd == -1) 
					continue;
				if(k != userid) {
					dprintf(client[k].fd, " *** User %s renamed to %s\n", nameStr[userid], newname);
				}
			}
			break;
		case SELFCHANGE:			
			dprintf(sockfd, " *** Nickname changed to %s\n", newname);
			break;
		case NAMELIST:
			dprintf(sockfd, "------------------------\n");
			for(int k = 1; k <= maxi; k++) {
				if(client[k].fd == -1) 
					continue;
				if(k == userid) dprintf(client[k].fd, " * ");
				else dprintf(sockfd, "   ");
				Inet_ntop(AF_INET, (sockaddr*)&CAddr[k].sin_addr, ip, INET_ADDRSTRLEN);
				dprintf(sockfd, "%s     %s:%d\n", nameStr[k], ip, CAddr[k].sin_port);
			}
			break;
		case WRONGCMD:
			dprintf(sockfd, " *** Unknown or incomplete command : %s\n", buf);
			break;
		case CHAT:
			for(int k = 1; k < maxi+1; k++) {
				if(client[k].fd == -1) 
					continue;
				if(k == userid) 
					continue;
				dprintf(client[k].fd, "%s: %s\n", nameStr[userid], buf);
			}
		default:
			break;
	}
}
void command_prompt(int sockfd, int userid) {
	if ( !strncmp(buf, "/name", 5) ) {
		strncpy(newname, &buf[6], strlen(buf)-6);
		newname[strlen(newname)-1] = NULL;
		Notify(sockfd, userid, SELFCHANGE);
		Notify(sockfd, userid, NAMECHANGE);
		strcpy(nameStr[userid], newname);
	}
	else if ( !strncmp(buf, "/who", 4) ) {
		Notify(sockfd, userid, NAMELIST);
	}
	else {				
		buf[n-1] = NULL;
		Notify(sockfd, userid, WRONGCMD);
	}
	return ;
}
int main(int argc, char** argv){
    Start_TCP_Server(&listenfd, strtol(argv[1], NULL, 10));
	signal(SIGPIPE, sig_hand);
	howManyUsers = 0;

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
			
			Inet_ntop(AF_INET, (sockaddr*)&cliaddr.sin_addr, ip, INET_ADDRSTRLEN);
			printf("* client connected from %s:%d\n", ip, cliaddr.sin_port);
			howManyUsers++;

			client[i].events = POLLRDNORM;
			CLen[i] = clilen;
			CAddr[i] = cliaddr;
			if (i > maxi)
				maxi = i;				/* max index in client[] .fdarray */
			
			strcpy(nameStr[i], "kkmelon");
			Notify(connfd, i, WELCOME);
			Notify(connfd, i, ARRIVE);

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
					Close(nowfd);
					client[i].fd = -1;
					Inet_ntop(AF_INET, (sockaddr*)&CAddr[i].sin_addr, ip, INET_ADDRSTRLEN);
					printf("* client %s:%d disconnected\n", ip, CAddr[i].sin_port);
					Notify(connfd, i, LEAVE);
					howManyUsers--;
				}
				else if ( buf[0] == '/' ) {
					command_prompt(client[i].fd, i);
				}
				else {
					buf[n-1] = NULL;
					Notify(connfd, i, CHAT);
				}


				if (--nready <= 0)
					break;				/* no more readable descriptors */
			}
		}
	}
	return 0;
}