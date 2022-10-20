#include "../Header/MySocket.h"
#include <sys/types.h>
#include <sys/wait.h>
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

int printTimeString(){
	return 0;
}
void Notify(int sockfd, int userid, int MessageId){
    		time_t times = time(NULL);
    		struct tm* utcTime = gmtime(&times);
	switch(MessageId){
		case WELCOME:
			// dup2(sockfd, fileno(stdout));
			// cout << timeStr << " *** Welcome to the simple CHAT server" << endl;
			// cout << timeStr << " *** Total " << howManyUsers << " users online now. Your name is <" << nameStr[userid] << ">" << endl;
			// printTimeString();
			// Writen(sockfd, timeStr, timeStrLen);
			return ;
			Writen(sockfd, " *** Welcome to the simple CHAT server\n", 39);
			// Writen(sockfd, timeStr, timeStrLen);
			Writen(sockfd, " *** Total ", 11);
			Writen(sockfd, to_string(howManyUsers).c_str(), strlen(to_string(howManyUsers).c_str()));
			Writen(sockfd, " users online now. Your name is <", 33);
			Writen(sockfd, nameStr[userid], strlen(nameStr[userid]));
			Writen(sockfd, ">\n", 2);
			break;
		case ARRIVE:
			for(int k = 0; k <= maxi; k++) {
				if(client[k] < 0) 
					continue;
				if(k != userid) {
					// Writen(client[k], timeStr, timeStrLen);
					Writen(client[k], " *** User ", 10);
					Writen(client[k], nameStr[userid], strlen(nameStr[userid]));
					Writen(client[k], " has just landed on the server\n", 31);
					// cout << timeStr << " *** User " << nameStr[userid] << " has just landed on the server" << endl;
				}
			}
			break;
		case LEAVE:
			for(int k = 0; k <= maxi; k++) {
				if(client[k] < 0) 
					continue;
				if(k != userid) {
					// Writen(client[k], timeStr, timeStrLen);
					Writen(client[k], " *** User ", 10);
					Writen(client[k], nameStr[userid], strlen(nameStr[userid]));
					Writen(client[k], " has just left the server\n", 26);
					// cout << timeStr << " *** User " << nameStr[userid] << " has just landed on the server" << endl;
				}
				// if(client[k] == -1) continue;
				// if(k != userid){
				// 	dup2(client[k], fileno(stdout));
				// 	cout << timeStr << " *** User " << nameStr[userid] << " has just left the server" << endl;
				// }
			}
			break;
		case NAMECHANGE:
			for(int k = 0; k < maxi+1; k++) {
				if(client[k] == -1) 
					continue;
				if(k != userid) {
					// dup2(client[k], STDOUT_FILENO);
					// cout << " *** User " << nameStr[userid] << " renamed to " << newname << endl;
					Writen(client[k], " *** User ", 10);
					Writen(client[k], nameStr[userid], strlen(nameStr[userid]));
					Writen(client[k], " renamed to ", 12);
					Writen(client[k], newname, strlen(newname));
				}
			}
			break;
		case SELFCHANGE:			
			// dup2(sockfd, fileno(stdout));
			// Writen(sockfd, timeStr, timeStrLen);
			Writen(sockfd, " *** Nickname changed to ", 25);
			Writen(sockfd, newname, strlen(newname));
			// cout << timeStr << " *** Nickname changed to " << newname << endl;
			break;
		case NAMELIST:
			// dup2(sockfd, fileno(stdout));
			// cout << "---" << endl;
			Writen(sockfd, "---\n", 4);
			for(int k = 0; k < maxi+1; k++) {
				if(client[k] == -1) 
					continue;
				if(k == userid) Writen(client[k], " * ", 3);
				else Writen(sockfd, "   ", 3);
				Writen(sockfd, nameStr[k], strlen(nameStr[k]));
				Inet_ntop(AF_INET, (sockaddr*)&CAddr[k].sin_addr, ip, INET_ADDRSTRLEN);
				Writen(sockfd, ip, strlen(ip));
				Writen(sockfd, ":", 1);
				Writen(sockfd, to_string(CAddr[k].sin_port).c_str(), strlen(to_string(CAddr[k].sin_port).c_str()));
				Writen(sockfd, "\n", 1);
				// cout << setw(NAMELEN) << left << nameStr[k] << Inet_ntop(AF_INET, (sockaddr*)&CAddr[k].sin_addr, ip, INET_ADDRSTRLEN) << ":" << CAddr[k].sin_port << endl;
			}
			break;
		case WRONGCMD:
			// dup2(sockfd, fileno(stdout));
			// cout << timeStr << " *** Unknown or incomplete command : " << buf << endl;
			// Writen(sockfd, timeStr, timeStrLen);
			Writen(sockfd, " *** Unknown or incomplete command : ", 37);
			Writen(sockfd, buf, strlen(buf));
			Writen(sockfd, "\n", 1);
			break;
		case CHAT:
			for(int k = 0; k < maxi+1; k++) {
				if(client[k] == -1) 
					continue;
				if(k == userid) 
					continue;
				// dup2(client[k], STDOUT_FILENO);
				// Writen(client[k], timeStr, timeStrLen);
				Writen(client[k], nameStr[userid], strlen(nameStr[userid]));
				Writen(client[k], " ", 1);
				Writen(sockfd, buf, strlen(buf));
				Writen(sockfd, "\n", 1);
				// cout << timeStr << nameStr[userid] << " " << buf << endl;
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
// void ServerMessage(const sockaddr_in caddr, int MessageId) {
// 	if(MessageId == SARRIVE)
// 	else {
// 	}
// }
int main(int argc, char** argv){
	// int value = 1;
	// struct linger L = {1, 1};
	// setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));
	// setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, &value, sizeof(value));
	// setsockopt(listenfd, SOL_SOCKET, SO_LINGER, &L, sizeof(L));
    Start_TCP_Server(&listenfd, strtol(argv[1], NULL, 10));
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
		// cout << nready << " ";
		if (FD_ISSET(listenfd, &rset)) {	/* new client connection */
			clilen = sizeof(cliaddr);
			connfd = Accept(listenfd, (sockaddr *) &cliaddr, &clilen);
			// ServerMessage(cliaddr, SARRIVE);
			cout << i << " " << connfd;
			cout << "* client connected from " << Inet_ntop(AF_INET, (sockaddr*)&cliaddr.sin_addr, ip, INET_ADDRSTRLEN) << ":" << cliaddr.sin_port << endl;
			howManyUsers++;
			for (i = 0; i < FD_SETSIZE && connfd < FD_SETSIZE; i++) {
				if (client[i] < 0) {
					client[i] = connfd;	/* save descriptor */
					break;
				}
			}
			if (i == FD_SETSIZE) {
				cerr << "too many clients\n";
                return -1;
            }
			if (connfd <= 2 || connfd >= FD_SETSIZE) {
				cerr << "connfd error\n";
				return -1;
			}
			CLen[i] = clilen;
			CAddr[i] = cliaddr;
			FD_SET(connfd, &allset);	/* add new descriptor to set */
			if (connfd > maxfd)
				maxfd = connfd;			/* for select */
			if (i > maxi)
				maxi = i;				/* max index in client[] array */
			strcpy(nameStr[i], "kkmelon");
			Notify(connfd, i, WELCOME);
			return 0;
			Notify(connfd, i, ARRIVE);
			if (--nready <= 0)
				continue;				/* no more readable descriptors */
		}
		for (i = 0; i <= maxi; i++) {	/* check all clients for data */
			if ( client[i] < 0 )
				continue;
			if (FD_ISSET(client[i], &rset)) {
				if ( ( n = read(client[i], buf, MAXLINE)) < 0) {
					if (errno == ECONNRESET) {
						FD_CLR(client[i], &allset);
						Close(client[i]);
						client[i] = -1;
					} else
						err_sys("read error");
				}
				else if (n == 0) {
					if(client[i] < 0) continue;
					// ServerMessage(CAddr[i], SLEAVE);
					cout << "* client " << Inet_ntop(AF_INET, (sockaddr*)&CAddr[i].sin_addr, ip, INET_ADDRSTRLEN) << ":" << CAddr[i].sin_port << " disconnected" << endl;
					Notify(connfd, i, LEAVE);
					FD_CLR(client[i], &allset);
					Close(client[i]);
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
		for(i = 0; i <= maxi; i++) cout<< client[i] << " ";
		cout << endl;
	}
	return 0;
}