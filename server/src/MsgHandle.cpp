#include "MsgHandle.h"
#include "common.h"

std::shared_ptr<SessionMng> SessionMng::m_pSessionMng = nullptr; 

int SessionMng::addSession(SessionPtr newSession,int fd){
    std::lock_guard<std::mutex> guard(m_gSessionMapLock);
    // LOGINFO("insert fd:%d\n",fd);
    std::pair<SessionMapIter, bool> bRet = m_gSessionMap.insert(std::make_pair(fd, newSession));
    if (!bRet.second)
    {
        LOGINFO("insert map err\n");
        return -1;
    }
    return 0;
}
int SessionMng::delSession(int fd){
    std::lock_guard<std::mutex> guard(m_gSessionMapLock);
    auto iter = m_gSessionMap.find(fd);
    if (iter != m_gSessionMap.end())
    {
        SessionPtr sp = iter->second;
        if (sp != nullptr)
        {
            // 可能需要清理资源
            // sp->closeStream();
        }
        m_gSessionMap.erase(iter);
    }
    return 0;
}
int SessionMng::handleSession(int fd){
    std::lock_guard<std::mutex> guard(m_gSessionMapLock);
    auto iter = m_gSessionMap.find(fd);
    if (iter != m_gSessionMap.end())
    {
        SessionPtr sp = iter->second;
        if (sp != nullptr)
        {
            if (sp->readEvent() == RET_EXIT)
            {
                m_gSessionMap.erase(iter);
                return -1;
            }
        }
    }
    return 0;
}

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
    if(m_isLogin == 1){
        cleanSession();
        m_isLogin = -1;
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
    printf("ret:%d\n",ret);
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
        //蓝星头
        DeMessageHead *tmp = (DeMessageHead*)m_head;
        if(strncmp(tmp->mark,"DE",2)!=0){
            LOGINFO("mark:%s ,length:%d\n",tmp->mark,tmp->length);
            m_bufLen = sizeof(DeMessageHead);
            m_readPos = 0;
            m_type = RECV_HEAD;
            return RET_AGAIN;
        }
        m_type = RECV_BODY;
        int bufLen = ((DeMessageHead *)m_head)->length;
        // if(bufLen)
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
        // 读取完成后不需要继续读
        // return RET_AGAIN;
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
    // LOGINFO("mark:%s,encoded:%d,length:%d,version:%d\n", head->mark, head->encoded, head->length, head->version);
    // LOGINFO("command:%d,error:%d,,mode:%d,sequence:%d\n", packet->command, packet->error, packet->mode, packet->sequence);
    void *reqData = NULL;
    int reqDataLen = -1;
    short ret = RET_OK;
    LOGINFO("handleMsg command:%d\n",packet->command);
    switch (packet->command)
    {
        case CommandEnum_Registe:
            handleRegiste(this,rMsg->body + sizeof(DeMessagePacket));
            break;
        case CommandEnum_Login:
            handleLogin(this, rMsg->body + sizeof(DeMessagePacket));
            break;
        case CommandEnum_Logout:
            handleLogout(this, rMsg->body + sizeof(DeMessagePacket));
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
            handleGetGroupList(rMsg->body + sizeof(DeMessagePacket));
            break;
        case CommandEnum_GetGroupInfo:
            handleGetGroupInfo(rMsg->body + sizeof(DeMessagePacket));
            break;
        case CommandEnum_GetFriendInfo:
            handleGetFriendInfo(rMsg->body + sizeof(DeMessagePacket));   
            break;
    }
    delete rMsg;
    return 0;
}

/**
 * @brief 
 * 
 * @param socket 
 * @param buf 
 * @param bufLen 
 * @param type 
 * @param error 0为正常 其他为错误码
 * @param mode 消息模式 请求/响应   1/2
 */
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
    mapUserInfo::iterator iter = g_UserInfoMap.find(m_account);
    if (iter == g_UserInfoMap.end())
    {
        LOGINFO("出错了..\n");
    }
    else
    {
        UserInfo* delUserInfo = iter->second;
        g_UserInfoMap.erase(iter);
        if(delUserInfo!=NULL){
            free(delUserInfo);
            delUserInfo =NULL;
        }
        LOGINFO("会话关闭成功..\n在线用户数量:%lu\n",g_UserInfoMap.size());
    }
}

