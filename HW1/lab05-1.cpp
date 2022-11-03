#include "header.h"
#define MAXUSER 1024
#define MAXLINE 2000
#define NAMELEN 20
#define WELCOME 10000
#define ARRIVE 10001
#define LEAVE 10002
#define NAMECHANGE 10003
#define SELFCHANGE 10004
#define NAMELIST 10005
#define WRONGCMD 10006
#define CHAT 10007
#define SARRIVE 10000
#define SLEAVE 10001

using namespace std;

char nameStr[1024][NAMELEN], newname[NAMELEN];
int i, maxi, maxfd, listenfd, connfd, nowfd;
int howManyUsers;
int nready;
struct pollfd client[1024];
ssize_t n;
fd_set rset, allset;
char buf[MAXLINE], ip[MAXLINE];
socklen_t clilen;
struct sockaddr_in cliaddr;
socklen_t CLen[1024];
struct sockaddr_in CAddr[1024];

void nick(list<string> &arg_str, int fd)
{
	std::string CMD, nickname;
	CMD = arg_str.front();
	arg_str.pop_front();
	nickname = arg_str.front();
	arg_str.pop_front();
	return;
}
void user(list<string> &arg_str, int fd)
{
	std::string CMD, username, servername, hostname, realname;
	CMD = arg_str.front();
	arg_str.pop_front();
	servername = arg_str.front();
	arg_str.pop_front();
	username = arg_str.front();
	arg_str.pop_front();
	hostname = arg_str.front();
	arg_str.pop_front();
	realname = arg_str.front();
	arg_str.pop_front();
	dprintf(fd, ":mircd 001 user1 :Welcome to the minimized IRC daemon!\n");
	dprintf(fd, ":mircd 251 user1 :There are 1 users and 0 invisible on 1 server\n");
	dprintf(fd, ":mircd 375 user1 :- mircd Message of the day -\n");
	dprintf(fd, ":mircd 372 user1 :-  Hello, World!\n");
	dprintf(fd, ":mircd 372 user1 :-               @                    _ \n");
	dprintf(fd, ":mircd 372 user1 :-   ____  ___   _   _ _   ____.     | |\n");
	dprintf(fd, ":mircd 372 user1 :-  /  _ `'_  \ | | | '_/ /  __|  ___| |\n");
	dprintf(fd, ":mircd 372 user1 :-  | | | | | | | | | |   | |    /  _  |\n");
	dprintf(fd, ":mircd 372 user1 :-  | | | | | | | | | |   | |__  | |_| |\n");
	dprintf(fd, ":mircd 372 user1 :-  |_| |_| |_| |_| |_|   \____| \___,_|\n");
	dprintf(fd, ":mircd 372 user1 :-  minimized internet relay chat daemon\n");
	dprintf(fd, ":mircd 372 user1 :-\n");
	dprintf(fd, ":mircd 376 user1 :End of message of the day\n");
	return;
}
void ping(list<string> &arg_str, int fd)
{
	return;
}
void lst(list<string> &arg_str, int fd)
{
	return;
}
void join(list<string> &arg_str, int fd)
{
	dprintf(fd, ":user2 JOIN #chal1\n");
	dprintf(fd, ":mircd 331 user2 #chal1 :No topic is set\n");
	dprintf(fd, ":mircd 353 user2 #chal1 :user2\n");
	dprintf(fd, ":mircd 353 user2 #chal1 :user2\n");
	dprintf(fd, ":mircd 353 user2 #chal1 :user2\n");
	dprintf(fd, ":mircd 353 user2 #chal1 :user2\n");
	dprintf(fd, ":mircd 366 user2 #chal1 :End of Names List\n");
	return;
}
void topic(list<string> &arg_str, int fd)
{
	return;
}
void names(list<string> &arg_str, int fd)
{
	return;
}
void part(list<string> &arg_str, int fd)
{
	return;
}
void users(list<string> &arg_str, int fd)
{
	dprintf(fd, ":mircd 392 :UserID   Terminal  Host\n");
	dprintf(fd, ":mircd 393 :%-8s %-9s %-8s\n", "kk1", "-", "localhost");
	dprintf(fd, ":mircd 393 :%-8s %-9s %-8s\n", "kk2", "-", "localhost");
	dprintf(fd, ":mircd 393 :%-8s %-9s %-8s\n", "kk3", "-", "localhost");
	dprintf(fd, ":mircd 393 :%-8s %-9s %-8s\n", "kk4", "-", "localhost");
	dprintf(fd, ":mircd 394 :End of users\n");
	return ;
}
void privmsg(list<string> &arg_str, int fd)
{
	return;
}
void quit(list<string> &arg_str, int fd)
{
	return;
}
typedef void (*FuncPtr)(list<string> &arg_str, int fd);
unordered_map<string, FuncPtr> kCommandFuntions = {
	{"NICK", nick},
	{"USER", user},
	{"PING", ping},
	{"LIST", lst},
	{"JOIN", join},
	{"TOPIC", topic},
	{"NAMES", names},
	{"PART", part},
	{"USERS", users},
	{"PRIVMSG", privmsg},
	{"QUIT", quit}};

