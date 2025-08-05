// 负责记录当前实际时间
#pragma once

#include <iostream>
#include <string>

class Timestamp
{
public:
    Timestamp();
    // explicit关键字 防止类的构造函数或转换函数被隐式调用
    explicit Timestamp(int64_t microSenondsSinceEpoch);
    static Timestamp now();
    std::string toString() const;
private:
	//int64_t 固定64位有符号整数，宽度跨平台一致
    int64_t microSenondsSinceEpoch_;
};