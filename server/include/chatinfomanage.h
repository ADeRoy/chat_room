#ifndef CHATINFOMANAGE_H
#define CHATINFOMANAGE_H
#include "common.h"
#include <map>

class ChatInfoManage
{
public:
    ChatInfoManage();
public:
    static ChatInfoManage* getInstance(){
        if(NULL == m_pInstance)
        {
            m_pInstance = new ChatInfoManage();
        }
        return m_pInstance;
    }
public:

private:
    static ChatInfoManage* m_pInstance;
    
};

#endif // CHATINFOMANAGE_H
