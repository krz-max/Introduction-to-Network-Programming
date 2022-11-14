#include "header.h"

#define IP "127.0.0.1"
#define PORT 10002
#define HOST "inp111.zoolab.org"
#define MAXLINE 18000
#define NCLIENTS 52
#define ITER 280
using namespace std;

int cmdfd;
int sinkfd[NCLIENTS];

void sig_hand(int en)
{
    Writen(cmdfd, "/report\n", 8);
    exit(0);
}

int main(int argc, char **argv)
{
    signal(SIGTERM, sig_hand);
    Start_TCP_Client(&cmdfd, strtol(argv[2], NULL, 10), argv[1]);
    char dummy[MAXLINE];
    memset(&dummy, '1', sizeof(dummy));
    for (int i = 0; i < NCLIENTS; i++)
        Start_TCP_Client(&sinkfd[i], strtol(argv[2], NULL, 10) + 1, argv[1]);
    dprintf(cmdfd, "/reset\n");
    for (int j = 0; j < ITER; j++)
    {
        for (int i = 0; i < NCLIENTS; i++)
        {
            Writen(sinkfd[i], dummy, MAXLINE);
        }
    }
    dprintf(cmdfd, "/report\n");
}