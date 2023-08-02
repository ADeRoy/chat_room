/**	
 *  @file     BCFSysClock.hpp
 *  @brief    BCF系统时钟类
 *  @version  0.0.1
 *  @note     无
 */

#ifndef _MYSYSCLOCK_H_
#define _MYSYSCLOCK_H_

#if defined(linux)
#include <time.h>
#include <sys/time.h>
#endif

/** 系统精确时钟类,该类提供系统时钟相关的函数\n
   *  可用于相对时间的处理
   */
class BCFSysClock
{

public:
    static int getOSUpTime(int *pSec, int *pMicroSec);

    static int getTimeOfday(time_t *pSec, int *pMicroSec);

private:
    /** @brief 空构造函数 */
    BCFSysClock() {}

    /** @brief 空析构函数 */
    ~BCFSysClock() {}
};

/**
   *  用于计算时间差异的类,精确到毫秒
   */
class TimeDifference
{
public:
    TimeDifference()
    {
        sec = 0;
        microSec = 0;
        BCFSysClock::getOSUpTime(&sec, &microSec);
    }

    /**
     *@fn           TimeDifference::operator-(const TimeDifference& td) const
     *@brief        重载-运算符,计算两个对象的时间差值, 注意: 当时间值过大时,会溢出
     *@param[in] td 被减对象
     *return        两个对象的时间差值(毫秒)
     */
    int operator-(const TimeDifference &td) const
    {
        if (this == &td)
            return 0;

        return ((this->sec - td.sec) * 1000 + (this->microSec - td.microSec) / 1000);
    }

private:
    int sec;
    int microSec;
};

#endif
