#include <unistd.h>
#include <stdio.h>

int main(){
    int pid = fork();
    if(pid == 0){
        while(1){
            sleep(1);
            printf("%d, parent %d\n", getpid(), getppid());
        }
    }
    else{
        sleep(3);
    }
    return 0;
}
