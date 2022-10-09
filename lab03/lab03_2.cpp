#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/signal.h>
#include <MySocket.h>

using namespace std;

static struct timeval _t0;
static unsigned long long bytesent = 0;

double tv2s(struct timeval *ptv) {
	return 1.0*(ptv->tv_sec) + 0.000001*(ptv->tv_usec);
}

void handler(int s) {
	struct timeval _t1;
	double t0, t1;
	gettimeofday(&_t1, NULL);
	t0 = tv2s(&_t0);
	t1 = tv2s(&_t1);
	fprintf(stderr, "\n%lu.%06lu %llu bytes sent in %.6fs (%.6f Mbps; %.6f MBps)\n",
		_t1.tv_sec, _t1.tv_usec, bytesent, t1-t0, 8.0*(bytesent/1000000.0)/(t1-t0), (bytesent/1000000.0)/(t1-t0));
	exit(0);
}

int main(int argc, char *argv[]) {
    int sockfd;
    Start_TCP_Client(&sockfd, 10003, HostToIp("inp111.zoolab.org"));

	signal(SIGINT,  handler);
	signal(SIGTERM, handler);

    char garbage[1374];
    memset(garbage, 0, sizeof(garbage));
    struct timespec t = {0, 1};
    if(strcmp(argv[1], "1") == 0){
        t.tv_nsec = 1230000;
    }
    else if(strcmp(argv[1], "1.5") == 0){
        t.tv_nsec =  775000;
    }
    else if(strcmp(argv[1], "2") == 0){
        t.tv_nsec =  535000;
    }
    else{
        t.tv_nsec =  310000;
    }
	gettimeofday(&_t0, NULL);
	while(1) {
		bytesent += send(sockfd, garbage, 1374, 0);
		nanosleep(&t, NULL);
	}

	return 0;
}
