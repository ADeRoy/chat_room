/**	
 *  @file     BCFSysClock.cpp
 *  @brief    BCF系统时钟类
 *  @version  0.0.1
 *  @author   yulichun<yulc@bstar.com.cn>
 *  @date     2010-06-18    Created it
 *  @note     无
 */


#include "MySysClock.h"


/**
 *@fn           BCFSysClock::getOSUpTime(int* pSec, int* pMicroSec)
 *@brief        返回系统开机以来的时间值
 *@param[out] pSec      系统运行自现在的时间值(秒),不能为NULL
 *@param[out] pMicroSec 精确度(微秒), 该值允许为NULL,表示忽略.
 *@return    0: 成功;\n -1: 失败,调用 BCFError::getSysError(std::string&) 获取错误信息
 *
 */
int BCFSysClock::getOSUpTime(int* pSec, int* pMicroSec)
{
#if defined(linux)

  struct timespec tp;

  int rs = clock_gettime(CLOCK_MONOTONIC, &tp);
  if(rs != 0)
    return rs;

  *pSec = tp.tv_sec;
  if(pMicroSec != NULL)
    *pMicroSec = tp.tv_nsec/1000;  // 纳秒转换成微秒

#endif

  return 0;
}

/**
 *@fn                BCFSysClock::getTimeOfday(time_t* pSec, int* pMicroSec)
 *@brief             获取当前系统时间,精确到微秒
 *@param[out] pSec      当前系统时间(秒)
 *@param[out] pMicroSec 当前系统时间(微秒)
 *@return    0: 成功;\n -1: 失败,调用 BCFError::getSysError(std::string&) 获取错误信息
 *
 */
int BCFSysClock::getTimeOfday(time_t* pSec, int* pMicroSec)
{
#if defined(linux)
  struct timeval tv;
  gettimeofday(&tv, NULL);
  *pSec = tv.tv_sec;
  *pMicroSec = tv.tv_usec;
#endif


  return 0;
}

