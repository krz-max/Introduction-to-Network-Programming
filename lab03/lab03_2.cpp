#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/signal.h>
#include <MySocket.h>

using namespace std;

#define BUFFERSIZE 5000

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

    char garbage[BUFFERSIZE];
    memset(garbage, 0, sizeof(garbage));
	int second = 0;
	int bytepercycle = 0, bytepersecond = 0;
	int BytesPerSecond = 1005000;
	double rate = atof(argv[1]);
	BytesPerSecond = BytesPerSecond*rate;
	int BytesPerCycle = BUFFERSIZE*rate;
    struct timespec t = {0, 5000000};
	gettimeofday(&_t0, NULL);
	while(1) {
		second += t.tv_nsec;
		if(second == 1000000000){
			second = 0;
			while(bytepersecond < BytesPerSecond-BUFFERSIZE) {
				bytepersecond += send(sockfd, garbage, BUFFERSIZE, 0);
			}
			while(bytepersecond < BytesPerSecond) bytepersecond += send(sockfd, garbage, BytesPerSecond-bytepersecond, 0);
			bytesent += bytepersecond;
			bytepersecond = 0;
		}
		else if(bytepersecond < BytesPerSecond) {
			while(bytepercycle < BytesPerCycle-BUFFERSIZE)
				bytepercycle += send(sockfd, garbage, BUFFERSIZE, 0);
			while(bytepercycle < BytesPerCycle) bytepercycle += send(sockfd, garbage, BytesPerCycle-bytepercycle, 0);
			bytepersecond += bytepercycle;
			bytepercycle = 0;
		}
		nanosleep(&t, NULL);
	}

	return 0;
}
