#include <stdio.h>
#include <string.h>

#include <iostream>
using namespace std;
class C {
    C(int a) {}
    C(int& c) = delete;
};

int main() {
}