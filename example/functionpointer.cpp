#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

using namespace std;

void my_handler(int s){
    switch (s)
    {
    case SIGUSR1:
        /* code */
    printf("Hello, world: %d\n", s);
        break;
    case SIGUSR2:
    printf("Hello, world: %d\n", s);
        break;
    case SIGINT:
    printf("Hello, world: %d\n", s);
        break;
    case SIGTERM:
    printf("Hello, world: %d\n", s);
        break;
    default:
    printf("killed: &d\n", s);
        return;
    }
}

int a(int x, int y){ return x+y; }
int b(int x, int y){ return x*y; }

int main(){
    int (*x)(int, int) = NULL;
    int i = 0;
    x = a;
    printf("%d %d, %d\n", 3, 4, x(3, 4));
    x = b;
    printf("%d %d, %d\n", 3, 4, x(3, 4));

    signal(SIGUSR1, my_handler);
    signal(SIGUSR2, my_handler);
    signal(SIGINT, my_handler);
    signal(SIGTERM, my_handler);
    // kill now cannot stop this program
    // use kill -9 pid instead

    while(1){
        printf("%d ", i++);
        fflush(stdout);
        sleep(1);
    }
    return 0;
}