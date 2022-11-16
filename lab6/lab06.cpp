#include "header.h"
#define MAXLINE 1000000

using namespace std;

int i, maxi, maxfd, listenfd, connfd, nowfd;
int nready;
struct pollfd client[200];
int writefd;
ssize_t n;
fd_set rset, allset;
char buf[MAXLINE];
socklen_t clilen;
struct sockaddr_in cliaddr;
int sinkfd;
struct timeval start, e;

inline std::string &rtrim(std::string &s)
{
	if (s.empty())
		return s;

	s.erase(s.find_last_not_of(" \n\r") + 1);
	return s;
}
void sig_hand(int signo) {}
int main(int argc, char **argv)
{
	long long bytes_counter = 0;
	Start_TCP_Server(&listenfd, strtol(argv[1], NULL, 10));
	int flag = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int));
	Start_TCP_Server(&sinkfd, strtol(argv[1], NULL, 10) + 1);
	setsockopt(sinkfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int));
	signal(SIGPIPE, sig_hand);
	gettimeofday(&start, 0);

	client[0].fd = listenfd;
	client[1].fd = sinkfd;
	client[0].events = POLLRDNORM;
	client[1].events = POLLRDNORM;
	for (i = 2; i < 200; i++)
	{
		client[i].fd = -1; /* -1 indicates available entry */
	}
	maxi = 0;
	for (;;)
	{
		nready = Poll(client, maxi + 1, INFTIM);
		if (client[0].revents & POLLRDNORM)
		{
			clilen = sizeof(cliaddr);
			connfd = Accept(listenfd, (sockaddr *)&cliaddr, &clilen);
			for (i = 2; i < 200; i++)
				if (client[i].fd < 0)
				{
					client[i].fd = connfd; /* save descriptor */
					break;
				}
			if (i == 200)
				err_sys("too many clients");
			client[i].events = POLLRDNORM;
			if (i > maxi)
				maxi = i; /* max index in client[] .fdarray */

			if (--nready <= 0)
				continue; /* no more readable descriptors */
		}

		if (client[1].revents & POLLRDNORM)
		{
			clilen = sizeof(cliaddr);
			connfd = Accept(sinkfd, (sockaddr *)&cliaddr, &clilen);
			for (i = 2; i < 200; i++)
				if (client[i].fd < 0)
				{
					client[i].fd = connfd; /* save descriptor */
					break;
				}
			if (i == 200)
				err_sys("too many clients");

			client[i].events = POLLRDNORM;
			if (i > maxi)
				maxi = i; /* max index in client[] .fdarray */

			if (--nready <= 0)
				continue; /* no more readable descriptors */
		}
		// client command
		for (i = 2; i <= maxi; i++)
		{ /* check all clients for data */
			if ((nowfd = client[i].fd) < 0)
				continue;
			if (client[i].revents & (POLLRDNORM | POLLERR))
			{
				bzero(buf, MAXLINE);
				if ( (n = read(nowfd, buf, MAXLINE)) == 0 )
				{
					/*4connection closed by client */
					Close(nowfd);
					client[i].fd = -1;
				}
				else
				{
					if (buf[0] == '/')
					{
						if (strcmp(buf, "/reset\n") == 0)
						{
							gettimeofday(&start, 0);
							fprintf(stdout, "%.6f", (float)start.tv_sec * 1000 + (start.tv_usec / 1000.0));
							cout << " RESET " << bytes_counter << endl;
							bytes_counter = 0;
						}
						else if (strcmp(buf,"ping\n") == 0)
						{
							gettimeofday(&e, 0);
							fprintf(stdout, "%.6f", (float)e.tv_sec * 1000 + (e.tv_usec / 1000.0));
							cout << " PONG " << endl;
						}
						else if (strcmp(buf,"clients\n") == 0)
						{
							gettimeofday(&e, 0);
							fprintf(stdout, "%.6f", (float)e.tv_sec * 1000 + (e.tv_usec / 1000.0));
							cout << " CLIENTS " << maxi - 2 << endl;
						}
						else
						{
							gettimeofday(&e, 0);
							int sec = e.tv_sec - start.tv_sec;
							int usec = e.tv_usec - start.tv_usec;
							float elaps_sec = (float)sec + (usec / 1000000.0);
							fprintf(stdout, "%.6f", (float)e.tv_sec * 1000 + (e.tv_usec / 1000.0));
							cout << " REPORT " << bytes_counter << " " << elaps_sec << " " << abs(8 * bytes_counter / (1000000.0 * elaps_sec)) << "Mbps" << endl;
						}
					}
					else
					{
						// cout << "rcv: " << n << endl;
						bytes_counter += n;
					}
				}

				if (--nready <= 0)
					break; /* no more readable descriptors */
			}
		}
	}
	return 0;
}
