#ifndef STD_HEADER
#define STD_HEADER
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#endif

void Close(int fd);
void Dup2(int fd1, int fd2);