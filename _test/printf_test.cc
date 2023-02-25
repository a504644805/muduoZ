#include <stdio.h>
#include <string.h>

#include <iostream>
using namespace std;
int main() {
    const char* str = "Hello World\n";
    cout << sizeof str << endl;
    cout << strlen(str) << endl;

    char buf[64];
    cout << sizeof buf << endl;
}