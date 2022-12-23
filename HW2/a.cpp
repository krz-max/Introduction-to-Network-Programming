#include <iostream>
using namespace std;
void foo(int **b){
    *b = *b + 1;
}
int main(){
    int a = 10;
    int *b = &a;
    cout << b << endl;
    foo(&b);
    cout << b << endl;
}