#include "header.h"
#include <sys/wait.h>
#include <sys/types.h>

#define PORT 9877
#define MAXLINE 2000
#define LISTENQ 1024

using namespace std;

void sighandler(int signo)
{
	pid_t pid;
	int stat;
	while ( (pid = waitpid(-1, &stat, 0)) > 0 )
		printf("child %d terminated\n", pid);
	return ;		
}

int main(int argc, char** argv){
	char ip[INET_ADDRSTRLEN];
	pid_t pid;
	int listenfd, connfd;
	socklen_t			clilen;
	struct sockaddr_in	cliaddr;
	Start_TCP_Server(&listenfd, strtol(argv[1], NULL, 10));

	signal(SIGCHLD, sighandler);

	for ( ; ; ) {
		// new client
		clilen = sizeof(cliaddr);
		connfd = Accept(listenfd, (sockaddr *) &cliaddr, &clilen);
		if ( (pid = fork()) < 0 ) {
			fprintf(stderr, "Fork failed\n");
			return -1;
		}
		else if ( pid == 0 ) {
			fprintf(stdout, "New connection from %s:%d\n", Inet_ntop(AF_INET, (sockaddr*)&cliaddr.sin_addr, ip, INET_ADDRSTRLEN), cliaddr.sin_port);
			int errfd = dup(STDERR_FILENO);
			argv[argc] = NULL;
			// new = old
			Dup2(connfd, STDIN_FILENO);
			Dup2(connfd, STDOUT_FILENO);
			Dup2(connfd, STDERR_FILENO);
			Close(listenfd);
			if( execvp(argv[2], argv+2) < 0 ) {
				Dup2(errfd, STDERR_FILENO);
				err_sys(argv[2]);
			}
			_exit(0);
		}
		else {
			Close(connfd);			/* parent closes connected socket */
		}
	}
	return 0;
}