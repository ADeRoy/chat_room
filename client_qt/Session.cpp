#include "Session.h"
#include <QTcpSocket>
Session::Session(QTcpSocket* socket)
{
    m_socket = socket;
    m_type = RECV_HEAD;
    m_head = NULL;
    m_bufLen = 0;
    m_readPos = 0;
    m_body = NULL;
    m_isFinish = false;
    m_head = new char[sizeof(DeMessageHead)];
    m_bufLen = sizeof(DeMessageHead);
}

Session::~Session(){
    if (m_head != NULL)
        delete[] m_head;
    if (m_body != NULL)
        delete[] m_body;
    if (m_socket != NULL){
        m_socket->close();
        m_socket = NULL;
    }
};

int Session::readEvent()
{
    int ret = 0;
    switch (m_type)
    {
    case RECV_HEAD:
        ret = recvHead();
        break;
    case RECV_BODY:
        ret = recvBody();
        break;
    default:
        break;
    }
    if (ret == RET_AGAIN)
        return readEvent();
    return ret;
}

int Session::recvHead()
{
    if (m_head == NULL)
    {
        m_head = new char[sizeof(DeMessageHead)];
        assert(m_head != NULL);
        m_bufLen = sizeof(DeMessageHead);
        m_readPos = 0;
    }
    int len = m_socket->read(m_head + m_readPos, m_bufLen - m_readPos);
    if (len < 0)
        return RET_ERROR;
    if (len == 0)
        return RET_END;
    m_readPos += len;
    if (m_readPos == m_bufLen)
    {
        m_type = RECV_BODY;
        int bufLen = ((DeMessageHead *)m_head)->length;
        m_body = new char[bufLen];

        assert(m_body != NULL);
        m_bufLen = bufLen;
        m_readPos = 0;  //读取的位置置零
        return RET_AGAIN;
    }
    return 0;
}

int Session::recvBody()
{
    /* 先判断读取的位置是否是 ((DeMessageHead*)m_head)->length 接收头指定的长度 */
    if (m_readPos == m_bufLen)
    {
        m_type = RECV_HEAD;
        handleMsgBase();
        m_isFinish = true;
        return RET_AGAIN;
    }
    /* 读取指定 Body 大小的数据 */
    int len = m_socket->read(m_body + m_readPos, m_bufLen - m_readPos);

    if (len < 0)
        return RET_ERROR;

    m_readPos += len;

    /* 判断读取的位置是否是 ((DeMessageHead*)m_head)->length 接收头指定的长度 */
    if (m_readPos == m_bufLen)
    {
        m_type = RECV_HEAD;
        handleMsgBase();
        m_bufLen = 0;
        m_isFinish = true;
        return RET_AGAIN;
    }
    return RET_OK;
}

int Session::handleMsgBase(){
    recvMsg *rMsg = new recvMsg();
    rMsg->head = m_head;
    rMsg->body = m_body;
    rMsg->bodyLen = m_bufLen;
    emit signal_handleMsg(rMsg);
//    handleMsg(rMsg);
    m_head = NULL;
    m_body = NULL;
    return RET_OK;
}

void Session::cleanSession(){

}
