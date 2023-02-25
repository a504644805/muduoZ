#include <functional>
#include <iostream>

using namespace std;

void func() {
}
int main() {
    function<void()> f;
    if (!f) cout << "f is NULL" << endl;
    f = func;
    if (f) cout << "f is not NULL" << endl;
    return 0;
}