void Session::noticeUserLogin(UserInfo*userInfo)
{
    GroupUserInfo* groupUserInfo = (GroupUserInfo*)malloc(sizeof(GroupUserInfo));
    groupUserInfo->m_account = userInfo->m_account;
    groupUserInfo->m_right = 0;
    strncpy(groupUserInfo->m_userName,userInfo->m_userName,sizeof(userInfo->m_userName));
    LOGINFO("=======//%s\n",groupUserInfo->m_userName);
    mapUserInfo::iterator iter = g_UserInfoMap.begin();
    for (; iter != g_UserInfoMap.end(); iter++)
    {
        if(iter->first == m_account)
        {
            LOGINFO("不给自己发..\n");
            continue;
        }
        sendMsg(iter->second->m_socket, groupUserInfo, sizeof(GroupUserInfo), CommandEnum_Login,0,3);   //发送用户上线通知
    }
}

/**
 * @brief 未添加存储功能时，注册信息申请的RegistInfoReq*一直存在
 */
int Session::handleRegiste(void*arg,void* msg){
    Session* pthis = (Session*)arg;
    RegistInfoReq* registInfo = (RegistInfoReq*)malloc(sizeof(RegistInfoReq));
    memmove(registInfo,msg,sizeof(RegistInfoReq));
    LOGINFO("username:%s,password:%s\n",registInfo->m_userName,registInfo->m_password);
    RegistInfoResp registInfoResp;
    registInfoResp.m_account = g_idIndex++;

    g_AccountInfoMap.insert(std::make_pair(registInfoResp.m_account,registInfo));
    LOGINFO("account:%d,注册成功..\n",registInfoResp.m_account);
    sendMsg(pthis->m_socket,&registInfoResp,sizeof(RegistInfoResp),CommandEnum_Registe);
    return 0;
}

int Session::handleLogin(void*arg,void*msg){
    Session* pthis = (Session*)arg;
    LoginInfoReq* loginInfo = (LoginInfoReq*)msg;
    LOGINFO("account:%d,password:%s\n",loginInfo->m_account,loginInfo->m_password);

    mapAccountInfo::iterator iter = g_AccountInfoMap.find(loginInfo->m_account);
    if (iter == g_AccountInfoMap.end())
    {
        sendMsg(pthis->m_socket,NULL,0,CommandEnum_Login,RET_ERROR);
        LOGINFO("请先注册..\n");
    }
    else
    {
        mapUserInfo::iterator ite = g_UserInfoMap.find(loginInfo->m_account);
        if(ite!=g_UserInfoMap.end()){
            sendMsg(pthis->m_socket,NULL,0,CommandEnum_Login,RET_ERROR);
            LOGINFO("已登录，重复登录\n");
            return -1;
        }
        //匹配账号密码
        RegistInfoReq* pRegistInfoReq = iter->second;
        if (strcmp(loginInfo->m_password, iter->second->m_password) == 0)
        {
            // 给当前会话保存账号信息
            pthis->m_account = loginInfo->m_account;
            pthis->m_isLogin = 1;
            strncpy(pthis->m_userName,pRegistInfoReq->m_userName,sizeof(pRegistInfoReq->m_userName));
            // 添加用户信息
            UserInfo* userInfo = (UserInfo*)malloc(sizeof(UserInfo)); 
            userInfo->m_account = loginInfo->m_account;
            userInfo->m_socket = pthis->m_socket;
            strncpy(userInfo->m_password,loginInfo->m_password,sizeof(loginInfo->m_password));
            strncpy(userInfo->m_userName,pRegistInfoReq->m_userName,sizeof(pRegistInfoReq->m_userName));
            
            g_UserInfoMap.insert(std::make_pair(userInfo->m_account,userInfo));
            // 给用户添加一个公共群聊 （后续程序完善后会删除公共群聊） 
            mapGroupChatInfo::iterator iter = g_GroupCharInfoMap.find(0);
            if( iter!= g_GroupCharInfoMap.end()){
                GroupChatInfo* info = iter->second;
                info->m_size++;
                LOGINFO("current group size:%lu\n",g_GroupCharInfoMap.size());
            }
            else{
                //第一次添加往群信息表中插入群信息
                GroupChatInfo *groupChatinfo = (GroupChatInfo *)malloc(sizeof(GroupChatInfo));
                groupChatinfo->m_account = 0;
                groupChatinfo->m_size = 1;  //服务启动后新用户登录时初始化公共聊天群 公共聊天群成员大小为1
                strncpy(groupChatinfo->m_groupName, "C/C++学习交流群", strlen("C/C++学习交流群"));
                std::pair<mapGroupChatInfo::iterator, bool> Insert_Pair = g_GroupCharInfoMap.insert(std::make_pair(groupChatinfo->m_account, groupChatinfo));
                if (Insert_Pair.second == true)
                {
                    ChatLog << "Insert Successfully:" << g_GroupCharInfoMap.size() << endl;
                }
                else
                {
                    ChatLog << "Insert fail" << endl;
                }
            }
            //发送响应,告诉客户端你登录成功了
            sendMsg(pthis->m_socket,NULL, 0, CommandEnum_Login, RET_OK);
            LOGINFO("登录成功..\n在线用户数量:%lu\n",g_UserInfoMap.size());
            pthis->noticeUserLogin(userInfo);
        }
        else
        {
            sendMsg(pthis->m_socket,NULL,0,CommandEnum_Login,RET_ERROR);
            LOGINFO("密码错误..  password:%s\n",iter->second->m_password);
        }
    }
    return 0;
}

