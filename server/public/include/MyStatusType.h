/**	
 *  @file     MyStatusType.hpp
 *  @brief    框架所有组件接口的状态类型值定义
 *  @version  0.0.1
 *  @note     无
 */

#ifndef _MYSTATUSTYPE_H_
#define _MYSTATUSTYPE_H_

#if defined(_MSC_VER)
#include <winsock2.h>
#endif

#include <vector>

/** 状态值类型声明\n
   *  BCF提供的所有接口,当需要用函数的返回值来表示不同的执行结果时,\n
   *  需将函数的返回值声明为StatusType
   */
enum StatusType
{
    ST_OK = 0,       /**< 正确,无异常  */
    ST_NOTIMPL = 1,  /**< 暂未实现该方法 */
    ST_SYSERROR = 2, /**< 系统API调用错误,调用 BCFError::getSysError(std::string&) 获取错误信息 */

    ST_PARAMERROR, /**< 参数不合法  */
    ST_UNINIT,     /**< 对象没有初始化  */
    ST_REINIT,     /**< 对象重复初始化  */
    ST_EXCEEDMAX,  /**< 资源数目或大小超出最大值  */
    ST_TIMEOUT,    /**< 超时  */

    ST_FILE_CRE_EXSIT_FI,     /**< 创建的文件已经存在  */
    ST_FILE_MODE_ERROR,       /**< 打开文件模式错误  */
    ST_FILE_SET_BUFFER_ERROR, /**< 设置文件缓存失败  */
    ST_FILE_OPEN_ERROR,       /**< 打开一个不存在的文件  */

    ST_BSCP_PACKET_INVALID, /**< 包头格式错误 */
    ST_BSCP_PACKET_LENGTH,  /**< 数据包大小错误  */
    ST_BSCP_PACKET_ERROR,   /**< 应答中error字段错误  */
    ST_BSCP_DISCONNECTED,   /**< 连接未建立或已断开 */
    ST_MEM_ERROR,           /**< 内存池内存分配或释放错误 */

    ST_DBQUERY_ERROR, /**< 数据库操作错误 */

    ST_Z_BUF_ERROR, /*用来存放解压缩的buf太小*/
    ST_Z_MEM_ERROR, /*内存错误*/
    ST_Z_DATA_ERROR /*被解压的数据错误*/
};

#if defined(_MSC_VER)
typedef int ssize_t;
typedef int off_t;
typedef int socklen_t;
#endif

#if defined(_MSC_VER)
typedef SOCKET SocketHandle;
#elif defined(linux)
typedef int SocketHandle;
#define INVALID_SOCKET (-1)
#endif

/** 通用的二进制数据类型
 */
typedef std::vector<char> DeBinary;

#endif