void sig_hand(int signo) {}
void Notify(int sockfd, int userid, int MessageId)
{
	time_t raw_time = time(0);
	struct tm *TIME = localtime(&raw_time);
	char timeStr[40];
	sprintf(timeStr, "%04d-%02d-%02d %d:%d:%d ", TIME->tm_year + 1900, TIME->tm_mon + 1, TIME->tm_mday, TIME->tm_hour, TIME->tm_min, TIME->tm_sec);
	switch (MessageId)
	{
	case WELCOME:
		// dprintf(sockfd, "%s *** Welcome to the simple CHAT server\n", timeStr);
		// dprintf(sockfd, "%s *** Total %d users online now. Your name is %s\n", timeStr, howManyUsers, nameStr[userid]);
		dprintf(sockfd, ":mircd 001 user1 :Welcome to the minimized IRC daemon!\n");
		dprintf(sockfd, ":mircd 251 user1 :There are 1 users and 0 invisible on 1 server\n");
		dprintf(sockfd, ":mircd 375 user1 :- mircd Message of the day -\n");
		dprintf(sockfd, ":mircd 372 user1 :-  Hello, World!\n");
		dprintf(sockfd, ":mircd 372 user1 :-               @                    _ \n");
		dprintf(sockfd, ":mircd 372 user1 :-   ____  ___   _   _ _   ____.     | |\n");
		dprintf(sockfd, ":mircd 372 user1 :-  /  _ `'_  \ | | | '_/ /  __|  ___| |\n");
		dprintf(sockfd, ":mircd 372 user1 :-  | | | | | | | | | |   | |    /  _  |\n");
		dprintf(sockfd, ":mircd 372 user1 :-  | | | | | | | | | |   | |__  | |_| |\n");
		dprintf(sockfd, ":mircd 372 user1 :-  |_| |_| |_| |_| |_|   \____| \___,_|\n");
		dprintf(sockfd, ":mircd 372 user1 :-  minimized internet relay chat daemon\n");
		dprintf(sockfd, ":mircd 372 user1 :-\n");
		dprintf(sockfd, ":mircd 376 user1 :End of message of the day\n");
		break;
	case ARRIVE:
		for (int k = 1; k <= maxi; k++)
		{
			if (client[k].fd < 0)
				continue;
			if (k != userid)
			{
				dprintf(client[k].fd, " *** User %s has just landed on the server\n", nameStr[userid]);
			}
		}
		break;
	case LEAVE:
		for (int k = 1; k <= maxi; k++)
		{
			if (client[k].fd < 0)
				continue;
			if (k != userid)
			{
				dprintf(client[k].fd, " *** User %s has left the server\n", nameStr[userid]);
			}
		}
		break;
	case NAMECHANGE:
		for (int k = 1; k <= maxi; k++)
		{
			if (client[k].fd == -1)
				continue;
			if (k != userid)
			{
				dprintf(client[k].fd, " *** User %s renamed to %s\n", nameStr[userid], newname);
			}
		}
		break;
	case SELFCHANGE:
		dprintf(sockfd, " *** Nickname changed to %s\n", newname);
		break;
	case NAMELIST:
		dprintf(sockfd, "------------------------\n");
		for (int k = 1; k <= maxi; k++)
		{
			if (client[k].fd == -1)
				continue;
			if (k == userid)
				dprintf(client[k].fd, " * ");
			else
				dprintf(sockfd, "   ");
			Inet_ntop(AF_INET, (sockaddr *)&CAddr[k].sin_addr, ip, INET_ADDRSTRLEN);
			dprintf(sockfd, "%s     %s:%d\n", nameStr[k], ip, CAddr[k].sin_port);
		}
		break;
	case WRONGCMD:
		dprintf(sockfd, " *** Unknown or incomplete command : %s\n", buf);
		break;
	case CHAT:
		for (int k = 1; k < maxi + 1; k++)
		{
			if (client[k].fd == -1)
				continue;
			if (k == userid)
				continue;
			dprintf(client[k].fd, "%s: %s\n", nameStr[userid], buf);
		}
	default:
		break;
	}
}

