#include "../Header/MySocket.h"
#define MAXUSER     1024
#define MAXLINE     2000
#define NAMELEN     20
#define NICK        10000
#define USER        10001
#define PING        10002
#define LIST        10003
#define JOIN        10004
#define TOPIC       10005
#define NAMES       10006
#define PART        10007
#define USERS       10008
#define PRIVMSG     10009
#define LEAVE       10010
/*
Additional remarks for the commands listed above are summarized as follows. These remarks might make your implementation simpler.

A <nickname> cannot contain spaces.

A <channel> must be prefixed with a pond (#) symbol, e.g., #channel.

The last parameter must be prefixed with a colon (’:’).

By default, there is no response from NICK and USER commands unless there are errors. But the server must send the message of the day (motd) to a client once both commands are accepted.

JOIN a channel is always a success. If the channel does not exist, your server should automatically create the channel.

For successful JOIN and PART commands, the server must respond :<nickname> <the command & params received from the server> first, followed by the rest of the responses.

PRIVMSG can be only used to send messages into a channel in our implementation. You do not have to implement sending a message to a user privately.

=== ERROR MESSAGE ===
Additional remarks for the general form of the server responded messages are summarized as follows. These remarks might make your implementation simpler.

The prefix can be a fixed string, e.g., mircd or your preferred server name.

NNN is the response/error code composed of three digits.

<nickname> is required when it is known for an associated connection.

The number of <params> depends on the corresponding response/error code.

The last parameter must be prefixed with a colon (’:’).

To deliver a PRIVMSG message received from <userX>, the received message from <userX> is prepended with the prefix :<userX> and then delivered to all users in the channel. The resulted message should look like :<userX> PRIVMSG #<channel> :<message>.


NICK <nickname>
USER <username> <hostname> <servername> <realname>
PING [<message>]
LIST [<channel>]
JOIN <channel>
TOPIC <channel> [<topic>]
NAMES [<channel>]
PART <channel>
USERS
PRIVMSG <channel> <message>
QUIT
*/
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
void parseString(char* cmd){

}

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