#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <string.h>

#include <vector>
#include <iostream>
#include <string>
#include <sys/wait.h>

using namespace std;

#define err_quit(m) \
    {               \
        perror(m);  \
        exit(-1);   \
    }
#define MAXLINE 1024
struct Node{
    int x;
    int y;
    Node(int x, int y):x(x), y(y){};
};
bool isSafe(vector<vector<int>>& board, int row, int col, int num){
    for(int i = 0; i < 9; i++){
        if(board[row][i] == num){
            return false;
        }
    }
    for(int i = 0; i < 9; i++){
        if(board[i][col] == num){
            return false;
        }
    }
    int bx = row - row % 3, by =col - col%3;
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 3; j++){
            if(board[bx+i][by+j] == num){
                return false;
            }
        }
    }
    return true;
}
bool find(vector<vector<int>> &board, int &x, int &y){
    for(int i = 0; i < 9; i++){
        for(int j = 0; j < 9; j++){
            if(board[i][j] == -1){
                x = i;
                y = j;
                return true;
            }
        }
    }
    return false;
}
bool _solver(vector<vector<int>>& board){
    int row, col;
    if(!find(board, row, col)) return true;

    for(int i = 1; i <= 9; i++){
        if(isSafe(board, row, col, i) && board[row][col] == -1){
            board[row][col] = i;
            if(_solver(board))
                return true;
            board[row][col] = -1;
        }
    }
    return false;
}
void str_cli(FILE *fp, int sockfd)
{
    int n;
    string sendline, recvline;
    char buf[MAXLINE];
    write(sockfd, "S", 1);
    n = read(sockfd, buf, MAXLINE);
    recvline = buf;
    recvline = recvline.substr(4);
    vector<vector<int>> board(9, vector<int>(9, -1));
    cout << recvline.length() << endl;
    for(int i = 0; i < 9; i++){
        for(int j = 0; j < 9; j++){
            if(recvline[9*i+j] == '.'){
                board[i][j] = -1;
            }
            else{
                board[i][j] = recvline[9*i+j]-'0';
            }
        }
    }
    vector<Node> ans;
    for(int i = 0; i < 9; i++){
        for(int j = 0; j < 9; j++){
            cout << board[i][j] << " ";
            if(board[i][j] < 0){
                ans.push_back(Node(i,j));
            }
        }
        cout << endl;
    }
    _solver(board);
    cout << "solved" << endl;
    for(int i = 0; i < 9; i++){
        for(int j = 0; j < 9; j++){
            cout << board[i][j] << " ";
        }
        cout << endl;
    }
    char t[MAXLINE];
    write(sockfd, "P\n", 2);
    n = read(sockfd, buf, MAXLINE);
    cout << buf << endl;
    sleep(1);
    for(int i = 0; i < ans.size(); i++){
        // cout << ans[i].x << " " << ans[i].y << " " << board[ans[i].x][ans[i].y] << endl;
        sprintf(t, "V %d %d %d\n", ans[i].x, ans[i].y, board[ans[i].x][ans[i].y]);
        write(sockfd, "S\n", 2);
        sleep(1);
        write(sockfd, t, strlen(t));
        sleep(1);
    }
    write(sockfd, "P\n", 2);
    sleep(1);
    write(sockfd, "C\n", 2);
    sleep(1);
}
void strcli(FILE *fp, int sockfd)
{
    char sendline[MAXLINE], recvline[MAXLINE];
    while (fgets(sendline, MAXLINE, fp) != NULL)
    {
        write(sockfd, sendline, strlen(sendline));
        if(read(sockfd, recvline, MAXLINE) == 0){
            err_quit("str_cli: server terminated prematurely");
        }
        fputs(recvline, stdout);
    }
}
int main(int argc, char **argv)
{
    int sockfd;
    struct sockaddr_un servaddr;
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sun_family = AF_UNIX;
    strcpy(servaddr.sun_path, "/sudoku.sock");
    connect(sockfd, (sockaddr *)&servaddr, sizeof(servaddr));
    pid_t p = fork();
    if(p == 0) str_cli(stdin, sockfd); /* do it all */
    else{
        wait(nullptr);
        strcli(stdin, sockfd);
    }
    exit(0);
}