void Parser(stringstream &ss, list<string> &args)
{
	string str;
	while (getline(ss, str, ' '))
	{
		if (str[0] == '#')
		{
			// JOIN, TOPIC, PART, PRIVMSG, NAMES has <channel> as argument
			// #channel
			string temp = str.substr(1);
			args.push_back(str);
		}
		else if (str[0] == ':') // last argument
		{
			string temp = str.substr(1);
			args.push_back(str);
			return;
		}
		else
		{
			args.push_back(str);
		}
	}
}
// parse command and do action
// return :
//      true: close connection
//      false: keep reading
bool ParseCommand(stringstream &ss, int fd, list<string> &cmd)
{
	Parser(ss, cmd);
	fprintf(stdout, "cmd size: %ld\n", cmd.size());
	for (string it : cmd)
		fprintf(stdout, "parsed cmd: %s\n", it.c_str());

	if (cmd.empty())
		return false;
	// string now;
	// now = cmd.front();
	// cmd.pop_front();
	// while (!cmd.empty())
	// {
	// 	if (now == "NICK")
	// 	{
	// 		while (!cmd.empty())
	// 			cmd.pop_front();
	// 		return false;
	// 	}
	// 	else if (now == "USER")
	// 	{
	// 		cmd.pop_front();
	// 		cmd.pop_front();
	// 		cmd.pop_front();
	// 		cmd.pop_front();
	// 		// cout <<client->fd << endl;
	// 		dprintf(client->fd, ":mircd 001 user1 :Welcome to the minimized IRC daemon!");
	// 		dprintf(client->fd, ":mircd 251 user1 :There are 1 users and 0 invisible on 1 server");
	// 		dprintf(client->fd, ":mircd 375 user1 :- mircd Message of the day -");
	// 	}
	// 	else
	// 	{
	// 		cmd.pop_front();
	// 		return false;
	// 	}
	// }
	auto func = kCommandFuntions.find(cmd.front());
	if (func != kCommandFuntions.end())
	{
		fprintf(stdout, "Incomming Cmd: %s\n", func->first.c_str());
		(*func->second)(cmd, fd);

		if (func->first == "QUIT")
			return true;
	}
	return false;
}
int main(int argc, char **argv)
{
	Start_TCP_Server(&listenfd, strtol(argv[1], NULL, 10));
	signal(SIGPIPE, sig_hand);
    int flag = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int));
	howManyUsers = 0;

	client[0].fd = listenfd;
	client[0].events = POLLRDNORM;
	for (i = 1; i < 1024; i++)
		client[i].fd = -1; /* -1 indicates available entry */
	maxi = 0;
	for (;;)
	{
		nready = Poll(client, maxi + 1, INFTIM);

		if (client[0].revents & POLLRDNORM)
		{
			clilen = sizeof(cliaddr);
			connfd = Accept(listenfd, (sockaddr *)&cliaddr, &clilen);
			for (i = 1; i < 1024; i++)
				if (client[i].fd < 0)
				{
					client[i].fd = connfd; /* save descriptor */
					break;
				}
			if (i == 1024)
				err_sys("too many clients");

			Inet_ntop(AF_INET, (sockaddr *)&cliaddr.sin_addr, ip, INET_ADDRSTRLEN);
			printf("* client connected from %s:%d\n", ip, cliaddr.sin_port);
			howManyUsers++;

			client[i].events = POLLRDNORM;
			CLen[i] = clilen;
			CAddr[i] = cliaddr;
			if (i > maxi)
				maxi = i; /* max index in client[] .fdarray */

			if ((n = read(connfd, buf, MAXLINE)) < 0)
			{
				if (errno == ECONNRESET)
				{
					Close(connfd);
					client[i].fd = -1;
				}
				else
					err_sys("read error");
			}
			else if (n == 0)
			{
				Close(connfd);
				client[i].fd = -1;
				Inet_ntop(AF_INET, (sockaddr *)&CAddr[i].sin_addr, ip, INET_ADDRSTRLEN);
				printf("* client %s:%d disconnected\n", ip, CAddr[i].sin_port);
				Notify(connfd, i, LEAVE);
				howManyUsers--;
			}
			else
			{
				stringstream ss;
				list<string> cmd;
				ss.str(buf);
				ParseCommand(ss, client[i].fd, cmd);
			}

			if (--nready <= 0)
				continue; /* no more readable descriptors */
		}

		for (i = 1; i <= maxi; i++)
		{ /* check all clients for data */
			if ((nowfd = client[i].fd) < 0)
				continue;
			if (client[i].revents & (POLLRDNORM | POLLERR))
			{
				bzero(buf, sizeof(buf));
				if ((n = read(nowfd, buf, MAXLINE)) < 0)
				{
					if (errno == ECONNRESET)
					{
						/*4connection reset by client */
						Close(nowfd);
						client[i].fd = -1;
					}
					else
						err_sys("read error");
				}
				else if (n == 0)
				{
					/*4connection closed by client */
					Close(nowfd);
					client[i].fd = -1;
					Inet_ntop(AF_INET, (sockaddr *)&CAddr[i].sin_addr, ip, INET_ADDRSTRLEN);
					printf("* client %s:%d disconnected\n", ip, CAddr[i].sin_port);
					Notify(connfd, i, LEAVE);
					howManyUsers--;
				}
				else
				{
					stringstream ss;
					list<string> cmd;
					ss.str(buf);
					ParseCommand(ss, client[i].fd, cmd);
				}

				if (--nready <= 0)
					break; /* no more readable descriptors */
			}
		}
	}
	return 0;
}