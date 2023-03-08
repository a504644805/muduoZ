#ifndef MUDUOZ_BASE_NONCOPYABLE_H
#define MUDUOZ_BASE_NONCOPYABLE_H

// Ref: https://stackoverflow.com/questions/31940886/is-there-a-stdnoncopyable-or-equivalent
//      https://github.com/chenshuo/muduo/blob/master/muduo/base/noncopyable.h
namespace muduoZ {
class noncopyable {
   public:
    noncopyable(const noncopyable&) = delete;
    void operator=(const noncopyable&) = delete;

   protected:
    noncopyable() = default;
    ~noncopyable() = default;
};

}  // namespace muduoZ

#endif