int Session::handleLogout(void*arg,void*msg){
    Session* pthis = (Session*)arg;
    if (pthis->m_isLogin == 1){
        pthis->cleanSession();
        pthis->m_isLogin = -1;
    }
    return 0;
}

int Session::handleGroupChat(void *arg, void *msg){
    Session* pthis = (Session*)arg;
    GroupChatReq* groupChatReq = (GroupChatReq*)msg;

    if(!pthis->m_isLogin)
        return -1;

    LOGINFO("account:%d,msgLen:%d,group:%d,type:%d\n",groupChatReq->m_UserAccount,groupChatReq->m_msgLen,groupChatReq->m_GroupAccount,groupChatReq->m_type);
    char*buf = (char*)malloc(groupChatReq->m_msgLen);
    memmove(buf,(char*)msg + sizeof(GroupChatReq),groupChatReq->m_msgLen);
    LOGINFO("recv msg:%s\n",buf);
    if(groupChatReq->m_GroupAccount == 0){
        //广播
        mapUserInfo::iterator iter = g_UserInfoMap.begin();
        for(;iter!=g_UserInfoMap.end();iter++){
            if(iter->first == pthis->m_account)
            {
                LOGINFO("不给自己发..\n");
                continue;
            }
            sendMsg(iter->second->m_socket,msg,sizeof(GroupChatReq) + groupChatReq->m_msgLen, CommandEnum_GroupChat);
        }
    }
    return 0;
}

int Session::handlePrivateChat(void*msg){
    PrivateChatReq* privateChatReq = (PrivateChatReq*)msg;

    if(!m_isLogin)
        return -1;
    LOGINFO("account:%d,msgLen:%d,friendAccount:%d,type:%d\n",privateChatReq->m_UserAccount,privateChatReq->m_msgLen,privateChatReq->m_FriendAccount,privateChatReq->m_type);
    char*buf = (char*)malloc(privateChatReq->m_msgLen);
    memmove(buf,(char*)msg + sizeof(PrivateChatReq),privateChatReq->m_msgLen);
    LOGINFO("recv msg:%s\n",buf);

    mapUserFriendInfo::iterator iter = g_UserFriendInfoMap.find(m_account);
    if (iter != g_UserFriendInfoMap.end())
    {
        //用户存在好友表
        mapFriendInfo::iterator ite = iter->second.find(privateChatReq->m_FriendAccount);
        if(ite!=iter->second.end()){
            //找到好友
            FriendInfo* friendInfo = ite->second;
            //给好友发消息
            mapUserInfo::iterator it = g_UserInfoMap.find(friendInfo->m_account);
            if(it!=g_UserInfoMap.end()){
                sendMsg(it->second->m_socket,msg,sizeof(PrivateChatReq) + privateChatReq->m_msgLen, CommandEnum_PrivateChat);
            }
            else{
                LOGINFO("用户不在线，请稍后发送..\n");
            }
        }
        else{
            LOGINFO("没有找到好友\n");
        }
    }
    else
    {
        LOGINFO("不存在好友表\n");
    }

    if(buf!=NULL)
        free(buf);
    return 0;
}
/**
 * @brief 获取群列表，这里需要修改，具体获取那个人的群聊列表
 */
