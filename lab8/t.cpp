#include <time.h>
#include <iostream>
using namespace std;
typedef struct {
	unsigned seq;
	struct timeval tv;
}	ping_t;
int main(){
    ping_t *p;
    cout << sizeof(*p) << endl;
    cout << sizeof(unsigned) + sizeof(struct timeval) << endl;
    return 0;
}