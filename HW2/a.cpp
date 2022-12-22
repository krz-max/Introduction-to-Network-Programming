#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdint.h>
int main(){
    int16_t t = -12819;
    printf("%d\n", (uint16_t)t);
    return 0;
}