int Session::handleGetGroupList(void*msg){
    //申请一块内存用于保存 群列表大小 以及 群信息
    int size = sizeof(GetGroupListResp) + sizeof(GroupChatInfo)*g_GroupCharInfoMap.size();
    char*p = (char*)malloc(size);
    GetGroupListResp* pResp = (GetGroupListResp*)p;
    pResp->m_size = g_GroupCharInfoMap.size();
    mapGroupChatInfo::iterator iter = g_GroupCharInfoMap.begin();
    for (int index = 0; iter != g_GroupCharInfoMap.end(); iter++, index++){
        //获取当前群聊信息
        GroupChatInfo *curGroupChatInfo = iter->second;
        //申请新的群聊信息
        GroupChatInfo *pGroupChatInfo = (GroupChatInfo *)(p + sizeof(GetGroupListResp) + sizeof(GroupChatInfo) * index);
        pGroupChatInfo->m_account = curGroupChatInfo->m_account;
        pGroupChatInfo->m_size = curGroupChatInfo->m_size;
        memmove(pGroupChatInfo->m_groupName, curGroupChatInfo->m_groupName, sizeof(curGroupChatInfo->m_groupName));
    }
    sendMsg(m_socket, p, size, CommandEnum_GetGroupList);
    return 0;
}

/**
 * @brief 获取群信息
 */
int Session::handleGetGroupInfo(void *msg)
{
    GetGroupInfoReq* pReq = (GetGroupInfoReq*)msg;
    if(pReq->m_GroupAccount == 0){
        // 全部用户 申请一块内存用于保存 群信息 以及 群成员信息
        int size = sizeof(GetGroupInfoResp) + sizeof(GroupUserInfo)*g_UserInfoMap.size();
        char*p = (char*)malloc(size);
        GetGroupInfoResp* pResp = (GetGroupInfoResp*)p;
        pResp->m_GroupAccount = pReq->m_GroupAccount;
        pResp->m_size = g_UserInfoMap.size();
        strncpy(pResp->m_groupName,"C/C++学习交流群",strlen("C/C++学习交流群"));
        //遍历保存所有群用户信息（仅在线用户在公共聊天群里面）
        mapUserInfo::iterator iter = g_UserInfoMap.begin();
        for (int index = 0; iter != g_UserInfoMap.end(); iter++, index++){
            UserInfo* curUserInfo = iter->second;
            GroupUserInfo *pUserInfo = (GroupUserInfo *)(p + sizeof(GetGroupInfoResp) + sizeof(GroupUserInfo) * index);
            pUserInfo->m_account = curUserInfo->m_account;
            pUserInfo->m_right = 0;
            strncpy(pUserInfo->m_userName,curUserInfo->m_userName,sizeof(curUserInfo->m_userName));
        }
        sendMsg(m_socket, p, size, CommandEnum_GetGroupInfo);
        free(p);
        p = NULL;
    }
    return 0;
}

