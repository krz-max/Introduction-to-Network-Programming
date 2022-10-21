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
void sig_hand(int signo) {}
int printTimeString(){
	return 0;
}
void Notify(int sockfd, int userid, int MessageId){
	time_t raw_time = time(0);
	struct tm *TIME = localtime(&raw_time);
	char timeStr[40];
	int timeStrLen = sprintf(timeStr, "%04d-$02d-%02d %d:%d:%d ", TIME->tm_year+1900, TIME->tm_mon+1, TIME->tm_mday, TIME->tm_hour, TIME->tm_min, TIME->tm_sec);
	switch(MessageId){
		case WELCOME:
			// dup2(sockfd, fileno(fileno(stdout)));
			// cout << timeStr << " *** Welcome to the simple CHAT server" << endl;
			// cout << timeStr << " *** Total " << howManyUsers << " users online now. Your name is <" << nameStr[userid] << ">" << endl;
			// printTimeString();
			Send(sockfd, timeStr, timeStrLen, MSG_NOSIGNAL);
			// return ;
			Send(sockfd, " *** Welcome to the simple CHAT server\n", 39, MSG_NOSIGNAL);
			Send(sockfd, timeStr, timeStrLen, MSG_NOSIGNAL);
			Send(sockfd, " *** Total ", 1, MSG_NOSIGNAL);
			Send(sockfd, to_string(howManyUsers).c_str(), strlen(to_string(howManyUsers).c_str()), MSG_NOSIGNAL);
			Send(sockfd, " users online now. Your name is <", 33, MSG_NOSIGNAL);
			Send(sockfd, nameStr[userid], strlen(nameStr[userid]), MSG_NOSIGNAL);
			Send(sockfd, ">\n", 2, MSG_NOSIGNAL);
			break;
		case ARRIVE:
			for(int k = 0; k <= maxi; k++) {
				if(client[k] < 0) 
					continue;
				if(k != userid) {
					// Send(client[k], timeStr, timeStrLen, MSG_NOSIGNAL);
					Send(client[k], " *** User ", 10, MSG_NOSIGNAL);
					Send(client[k], nameStr[userid], strlen(nameStr[userid]), MSG_NOSIGNAL);
					Send(client[k], " has just landed on the server\n", 31, MSG_NOSIGNAL);
					// cout << timeStr << " *** User " << nameStr[userid] << " has just landed on the server" << endl;
				}
			}
			break;
		case LEAVE:
			for(int k = 0; k <= maxi; k++) {
				if(client[k] < 0) 
					continue;
				if(k != userid) {
					// Send(client[k], timeStr, timeStrLen, MSG_NOSIGNAL);
					Send(client[k], " *** User ", 10, MSG_NOSIGNAL);
					Send(client[k], nameStr[userid], strlen(nameStr[userid]), MSG_NOSIGNAL);
					Send(client[k], " has just left the server\n", 26, MSG_NOSIGNAL);
					// cout << timeStr << " *** User " << nameStr[userid] << " has just landed on the server" << endl;
				}
				// if(client[k] == -1) continue;
				// if(k != userid){
				// 	dup2(client[k], fileno(fileno(stdout)));
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
					Send(client[k], " *** User ", 10, MSG_NOSIGNAL);
					Send(client[k], nameStr[userid], strlen(nameStr[userid]), MSG_NOSIGNAL);
					Send(client[k], " renamed to ", 12, MSG_NOSIGNAL);
					Send(client[k], newname, strlen(newname), MSG_NOSIGNAL);
				}
			}
			break;
		case SELFCHANGE:			
			// dup2(sockfd, fileno(fileno(stdout)));
			// Send(sockfd, timeStr, timeStrLen, MSG_NOSIGNAL);
			Send(sockfd, " *** Nickname changed to ", 25, MSG_NOSIGNAL);
			Send(sockfd, newname, strlen(newname), MSG_NOSIGNAL);
			// cout << timeStr << " *** Nickname changed to " << newname << endl;
			break;
		case NAMELIST:
			// dup2(sockfd, fileno(fileno(stdout)));
			// cout << "---" << endl;
			Send(sockfd, "---\n", 4, MSG_NOSIGNAL);
			for(int k = 0; k < maxi+1; k++) {
				if(client[k] == -1) 
					continue;
				if(k == userid) Send(client[k], " * ", 3, MSG_NOSIGNAL);
				else Send(sockfd, "   ", 3, MSG_NOSIGNAL);
				Send(sockfd, nameStr[k], strlen(nameStr[k]), MSG_NOSIGNAL);
				Inet_ntop(AF_INET, (sockaddr*)&CAddr[k].sin_addr, ip, INET_ADDRSTRLEN);
				Send(sockfd, ip, strlen(ip), MSG_NOSIGNAL);
				Send(sockfd, ":", 1, MSG_NOSIGNAL);
				Send(sockfd, to_string(CAddr[k].sin_port).c_str(), strlen(to_string(CAddr[k].sin_port).c_str()), MSG_NOSIGNAL);
				Send(sockfd, "\n", 1, MSG_NOSIGNAL);
				// cout << setw(NAMELEN) << left << nameStr[k] << Inet_ntop(AF_INET, (sockaddr*)&CAddr[k].sin_addr, ip, INET_ADDRSTRLEN) << ":" << CAddr[k].sin_port << endl;
			}
			break;
		case WRONGCMD:
			// dup2(sockfd, fileno(fileno(stdout)));
			// cout << timeStr << " *** Unknown or incomplete command : " << buf << endl;
			// Send(sockfd, timeStr, timeStrLen, MSG_NOSIGNAL);
			Send(sockfd, " *** Unknown or incomplete command : ", 37, MSG_NOSIGNAL);
			Send(sockfd, buf, strlen(buf), MSG_NOSIGNAL);
			Send(sockfd, "\n", 1, MSG_NOSIGNAL);
			break;
		case CHAT:
			for(int k = 0; k < maxi+1; k++) {
				if(client[k] == -1) 
					continue;
				if(k == userid) 
					continue;
				// dup2(client[k], STDOUT_FILENO);
				// Send(client[k], timeStr, timeStrLen, MSG_NOSIGNAL);
				Send(client[k], nameStr[userid], strlen(nameStr[userid]), MSG_NOSIGNAL);
				Send(client[k], " ", 1, MSG_NOSIGNAL);
				Send(sockfd, buf, strlen(buf), MSG_NOSIGNAL);
				Send(sockfd, "\n", 1, MSG_NOSIGNAL);
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
			// cout << i << " " << connfd;
			Send(fileno(stdout), "* client connected from ", 24, MSG_NOSIGNAL);
			Inet_ntop(AF_INET, (sockaddr*)&cliaddr.sin_addr, ip, INET_ADDRSTRLEN);
			Send(fileno(stdout), ip, strlen(ip), MSG_NOSIGNAL);
			Send(fileno(stdout), ":", 1, MSG_NOSIGNAL);
			Send(fileno(stdout), to_string(cliaddr.sin_port).c_str(), strlen(to_string(cliaddr.sin_port).c_str()), MSG_NOSIGNAL);
			Send(fileno(stdout), "\n", 1, MSG_NOSIGNAL);
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
					// cout << "* client " << Inet_ntop(AF_INET, (sockaddr*)&CAddr[i].sin_addr, ip, INET_ADDRSTRLEN) << ":" << CAddr[i].sin_port << " disconnected" << endl;
					Send(fileno(stdout), "* client ", 9, MSG_NOSIGNAL);
					Inet_ntop(AF_INET, (sockaddr*)&cliaddr.sin_addr, ip, INET_ADDRSTRLEN);
					Send(fileno(stdout), ip, strlen(ip), MSG_NOSIGNAL);
					Send(fileno(stdout), ":", 1, MSG_NOSIGNAL);
					Send(fileno(stdout), to_string(CAddr[i].sin_port).c_str(), strlen(to_string(CAddr[i].sin_port).c_str()), MSG_NOSIGNAL);
					Send(fileno(stdout), " disconnected\n", 14, MSG_NOSIGNAL);
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
		// for(i = 0; i <= maxi; i++) cout<< client[i] << " ";
		// cout << endl;
	}
	return 0;
}