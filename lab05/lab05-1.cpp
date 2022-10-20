#include "../Header/MySocket.h"
#include <iomanip>
#define MAXUSER 2000
#define MAXLINE 2000

using namespace std;

char Welome[] = {" *** Welcome to the simple CHAT server\n"};
char Total[] = {" *** Total "};
char Online[] = {" users online now. Your name is <"};
char nameStr[MAXUSER][MAXLINE];
int					i, maxi, maxfd, listenfd, connfd, sockfd;
int					nready, client[FD_SETSIZE];
ssize_t				n;
fd_set				rset, allset;
char				buf[MAXLINE], ip[MAXLINE];
socklen_t			clilen;
struct sockaddr_in	cliaddr, servaddr;
char newname[MAXLINE], oldname[MAXLINE];

int getTimeString(char* timeStr){
    time_t times = time(NULL);
    struct tm* utcTime = gmtime(&times);
    int timeStrLen = sprintf(timeStr, "%04d-%02d-%02d %02d:%02d:%02d ", utcTime->tm_year+1900, utcTime->tm_mon+1, utcTime->tm_mday, utcTime->tm_hour+8, utcTime->tm_min, utcTime->tm_sec);
    return timeStrLen;
}
void printWelcomeMessage(int sockfd, int users) {
	char timeStr[40];
    int timeStrLen = getTimeString(timeStr);
	dup2(sockfd, STDOUT_FILENO);
	cout << timeStr << " *** Welcome to the simple CHAT server" << endl;
	cout << timeStr << " *** Total " << users << " users online now. Your name is <" << nameStr[users-1] << ">" << endl;
    return ;
}
void Broadcast_Left(char* name) {
	char timeStr[40];
    int timeStrLen = getTimeString(timeStr);
	for (i = 0; i <= maxi; i++) {	/* check all clients for data */
		if ( (sockfd = client[i]) < 0)
			continue;
		if (FD_ISSET(sockfd, &rset)) {
			dup2(sockfd, STDOUT_FILENO);
			cout << timeStr << " *** User <" << name << "> has just left on the server\n";
			if (--nready <= 0)
				break;				/* no more readable descriptors */
		}
	}
	return ;
}
void cli_opr(int sockfd) {
	// printWelcomeMessage(sockfd);
	return ;
}
void command_prompt(int sockfd, int id, int users, int port) {
	dup2(sockfd, STDOUT_FILENO);
	char timeStr[40];
  	int timeStrLen = getTimeString(timeStr);
	if ( !strncmp(buf, "/name", 5) ) {
		cout << timeStr << " *** Nickname changed to " << newname << endl;
		strcpy(oldname, nameStr[id]);
		strcpy(nameStr[id], newname);
	}
	else if(!strncmp(buf, "/who", 4)){
		cout << "---" << endl;
		for(int i = 0; i < users; i++) {
			if(i == id) cout << " * ";
			else cout << "   ";
			cout << setw(10) << left << nameStr[i] << "   " << Inet_ntop(AF_INET, &cliaddr.sin_addr, ip, INET_ADDRSTRLEN) << ":" << cliaddr.sin_port << endl;
		}
		cout << "---" << endl;
	}
	else {
		cout << timeStr << " *** Unknown or incomplete command " << buf;
	}
	return ;
}
int main(int argc, char** argv){

    Start_TCP_Server(&listenfd, strtol(argv[1], NULL, 10));

	maxfd = listenfd;			/* initialize */
	maxi = -1;					/* index into client[] array */
	for (i = 0; i < FD_SETSIZE; i++)
		client[i] = -1;			/* -1 indicates available entry */
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);

	for ( ; ; ) {
		rset = allset;		/* structure assignment */
		nready = select(maxfd+1, &rset, NULL, NULL, NULL);

		if (FD_ISSET(listenfd, &rset)) {	/* new client connection */
			clilen = sizeof(cliaddr);
			connfd = Accept(listenfd, (sockaddr *) &cliaddr, &clilen);

			for (i = 0; i < FD_SETSIZE; i++)
				if (client[i] < 0) {
					client[i] = connfd;	/* save descriptor */
					break;
				}
			if (i == FD_SETSIZE) {
				cerr << "too many clients\n";
                return -1;
            }

			FD_SET(connfd, &allset);	/* add new descriptor to set */
			if (connfd > maxfd)
				maxfd = connfd;			/* for select */
			if (i > maxi)
				maxi = i;				/* max index in client[] array */
			strcpy(nameStr[maxi], "kkmelon");
			printWelcomeMessage(connfd, maxi+1);
			if (--nready <= 0)
				continue;				/* no more readable descriptors */
		}
		for (i = 0; i <= maxi; i++) {	/* check all clients for data */
			if ( (sockfd = client[i]) < 0)
				continue;
			if (FD_ISSET(sockfd, &rset)) {
				if ( (n = Read(sockfd, buf, MAXLINE)) == 0) {
					/*4connection closed by client */
					close(sockfd);
					FD_CLR(sockfd, &allset);
					client[i] = -1;
					Broadcast_Left(nameStr[sockfd]);
				} else {
					if ( buf[0] == '/' ) {
							for(int j = 6; j < strlen(buf)-1; j++) newname[j-6] = buf[j];
							command_prompt(sockfd, i, maxi+1, strtol(argv[1], NULL, 10));
							if(!strncmp(buf, "/name", 5)){
								for(int j = 0; j < maxi+1; j++) {
									dup2(client[j], STDOUT_FILENO);
									if(j != i) cout << " *** User " << oldname << " renamed to " << newname << endl;
								}
							}
					}
				}
					// Writen(sockfd, buf, n);

				if (--nready <= 0)
					break;				/* no more readable descriptors */
			}
		}
	}
	return 0;
}