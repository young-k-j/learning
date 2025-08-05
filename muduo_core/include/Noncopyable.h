#pragma once  // 防止头文件重复包含

//Noncopyable 被继承后，派生类对象禁用拷贝构造和赋值构造
/* 派生类的拷贝构造函数必须递归调用其所有基类的拷贝构造函数。
   如果基类的拷贝构造是 = delete 的，
   派生类就无法定义一个合法的拷贝构造函数。*/
class Noncopyable
{
public:
    Noncopyable(const Noncopyable&) = delete;
    Noncopyable &operator=(const Noncopyable&) = delete;
protected:
    Noncopyable() = default;
    ~Noncopyable() = default;
};