int Session::handleGetFriendInfo(void* msg){
    // 全部用户 申请一块内存用于保存 群信息 以及 群成员信息
    //1.从用户好友表找到好友表
    mapUserFriendInfo::iterator iter = g_UserFriendInfoMap.find(m_account);
    if(iter!=g_UserFriendInfoMap.end()){

        int size = sizeof(GetFriendInfoResp) + sizeof(FriendInfo) * iter->second.size();
        char *p = (char *)malloc(size);
        GetFriendInfoResp *pResp = (GetFriendInfoResp *)p;
        pResp->m_size = iter->second.size();
        //用户存在好友表
        mapFriendInfo::iterator ite = iter->second.begin();
        for(int index = 0;ite!=iter->second.end();ite++,index++){
            FriendInfo* pFriendInfo = ite->second;
            FriendInfo* pFriendInfoResp = (FriendInfo *)(p + sizeof(GetFriendInfoResp) + sizeof(FriendInfo) * index);
            strncpy(pFriendInfoResp->m_userName, pFriendInfo->m_userName, sizeof(pFriendInfo->m_userName));
            pFriendInfoResp->m_account = pFriendInfo->m_account;
            pFriendInfoResp->m_status = pFriendInfo->m_status;
        }
        sendMsg(m_socket, p, size, CommandEnum_GetFriendInfo);
    }
    else{
        //没有好友信息
        int size = sizeof(GetFriendInfoResp) + sizeof(FriendInfo) * 0;
        char *p = (char *)malloc(size);
        GetFriendInfoResp *pResp = (GetFriendInfoResp *)p;
        pResp->m_size = 0;
        LOGINFO("account:%d not has friends\n",m_account);
        sendMsg(m_socket, p, size, CommandEnum_GetFriendInfo);
    }
    return 0;
}

/**
 * @brief A:发送好友请求的客户端
 *        B:接收好友请求的客户端
 */
int Session::handleAddFriendReq(void*msg){
    AddFriendInfoReq* pReq = (AddFriendInfoReq*)msg;
    LOGINFO("recv addFriendReq:{\n\tfriendAccount:%d;\n\tsenderAccount:%d;\n\tmsg:%s;\n}\n",pReq->m_friendAccount,pReq->m_senderAccount,pReq->m_reqInfo);
    //A.找到 B 的用户信息
    UserInfo *pUserInfo = NULL;
    mapUserInfo::iterator UserInfoiter = g_UserInfoMap.find(pReq->m_friendAccount);
    if (UserInfoiter != g_UserInfoMap.end())
    {
        pUserInfo = UserInfoiter->second;
    }
    else{
        //没找到B用户信息，添加好友请求失败
        //给客户端发送请求失败的响应信息
        LOGINFO("not find account:%d\n",pReq->m_friendAccount);
        sendMsg(m_socket, NULL, 0, CommandEnum_AddFriend, -1);
        return -1;
    }

    //C 1.从用户好友表找到 A 对应的好友表
    mapUserFriendInfo::iterator iter = g_UserFriendInfoMap.find(m_account);
    if(iter!=g_UserFriendInfoMap.end()){
        //A用户存在好友表
        //2.用户好友表中查找B的账号是否在表中
        mapFriendInfo::iterator ite = iter->second.find(pReq->m_friendAccount);
        if(ite!=iter->second.end()){
            //找到了
            FriendInfo* pFriendInfo = ite->second;
            if(pFriendInfo->m_status == 1){
                //已经是好友了，重复添加 不应该出现的情况
                sendMsg(m_socket, NULL, 0, CommandEnum_AddFriend, -1);
                return -1;
            }
            else if(pFriendInfo->m_status == -1){
                //B还未同意，暂时不给B发请求了
                //给客户端发送请求成功的信息,防止重复添加
                sendMsg(m_socket, NULL, 0, CommandEnum_AddFriend, 0);
            }
        }
        else{
            //没有好友信息，需要添加好友信息
            //将B添加到A的好友表中
            FriendInfo *pFriendInfo = (FriendInfo *)malloc(sizeof(FriendInfo));
            pFriendInfo->m_account = pUserInfo->m_account;
            pFriendInfo->m_status = 0; //未同意的好友，当拒绝时删除这条信息
            strncpy(pFriendInfo->m_userName, pUserInfo->m_userName, sizeof(pUserInfo->m_userName));

            std::pair<mapFriendInfo::iterator, bool> Insert_Pair = iter->second.insert(std::make_pair(pReq->m_friendAccount, pFriendInfo));
            if (Insert_Pair.second == true)
            {
                ChatLog << "A friend map Insert Successfully:" << iter->second.size() << endl;
            }
            else
            {
                ChatLog << "A friend map Insert fail" << endl;
            }
            //给其 B 用户 转发 A 添加好友的请求
            sendMsg(pUserInfo->m_socket, msg, sizeof(AddFriendInfoReq), CommandEnum_AddFriend, 0, 1);
            //给客户端发送请求成功的信息,告诉客户端请求被受理了
            sendMsg(m_socket, NULL, 0, CommandEnum_AddFriend, 0);
        }
    }
    else{
        //A用户不存在好友表
        mapFriendInfo curFriendInfoMap;
        //将B添加到A的好友表中
        FriendInfo *pFriendInfo = (FriendInfo *)malloc(sizeof(FriendInfo));
        pFriendInfo->m_account = pReq->m_friendAccount;
        pFriendInfo->m_status = 0; //未同意的好友，当拒绝时删除这条信息
        strncpy(pFriendInfo->m_userName, pUserInfo->m_userName, sizeof(pUserInfo->m_userName));
        std::pair<mapFriendInfo::iterator, bool> Insert_Pair = curFriendInfoMap.insert(std::make_pair(pReq->m_friendAccount, pFriendInfo));
        if (Insert_Pair.second == true)
        {
            ChatLog << "A friend map Insert Successfully,user:"<<m_account<<" friend size:" << iter->second.size() <<",friend account:"<<pReq->m_friendAccount<< endl;
        }
        else
        {
            ChatLog << "A friend map Insert fail" << endl;
        }
        
        g_UserFriendInfoMap.insert(std::make_pair(m_account, curFriendInfoMap));
        //给其 B 用户 转发 A 添加好友的请求
        sendMsg(pUserInfo->m_socket, msg, sizeof(AddFriendInfoReq), CommandEnum_AddFriend,0,1);
        //给客户端发送请求成功的信息,告诉客户端请求被受理了
        sendMsg(m_socket, NULL, 0, CommandEnum_AddFriend, 0);
    }
    return 0;
}

