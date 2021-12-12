#include "MsgHandle.h"
#include "common.h"

Session::Session(int socket)
{
    m_isLogin = -1;
    m_account = -1;
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
    if (m_socket != -1){
        close(m_socket);
        m_socket = -1;
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

    int len = read(m_socket, m_head + m_readPos, m_bufLen - m_readPos);
    if (len < 0)
        return RET_ERROR;
    if (len == 0)
        return RET_EXIT;
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
    /*  先判断读取的位置是否是 ((DeMessageHead*)m_head)->length 接收头指定的长度  */
    if (m_readPos == m_bufLen)
    {
        m_type = RECV_HEAD;
        handleMsgBase();
        m_isFinish = true;
        return RET_AGAIN;
    }
    /*读取指定 Body 大小的数据*/
    int len = read(m_socket, m_body + m_readPos, m_bufLen - m_readPos);

    if (len < 0)
        return RET_ERROR;

    m_readPos += len;

    /*  判断读取的位置是否是 ((DeMessageHead*)m_head)->length 接收头指定的长度  */
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
    handleMsg(rMsg);
    m_head = NULL;
    m_body = NULL;
    return RET_OK;
}

int Session::handleMsg(recvMsg *rMsg)
{
    DeMessagePacket *packet = (DeMessagePacket *)(rMsg->body);
    DeMessageHead *head = (DeMessageHead *)(rMsg->head);
    //printf("mark:%s,encoded:%d,length:%d,version:%d\n", head->mark, head->encoded, head->length, head->version);

    void *reqData = NULL;
    int reqDataLen = -1;
    short ret = RET_OK;
    printf("recv command:%d\n", packet->command);
    switch (packet->command)
    {
        case CommandEnum_Registe:
            // handleRegiste(this,rMsg->body + sizeof(DeMessagePacket));
            break;
        case CommandEnum_Login:
            if(packet->mode == 3){
                handleLogin(this, rMsg->body + sizeof(DeMessagePacket));
            }else if(packet->mode == 2 && packet->error == 0){
                m_isLogin = 1;
                printf("登陆成功..\n");
            }
            break;
        case CommandEnum_Logout:
            // handleLogout(this, rMsg->body + sizeof(DeMessagePacket));
            break;
        case CommandEnum_GroupChat:
            handleGroupChat(this, rMsg->body + sizeof(DeMessagePacket));
            break;
        case CommandEnum_AddFriend:
            if(packet->mode == 1){
                handleAddFriendReq(rMsg->body + sizeof(DeMessagePacket));
            }else if(packet->mode == 2){
                handleAddFriendResp(rMsg->body + sizeof(DeMessagePacket));
            }
            break;
        case CommandEnum_delFriend:
            break;
        case CommandEnum_PrivateChat:
            handlePrivateChat(rMsg->body + sizeof(DeMessagePacket));
            break;
        case CommandEnum_CreateGroup:
            break;
        case CommandEnum_GetGroupList:
            break;
        case CommandEnum_GetGroupInfo:
            getGroupList(this, rMsg->body + sizeof(DeMessagePacket));
            break;
        case CommandEnum_GetFriendList:
            break;
    }
}

void Session::sendMsg(int socket,void*buf,int bufLen,int type,int error,int mode){
    DeMessageHead header;
    memcpy(header.mark, "DE", sizeof(header.mark));
    header.encoded = '0';
    header.version = '0';
    header.length = sizeof(DeMessagePacket) + bufLen;

    char *p = (char *)malloc(header.length);
    DeMessagePacket *pPacket = (DeMessagePacket *)p;
    pPacket->mode = mode;
    pPacket->sequence = getSeqNum();
    pPacket->command = type;
    pPacket->error = error;
    if(buf)
        memcpy(p + sizeof(DeMessagePacket), buf, bufLen);

    char *sendMsg = new char[sizeof(DeMessageHead) + header.length];
    memset(sendMsg, 0, sizeof(DeMessageHead) + header.length);
    memcpy(sendMsg, &header, sizeof(DeMessageHead));
    memcpy(sendMsg + sizeof(DeMessageHead), p, header.length);
    free(p);
    write(socket, sendMsg, sizeof(DeMessageHead) + header.length);
}

void Session::cleanSession(){

}

int Session::handleRegiste(void*arg,void* msg){
    Session* pthis = (Session*)arg;
    RegistInfoResp* registInfo = (RegistInfoResp*)msg;
    printf("account:%s\n",registInfo->m_account);
}

int Session::handleLogin(void*arg,void*msg){
    Session* pthis = (Session*)arg;
    GroupUserInfo* loginInfo = (GroupUserInfo*)msg;
    printf("new user login ,account:%d,name:%s\n",loginInfo->m_account,loginInfo->m_userName);

    mapGroupInfo::iterator iter = g_GroupUserInfoMap.find(0);
    if(iter!=g_GroupUserInfoMap.end()){
        std::pair<mapGroupUserInfo::iterator, bool> Insert_Pair = iter->second.insert(std::make_pair(loginInfo->m_account,loginInfo));
        if(Insert_Pair.second == true)
            printf("新用户上线了，保存用户信息..\n");
        else{
            printf("Insert new userInfo fail\n");
        }
    }
    return 0;

}

int Session::handleLogout(void*arg,void*msg){
    Session* pthis = (Session*)arg;

}

int Session::handleGroupChat(void *arg, void *msg){
    Session* pthis = (Session*)arg;
    GroupChatReq* groupChatReq = (GroupChatReq*)msg;
    //获取聊天群信息
    GroupChatInfo* groupChatInfo = NULL;
    mapGroupChatInfo::iterator groupIte = g_GroupCharInfoMap.find(groupChatReq->m_GroupAccount);
    if(groupIte!=g_GroupCharInfoMap.end()){
        groupChatInfo = groupIte->second;
    }
    else{
        printf("not find chatgroup\n");
    }
    // printf("g_GroupUserInfoMap size:%d\n",g_GroupUserInfoMap.size());
    // printf("msgLen:%d,group:%d,type:%d\n",groupChatReq->m_msgLen,groupChatReq->m_GroupAccount,groupChatReq->m_type);
    //从群用户信息总表中获取用户信息
    char*buf = (char*)malloc(groupChatReq->m_msgLen);
    memmove(buf,(char*)msg + sizeof(GroupChatReq),groupChatReq->m_msgLen);
    mapGroupInfo::iterator iter = g_GroupUserInfoMap.find(groupChatReq->m_GroupAccount);
    if(iter!=g_GroupUserInfoMap.end()){
        mapGroupUserInfo infomap = iter->second;
        mapGroupUserInfo::iterator ite = infomap.find(groupChatReq->m_UserAccount);
        if(ite!=infomap.end()){
            GroupUserInfo* userInfo = ite->second;
            if(groupChatInfo!=NULL){
                printf("[%s][%s]:%s\n",groupChatInfo->m_groupName,userInfo->m_userName,buf);
                fflush(stdout); //偶尔上面的输出不了，只能情况下缓冲区
            }
            else{
                printf("chatgroup not find..\n");
            }
        }
        else{
            printf("not find userInfo..\n");
        }
    }
    else{
        printf("not find chatgroup userInfoMap..\n");
    }
}

int Session::handlePrivateChat(void *msg)
{
    printf("recv a privateChat Msg..\n");
    PrivateChatReq* privateChatReq = (PrivateChatReq*)msg;
    //获取聊天群信息
    FriendInfo* friendInfo = NULL;
    mapFriendInfo::iterator privateIte = m_friendInfoMap.find(privateChatReq->m_UserAccount);
    if(privateIte!=m_friendInfoMap.end()){
        friendInfo = privateIte->second;
    }
    else{
        printf("not find friend\n");
    }

    char*buf = (char*)malloc(privateChatReq->m_msgLen);
    memmove(buf,(char*)msg + sizeof(PrivateChatReq),privateChatReq->m_msgLen);

    //从群用户信息总表中获取用户信息
    mapGroupInfo::iterator iter = g_GroupUserInfoMap.find(0);
    if(iter!=g_GroupUserInfoMap.end()){
        mapGroupUserInfo infomap = iter->second;
        mapGroupUserInfo::iterator ite = infomap.find(privateChatReq->m_UserAccount);
        if(ite!=infomap.end()){
            GroupUserInfo* userInfo = ite->second;
            if(friendInfo!=NULL){
                printf("[%s][%s]:%s\n",friendInfo->m_userName,userInfo->m_userName,buf);
                fflush(stdout); //偶尔上面的输出不了，只能清空下缓冲区
            }
            else{
                printf("chatgroup not find..\n");
            }
        }
        else{
            printf("not find userInfo..\n");
        }
    }
    else{
        printf("not find chatgroup userInfoMap..\n");
    }
}

int Session::getGroupList(void *arg, void *msg)
{
    Session* pthis = (Session*)arg;
    GetGroupInfoResp* pResp = (GetGroupInfoResp*)msg;
    mapGroupUserInfo groupUserInfoMap;
    //添加群信息到群管理
    GroupChatInfo* groupChatInfo = (GroupChatInfo*)malloc(sizeof(GroupChatInfo));
    groupChatInfo->m_account = pResp->m_GroupAccount;
    strncpy(groupChatInfo->m_groupName,pResp->m_groupName,sizeof(pResp->m_groupName));
    g_GroupCharInfoMap.insert(std::make_pair(pResp->m_GroupAccount,groupChatInfo));
    //添加群成员信息到群成员管理
    // printf("group:%d,size:%d\n",pResp->m_GroupAccount,pResp->m_size);
    for(int i=0;i<pResp->m_size;i++){
        GroupUserInfo* pUserInfo = (GroupUserInfo*)((char*)msg + sizeof(GetGroupInfoResp) + sizeof(GroupUserInfo)*i);
        GroupUserInfo* pNewUserInfo = (GroupUserInfo*)malloc(sizeof(GroupUserInfo));
        memmove(pNewUserInfo,pUserInfo,(sizeof(GroupUserInfo)));
        //群map表中插入用户数据
        groupUserInfoMap.insert(std::make_pair(pUserInfo->m_account,pNewUserInfo));
        printf("account:%d,name:%s,right:%d,size:%d\n",pUserInfo->m_account,pUserInfo->m_userName,pUserInfo->m_right,groupUserInfoMap.size());
    }
    std::pair<mapGroupInfo::iterator, bool> Insert_Pair = g_GroupUserInfoMap.insert(std::make_pair(pResp->m_GroupAccount,groupUserInfoMap));
    if(Insert_Pair.second == true)  
        cout<<"Insert Successfully"<<endl;
}

int Session::handleAddFriendReq(void *msg)
{
    AddFriendInfoReq* pAddFriendInfoReq = (AddFriendInfoReq*)msg;
    printf("get a add friend request..\n");
    mapGroupInfo::iterator iter = g_GroupUserInfoMap.find(0);
    if(iter!=g_GroupUserInfoMap.end()){
        mapGroupUserInfo::iterator ite = iter->second.find(pAddFriendInfoReq->m_senderAccount);
        if(ite!=iter->second.end()){
            printf("account:%d,m_reqInfo:%s\n", pAddFriendInfoReq->m_senderAccount, pAddFriendInfoReq->m_reqInfo);
            GroupUserInfo *pUserInfo = ite->second;
            //将等待添加的好友添加到 map 表中
            FriendInfo *pFriendInfo = (FriendInfo *)malloc(sizeof(FriendInfo));
            pFriendInfo->m_account = pUserInfo->m_account;
            pFriendInfo->m_status = 1; //Linux命令行客户端直接同意
            strncpy(pFriendInfo->m_userName, pUserInfo->m_userName, sizeof(pUserInfo->m_userName));
            m_friendInfoMap.insert(std::make_pair(pAddFriendInfoReq->m_senderAccount, pFriendInfo));
            //给等待添加的好友发送添加好友的响应
            AddFriendInfoResp resp;
            resp.m_friendAccount = pAddFriendInfoReq->m_friendAccount;
            resp.m_senderAccount = pAddFriendInfoReq->m_senderAccount;
            resp.status = 0;   //同意
            sendMsg(m_socket, &resp, sizeof(AddFriendInfoResp), CommandEnum_AddFriend,0,2);
        }
        else{
            printf("该用户不存在\n");
        }
    }
}

int Session::handleAddFriendResp(void *msg)
{
    //Linux命令行客户端暂时不做添加好友请求
}