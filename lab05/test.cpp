#include "../Header/MySocket.h"
#include <sys/types.h>
#include <sys/wait.h>
#define MAXUSER    2000
#define MAXLINE    2000
#define NAMELEN    20
#define WELCOME    0
#define ARRIVE     1
#define LEAVE      2
#define NAMECHANGE 3
#define SELFCHANGE 4
#define NAMELIST   5
#define WRONGCMD   6
#define CHAT       7
#define SARRIVE    0
#define SLEAVE     1

using namespace std;

char                nameStr[MAXUSER][NAMELEN], newname[NAMELEN];
int					i, maxi, maxfd, listenfd, connfd;
int                 howManyUsers;
int					nready, client[FD_SETSIZE];
int                 writefd;
ssize_t				n;
fd_set				rset, allset;
char				buf[MAXLINE], ip[MAXLINE];
socklen_t			clilen;
struct sockaddr_in	cliaddr;
socklen_t           CLen[MAXUSER];
struct sockaddr_in  CAddr[MAXUSER];

int getTimeString(char* timeStr){
    time_t times = time(NULL);
    struct tm* utcTime = gmtime(&times);
    int timeStrLen = sprintf(timeStr, "%04d-%02d-%02d %02d:%02d:%02d ", utcTime->tm_year+1900, utcTime->tm_mon+1, utcTime->tm_mday, utcTime->tm_hour+8, utcTime->tm_min, utcTime->tm_sec);
    return timeStrLen;
}
void Notify(int sockfd, int userid, int MessageId){
	char timeStr[40];
    int timeStrLen = getTimeString(timeStr);
	switch(MessageId){
		case WELCOME:
			dup2(sockfd, fileno(stdout));
			cout << timeStr << " *** Welcome to the simple CHAT server" << endl;
			cout << timeStr << " *** Total " << howManyUsers << " users online now. Your name is <" << nameStr[userid] << ">" << endl;
			break;
		case ARRIVE:
			for(int k = 0; k < maxi+1; k++) {
				if(client[k] == -1) continue;
				if(k != userid) {
					dup2(client[k], fileno(stdout));
					cout << timeStr << " *** User " << nameStr[userid] << " has just landed on the server" << endl;
				}
			}
			break;
		case LEAVE:
			for(int k = 0; k < maxi+1; k++) {
				if(client[k] == -1) continue;
				if(k != userid){
					dup2(client[k], fileno(stdout));
					cout << timeStr << " *** User " << nameStr[userid] << " has just left the server" << endl;
				}
			}
			break;
		case NAMECHANGE:
			for(int k = 0; k < maxi+1; k++) {
				if(client[k] == -1) continue;
				if(k != userid) {
					dup2(client[k], STDOUT_FILENO);
					cout << " *** User " << nameStr[userid] << " renamed to " << newname << endl;
				}
			}
			break;
		case SELFCHANGE:			
			dup2(sockfd, fileno(stdout));
			cout << timeStr << " *** Nickname changed to " << newname << endl;
			break;
		case NAMELIST:
			dup2(sockfd, fileno(stdout));
			cout << "---" << endl;
			for(int k = 0; k < maxi+1; k++) {
				if(client[k] == -1) continue;
				if(k == userid) cout << " * ";
				else cout << "   ";
				cout << setw(NAMELEN) << left << nameStr[k] << Inet_ntop(AF_INET, (sockaddr*)&CAddr[k].sin_addr, ip, INET_ADDRSTRLEN) << ":" << CAddr[k].sin_port << endl;
			}
			break;
		case WRONGCMD:
			dup2(sockfd, fileno(stdout));
			cout << timeStr << " *** Unknown or incomplete command : " << buf << endl;
			break;
		case CHAT:
			for(int k = 0; k < maxi+1; k++) {
				if(client[k] == -1) continue;
				if(k == userid) continue;
				dup2(client[k], STDOUT_FILENO);
				cout << timeStr << nameStr[userid] << " " << buf << endl;
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
		Notify(sockfd, userid, WRONGCMD);
	}
	return ;
}
void ServerMessage(const sockaddr_in caddr, int MessageId) {
	dup2(writefd, STDOUT_FILENO);
	if(MessageId == SARRIVE)
		cout << "* client connected from " << Inet_ntop(AF_INET, (sockaddr*)&caddr.sin_addr, ip, INET_ADDRSTRLEN) << ":" << caddr.sin_port << endl;
	else {
		cout << "* client " << Inet_ntop(AF_INET, (sockaddr*)&caddr.sin_addr, ip, INET_ADDRSTRLEN) << ":" << caddr.sin_port << " disconnected" << endl;
	}
}
int main(int argc, char** argv){
	int value = 1;
	struct linger L = {1, 1};
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, &value, sizeof(value));
	setsockopt(listenfd, SOL_SOCKET, SO_LINGER, &L, sizeof(L));
    Start_TCP_Server(&listenfd, strtol(argv[1], NULL, 10));
	// signal(SIGPIPE, SIG_IGN);
	writefd = dup(STDOUT_FILENO);
	howManyUsers = 0;
	maxfd = listenfd;			/* initialize */
	maxi = -1;					/* index into client[] array */
	for (i = 0; i < FD_SETSIZE; i++)
		client[i] = -1;			/* -1 indicates available entry */
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);

	for ( ; ; ) {
		rset = allset;		/* structure assignment */
		nready = Select(maxfd+1, &rset, NULL, NULL, NULL);

		if (FD_ISSET(listenfd, &rset)) {	/* new client connection */
			clilen = sizeof(cliaddr);
			connfd = Accept(listenfd, (sockaddr *) &cliaddr, &clilen);
			ServerMessage(cliaddr, SARRIVE);
			howManyUsers++;
			for (i = 0; i < FD_SETSIZE; i++)
				if (client[i] < 0) {
					client[i] = connfd;	/* save descriptor */
					break;
				}
			if (i == FD_SETSIZE) {
				cerr << "too many clients\n";
                return -1;
            }
			CLen[i] = clilen;
			CAddr[i] = cliaddr;
			FD_SET(connfd, &allset);	/* add new descriptor to set */
			if (connfd > maxfd)
				maxfd = connfd;			/* for select */
			if (i > maxi)
				maxi = i;				/* max index in client[] array */
			strcpy(nameStr[maxi], "kkmelon");
			Notify(connfd, i, WELCOME);
			Notify(connfd, i, ARRIVE);
			if (--nready <= 0)
				continue;				/* no more readable descriptors */
		}
		for (i = 0; i <= maxi; i++) {	/* check all clients for data */
			if ( client[i] < 0 )
				continue;
			if (FD_ISSET(client[i], &rset)) {
				if ( (n = read(client[i], buf, MAXLINE)) < 0 ) {
					if (errno == ECONNRESET) {
						close(client[i]);
						client[i] = -1;
					} else
						err_sys("read error");
				}
				else if ( n == 0) {
					/*4connection closed by client */
					// ServerMessage(CAddr[i], SLEAVE);
					ServerMessage(CAddr[i], SLEAVE);
					Notify(connfd, i, LEAVE);
					close(client[i]);
					FD_CLR(client[i], &allset);
					client[i] = -1;
					howManyUsers--;
				} 
				else if ( buf[0] == '/' ) {
					buf[n] = NULL;
					command_prompt(client[i], i);
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