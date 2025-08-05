#include <time.h>

#include "Timestamp.h"

//默认构造函数
Timestamp::Timestamp():microSenondsSinceEpoch_(0)
{
}

//重载构造函数，根据传入的微秒数初始化成员变量 microSecondsSinceEpoch_
Timestamp::Timestamp(int64_t microSenondsSinceEpoch)
    :microSenondsSinceEpoch_(microSenondsSinceEpoch)   
{
}

//记录当前时间, time(NULL)回当前的从1970开始的秒级时间戳
Timestamp Timestamp::now(){
    return Timestamp(time(NULL));
}

/*将 Timestamp 对象格式化为一个可读的字符串，
 *类似 "yyyy/mm/dd hh:mm:ss" 的格式*/ 

 std::string Timestamp::toString()const
 {
    char buf[128] = {0};
    // local 将 time_t 时间戳转换为 tm 结构的标准库函数
    tm *tm_time = localtime(&microSenondsSinceEpoch_);
    // int snprintf(char* str, size_t size, const char* format, ...);
    snprintf(buf, 128, "%4d/%02d/%02d %02d:%02d:%02d",
             tm_time->tm_year + 1900,
             tm_time->tm_mon + 1,
             tm_time->tm_mday,
             tm_time->tm_hour,
             tm_time->tm_min,
             tm_time->tm_sec);
    return buf;
 }