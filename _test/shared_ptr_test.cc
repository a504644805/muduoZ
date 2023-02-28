#include <functional>
#include <iostream>
#include <memory>
#include <unordered_set>
#include <vector>
using namespace std;

std::function<void()> func;

class A {
   public:
    typedef std::function<void()> Cb;
    A(Cb cb) : cb_(cb) {
        func = std::bind(&A::f, this);  // A is managed by shared_ptr, but we give raw pointer of A to Others
        data = new char(10);
    }
    ~A() {
        delete data;
        data = NULL;
        cout << "A is destroyed" << endl;
    }
    void f() {
        cb_();
        cout << "try to access A.data[0]" << endl;  // it is supposed that A has been destroyed
        cout << "data[0] is " << data[0] << endl;
    }
    void set_cb(const Cb& cb) { cb_ = cb; }

   private:
    Cb cb_;
    char* data;
};

class B {
   public:
    void clearA() {
        alist_.clear();
    }
    void genThenAdd_A() {
        alist_.push_back(shared_ptr<A>(new A(std::bind(&B::clearA, this))));
    }

   private:
    vector<shared_ptr<A>> alist_;
};

int main() {
    B b;
    b.genThenAdd_A();
    func();
    return 0;
}