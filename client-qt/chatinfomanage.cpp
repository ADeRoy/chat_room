#include "chatinfomanage.h"

ChatInfoManage* ChatInfoManage::m_pInstance = m_pInstance;

ChatInfoManage::ChatInfoManage()
{

}

GroupUserInfo *ChatInfoManage::getUserInfo(int account)
{
    GroupUserInfo* userInfo =NULL;
    mapGroupInfo::iterator iter = g_GroupUserInfoMap.find(0);
    if(iter!=g_GroupUserInfoMap.end()){
        mapGroupUserInfo::iterator ite = iter->second.find(account);
        ChatLogInfo()<<"### size:"<<iter->second.size();
        if(ite!=iter->second.end()){
            userInfo = ite->second;
            ChatLogInfo()<< "account:" << userInfo->m_account <<",username:" << QString(userInfo->m_userName);
            return userInfo;
        }
        else{
            ChatLogInfo()<<"find UserInfo fail..";
        }
    }
    else {
        ChatLogInfo()<<"not find group 0..";
    }
    return userInfo;
}

int ChatInfoManage::addGroupChatInfo(GroupChatInfo *pGroupChatInfo)
{
    std::pair<mapGroupChatInfo::iterator, bool> InsertPair = g_GroupCharInfoMap.insert(std::make_pair(pGroupChatInfo->m_account,pGroupChatInfo));
    if(InsertPair.second == true)
        return 0;
    return -1;
}

GroupChatInfo *ChatInfoManage::getGroupChatInfo(int groupAccount)
{
    GroupChatInfo* groupChatInfo = NULL;
    mapGroupChatInfo::iterator iter = g_GroupCharInfoMap.find(groupAccount);
    if(iter!=g_GroupCharInfoMap.end()){
        groupChatInfo = iter->second;
    }
    else {
        ChatLogInfo()<<"not find chatgroup indo";
        return NULL;
    }
    return groupChatInfo;
}

int ChatInfoManage::addGroupUserInfo(int groupAccount, GroupUserInfo *groupUserInfo)
{
    //将群用户信息添加到群里面
    mapGroupInfo::iterator iter = g_GroupUserInfoMap.find(groupAccount);
    if(iter != g_GroupUserInfoMap.end()){
        std::pair<mapGroupUserInfo::iterator, bool> InsertPair = iter->second.insert(std::make_pair(groupUserInfo->m_account,groupUserInfo));
        if(InsertPair.second == true)
            return 0;
        else {
            ChatLogInfo()<<"insert groupAccount:"<<groupAccount<<" error";
        }
    }
    else {
        mapGroupUserInfo groupUserInfoMap;
        //群 map 表中插入用户数据
        std::pair<mapGroupUserInfo::iterator, bool> insertPair = groupUserInfoMap.insert(std::make_pair(groupUserInfo->m_account,groupUserInfo));
        if(insertPair.second != true)
            return -1;
        std::pair<mapGroupInfo::iterator, bool> InsertPair = g_GroupUserInfoMap.insert(std::make_pair(groupAccount,groupUserInfoMap));
        if(InsertPair.second == true)
            return 0;
    }
    return -1;
}

GroupUserInfo *ChatInfoManage::getGroupUserInfo(int groupAccount, int userAccount)
{
    GroupUserInfo* groupUserInfo =NULL;
    mapGroupInfo::iterator iter = g_GroupUserInfoMap.find(groupAccount);
    if(iter!=g_GroupUserInfoMap.end()){
        mapGroupUserInfo::iterator ite = iter->second.find(userAccount);
        if(ite!=iter->second.end()){
            groupUserInfo = ite->second;
        }
    }
    return groupUserInfo;
}

int ChatInfoManage::addFriendInfo(FriendInfo *friendInfo)
{
    std::pair<mapFriendInfo::iterator, bool> InsertPair = m_FriendInfoMap.insert(std::make_pair(friendInfo->m_account,friendInfo));
    if(InsertPair.second == true)
        return 0;
    return -1;
}

FriendInfo *ChatInfoManage::getFriendInfo(int account)
{
    FriendInfo* friendInfo = NULL;
    mapFriendInfo::iterator iter = m_FriendInfoMap.find(account);
    if(iter!=m_FriendInfoMap.end()){
        friendInfo = iter->second;
    }
    else {
        ChatLogInfo()<<"not find chatgroup indo";
        return NULL;
    }
    return friendInfo;
}
