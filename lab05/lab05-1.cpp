#include "../Header/MySocket.h"
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
int printTimeString(){
	return 0;
}
void Notify(int sockfd, int userid, int MessageId){
	time_t raw_time = time(0);
	struct tm *TIME = localtime(&raw_time);
	char timeStr[40];
	int timeStrLen = sprintf(timeStr, "%04d-%02d-%02d %d:%d:%d ", TIME->tm_year+1900, TIME->tm_mon+1, TIME->tm_mday, TIME->tm_hour, TIME->tm_min, TIME->tm_sec);
	switch(MessageId){
		case WELCOME:
			// dup2(sockfd, fileno(fileno(stdout)));
			// cout << timeStr << " *** Welcome to the simple CHAT server" << endl;
			// cout << timeStr << " *** Total " << howManyUsers << " users online now. Your name is <" << nameStr[userid] << ">" << endl;
			// printTimeString();
			send(sockfd, timeStr, timeStrLen, MSG_NOSIGNAL);
			// return ;
			send(sockfd, " *** Welcome to the simple CHAT server\n", 39, MSG_NOSIGNAL);
			send(sockfd, timeStr, timeStrLen, MSG_NOSIGNAL);
			send(sockfd, " *** Total ", 11, MSG_NOSIGNAL);
			send(sockfd, to_string(howManyUsers).c_str(), strlen(to_string(howManyUsers).c_str()), MSG_NOSIGNAL);
			send(sockfd, " users online now. Your name is <", 33, MSG_NOSIGNAL);
			send(sockfd, nameStr[userid], strlen(nameStr[userid]), MSG_NOSIGNAL);
			send(sockfd, ">\n", 2, MSG_NOSIGNAL);
			break;
		case ARRIVE:
			for(int k = 1; k <= maxi; k++) {
				if(client[k].fd < 0) 
					continue;
				if(k != userid) {
					// send(client[k].fd, timeStr, timeStrLen, MSG_NOSIGNAL);
					send(client[k].fd, " *** User ", 10, MSG_NOSIGNAL);
					send(client[k].fd, nameStr[userid], strlen(nameStr[userid]), MSG_NOSIGNAL);
					send(client[k].fd, " has just landed on the server\n", 31, MSG_NOSIGNAL);
					// cout << timeStr << " *** User " << nameStr[userid] << " has just landed on the server" << endl;
				}
			}
			break;
		case LEAVE:
			for(int k = 1; k <= maxi; k++) {
				if(client[k].fd < 0) 
					continue;
				if(k != userid) {
					// send(client[k].fd, timeStr, timeStrLen, MSG_NOSIGNAL);
					send(client[k].fd, " *** User ", 10, MSG_NOSIGNAL);
					send(client[k].fd, nameStr[userid], strlen(nameStr[userid]), MSG_NOSIGNAL);
					send(client[k].fd, " has just left the server\n", 26, MSG_NOSIGNAL);
					// cout << timeStr << " *** User " << nameStr[userid] << " has just landed on the server" << endl;
				}
				// if(client[k].fd == -1) continue;
				// if(k != userid){
				// 	dup2(client[k].fd, fileno(fileno(stdout)));
				// 	cout << timeStr << " *** User " << nameStr[userid] << " has just left the server" << endl;
				// }
			}
			break;
		case NAMECHANGE:
			for(int k = 1; k <= maxi; k++) {
				if(client[k].fd == -1) 
					continue;
				if(k != userid) {
					// dup2(client[k].fd, STDOUT_FILENO);
					// cout << " *** User " << nameStr[userid] << " renamed to " << newname << endl;
					send(client[k].fd, " *** User ", 10, MSG_NOSIGNAL);
					send(client[k].fd, nameStr[userid], strlen(nameStr[userid]), MSG_NOSIGNAL);
					send(client[k].fd, " renamed to ", 12, MSG_NOSIGNAL);
					send(client[k].fd, newname, strlen(newname), MSG_NOSIGNAL);
					send(client[k].fd, "\n", 1, MSG_NOSIGNAL);
				}
			}
			break;
		case SELFCHANGE:			
			// dup2(sockfd, fileno(fileno(stdout)));
			// send(sockfd, timeStr, timeStrLen, MSG_NOSIGNAL);
			send(sockfd, " *** Nickname changed to ", 25, MSG_NOSIGNAL);
			send(sockfd, newname, strlen(newname), MSG_NOSIGNAL);
			send(sockfd, "\n", 1, MSG_NOSIGNAL);
			// cout << timeStr << " *** Nickname changed to " << newname << endl;
			break;
		case NAMELIST:
			// dup2(sockfd, fileno(fileno(stdout)));
			// cout << "---" << endl;
			send(sockfd, "---\n", 4, MSG_NOSIGNAL);
			for(int k = 1; k <= maxi; k++) {
				if(client[k].fd == -1) 
					continue;
				if(k == userid) send(client[k].fd, " * ", 3, MSG_NOSIGNAL);
				else send(sockfd, "   ", 3, MSG_NOSIGNAL);
				send(sockfd, nameStr[k], strlen(nameStr[k]), MSG_NOSIGNAL);
				send(sockfd, "     ", 5, MSG_NOSIGNAL);
				Inet_ntop(AF_INET, (sockaddr*)&CAddr[k].sin_addr, ip, INET_ADDRSTRLEN);
				send(sockfd, ip, strlen(ip), MSG_NOSIGNAL);
				send(sockfd, ":", 1, MSG_NOSIGNAL);
				send(sockfd, to_string(CAddr[k].sin_port).c_str(), strlen(to_string(CAddr[k].sin_port).c_str()), MSG_NOSIGNAL);
				send(sockfd, "\n", 1, MSG_NOSIGNAL);
				// cout << setw(NAMELEN) << left << nameStr[k] << Inet_ntop(AF_INET, (sockaddr*)&CAddr[k].sin_addr, ip, INET_ADDRSTRLEN) << ":" << CAddr[k].sin_port << endl;
			}
			break;
		case WRONGCMD:
			// dup2(sockfd, fileno(fileno(stdout)));
			// cout << timeStr << " *** Unknown or incomplete command : " << buf << endl;
			// send(sockfd, timeStr, timeStrLen, MSG_NOSIGNAL);
			send(sockfd, " *** Unknown or incomplete command : ", 37, MSG_NOSIGNAL);
			send(sockfd, buf, strlen(buf), MSG_NOSIGNAL);
			send(sockfd, "\n", 1, MSG_NOSIGNAL);
			break;
		case CHAT:
			for(int k = 1; k < maxi+1; k++) {
				if(client[k].fd == -1) 
					continue;
				if(k == userid) 
					continue;
				// dup2(client[k].fd, STDOUT_FILENO);
				// send(client[k].fd, timeStr, timeStrLen, MSG_NOSIGNAL);
				send(client[k].fd, nameStr[userid], strlen(nameStr[userid]), MSG_NOSIGNAL);
				send(client[k].fd, ": ", 2, MSG_NOSIGNAL);
				send(client[k].fd, buf, strlen(buf), MSG_NOSIGNAL);
				send(client[k].fd, "\n", 1, MSG_NOSIGNAL);
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
		buf[n-1] = NULL;
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
    Start_TCP_Server(&listenfd, strtol(argv[1], NULL, 10));
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

			write(fileno(stdout), "* client connected from ", 24);
			Inet_ntop(AF_INET, (sockaddr*)&cliaddr.sin_addr, ip, INET_ADDRSTRLEN);
			write(fileno(stdout), ip, strlen(ip));
			write(fileno(stdout), ":", 1);
			write(fileno(stdout), to_string(cliaddr.sin_port).c_str(), strlen(to_string(cliaddr.sin_port).c_str()));
			write(fileno(stdout), "\n", 1);
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
					write(fileno(stdout), "* client ", 9);
					Inet_ntop(AF_INET, (sockaddr*)&CAddr[i].sin_addr, ip, INET_ADDRSTRLEN);
					write(fileno(stdout), ip, strlen(ip));
					write(fileno(stdout), ":", 1);
					write(fileno(stdout), to_string(CAddr[i].sin_port).c_str(), strlen(to_string(CAddr[i].sin_port).c_str()));
					write(fileno(stdout), " disconnected\n", 14);
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