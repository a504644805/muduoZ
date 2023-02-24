#include <pthread.h>

#include <iostream>
using namespace std;

void* threadFunc(void*) {
    while (1)
        ;
}
int main() {
    pthread_t tid1;
    pthread_t tid2;
    pthread_t tid3;
    pthread_create(&tid1, NULL, threadFunc, NULL);
    pthread_create(&tid2, NULL, threadFunc, NULL);
    pthread_create(&tid3, NULL, threadFunc, NULL);
    cout << tid1 << " " << tid2 << " " << tid3 << endl;
    return 0;
}