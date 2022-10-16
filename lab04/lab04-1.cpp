#include "../Header/MySocket.h"
#include <sys/types.h>
#include <sys/wait.h>

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
			cerr << "Fork failed\n";
			return -1;
		}
		else if ( pid == 0 ) {
			cout << "New connection from " << Inet_ntop(AF_INET, (sockaddr*)&cliaddr.sin_addr, ip, INET_ADDRSTRLEN) << ":" << cliaddr.sin_port << endl;
			int errfd = dup(STDERR_FILENO);
			argv[argc] = NULL;
			// new = old
			dup2(connfd, STDIN_FILENO);
			dup2(connfd, STDOUT_FILENO);
			dup2(connfd, STDERR_FILENO);
			close(listenfd);
			if( execvp(argv[2], argv+2) < 0 ) {
				dup2(errfd, STDERR_FILENO);
				err_sys(argv[2]);
			}
			_exit(0);
		}
		else {
			close(connfd);			/* parent closes connected socket */
		}
	}
	return 0;
}