/**
 * @brief A:发送好友请求的客户端
 *        B:接收好友请求的客户端
 */
int Session::handleAddFriendResp(void*msg){
    AddFriendInfoResp* pResp = (AddFriendInfoResp*)msg;
    LOGINFO("recv a AddFriendResp msg,status:%d\n",pResp->status);
    //A.找到 A 的用户信息
    UserInfo *pUserInfo = NULL;
    mapUserInfo::iterator UserInfoiter = g_UserInfoMap.find(pResp->m_senderAccount);
    if (UserInfoiter != g_UserInfoMap.end())
    {
        pUserInfo = UserInfoiter->second;
    }
    else{
        //没找到A用户信息，添加好友请求失败
        //给客户端发送响应失败的响应信息(不应该出现的情况)
        sendMsg(m_socket, NULL, 0, CommandEnum_AddFriend, -1);
        return -1;
    }


    //C 1.从用户好友表找到 B 对应的好友表
    mapUserFriendInfo::iterator iter = g_UserFriendInfoMap.find(m_account);
    if(iter!=g_UserFriendInfoMap.end()){
        //B用户存在好友表
        //2.用户好友表中查找A的账号是否在表中
        mapFriendInfo::iterator ite = iter->second.find(pResp->m_senderAccount);
        if(ite!=iter->second.end()){
            LOGINFO("-----------error-----------\n");
            //找到了,这不应该出现的情况
            FriendInfo* pFriendInfo = ite->second;
            if(pFriendInfo->m_status == 1){
                //已经是好友了，重复添加 不应该出现的情况
                sendMsg(m_socket, NULL, 0, CommandEnum_AddFriend, -1);
                return -1;
            }
            else if(pFriendInfo->m_status == -1){
                //B还未同意，暂时不给B发请求了
                //给客户端发送请求成功的信息
                sendMsg(m_socket, NULL, 0, CommandEnum_AddFriend, 0);
            }
        }
        else{
            //没有好友信息，需要添加好友信息
            //将B添加到A的好友表中
            FriendInfo *pFriendInfo = (FriendInfo *)malloc(sizeof(FriendInfo));
            pFriendInfo->m_account = pResp->m_senderAccount;
            pFriendInfo->m_status = 1; //未同意的好友，当拒绝时删除这条信息
            strncpy(pFriendInfo->m_userName, pUserInfo->m_userName, sizeof(pUserInfo->m_userName));
            LOGINFO("[%d][%s]userName：%s,account:%d,status:%d\n",m_account,m_userName,pFriendInfo->m_userName,pFriendInfo->m_account,pFriendInfo->m_status);

            std::pair<mapFriendInfo::iterator, bool> Insert_Pair = iter->second.insert(std::make_pair(pResp->m_senderAccount, pFriendInfo));
            if (Insert_Pair.second == true)
            {
                ChatLog << "B friend map Insert Successfully,user:" << m_account << " friend size:" << iter->second.size() << ",friend account:" << pResp->m_senderAccount << endl;
            }
            else
            {
                ChatLog << "B friend map Insert fail" << endl;
            }

            //给其 A 用户 转发 B 添加好友的成功的响应
            sendMsg(pUserInfo->m_socket, pResp, sizeof(AddFriendInfoResp), CommandEnum_AddFriend);
            //给客户端发送请求成功的信息,告诉客户端请求被受理了
            sendMsg(m_socket, NULL, 0, CommandEnum_AddFriend, 0);
        }
    }
    else{
        //B用户不存在好友表
        mapFriendInfo curFriendInfoMap;
        //将A添加到B的好友表中
        FriendInfo *pFriendInfo = (FriendInfo *)malloc(sizeof(FriendInfo));
        pFriendInfo->m_account = pResp->m_senderAccount;
        pFriendInfo->m_status = 1; //未同意的好友，当拒绝时删除这条信息
        strncpy(pFriendInfo->m_userName, pUserInfo->m_userName, sizeof(pUserInfo->m_userName));
        LOGINFO("[%d][%s]userName：%s,account:%d,status:%d\n",m_account,m_userName,pFriendInfo->m_userName,pFriendInfo->m_account,pFriendInfo->m_status);

        std::pair<mapFriendInfo::iterator, bool> Insert_Pair = curFriendInfoMap.insert(std::make_pair(pResp->m_senderAccount, pFriendInfo));
        if (Insert_Pair.second == true)
        {
            ChatLog << "B friend map Insert Successfully,user:" << m_account << " friend size:" << iter->second.size() << ",friend account:" << pResp->m_senderAccount << endl;
        }
        else
        {
            ChatLog << "B friend map Insert fail" << endl;
        }

        g_UserFriendInfoMap.insert(std::make_pair(m_account, curFriendInfoMap));
        //给其 A 用户 转发 B 添加好友的成功的响应
        sendMsg(pUserInfo->m_socket, pResp, sizeof(AddFriendInfoResp), CommandEnum_AddFriend);
        //给客户端发送请求成功的信息,告诉客户端请求被受理了
        sendMsg(m_socket, NULL, 0, CommandEnum_AddFriend, 0);
    }

    //将A用户的好友标志置位
    //C 1.从用户好友表找到 A 对应的好友表
    mapUserFriendInfo::iterator iter2 = g_UserFriendInfoMap.find(pResp->m_senderAccount);
    if(iter2!=g_UserFriendInfoMap.end()){
        //A用户存在好友表
        //2.用户好友表中查找B的账号是否在表中
        mapFriendInfo::iterator ite2 = iter2->second.find(pResp->m_friendAccount);
        if(ite2!=iter2->second.end()){
            //找到了
            FriendInfo* pFriendInfo = ite2->second;
            pFriendInfo->m_status = 1;  //同意
            LOGINFO("[%d]userName：%s,account:%d,status:%d\n",pResp->m_senderAccount,pFriendInfo->m_userName,pFriendInfo->m_account,pFriendInfo->m_status);
        }
        else{
            LOGINFO("==========出错了==========\n");
        }
    }
    else{
        LOGINFO("==========出错了==========\n");
    }
    return 0;
}


