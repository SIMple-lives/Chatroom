#include "../head/head.hpp"
#include "../ser/User.hpp"
#include "../redis/redis.hpp"
#include "../head/define.hpp"
#include <fcntl.h>
#include <filesystem>
#include <hiredis/hiredis.h>
#include <string>
#include <unordered_map>
#include <vector>
#include "../ser/message.hpp"


void S_User::S_Login(int fd,std::string str)
{
    nlohmann::json json = nlohmann::json::parse(str);
    std::string id = json["id"] ;
    std::string password = json["password"];
    std::cout << "用户\033[41;32m " << id << " \033[0m正在登录 : " << std::endl;
    std::cout << "密码 : " << password << std::endl;
    redisAsyncContext redis;
    // std::string redis_str = redis.HashGet("userinfo",id);
    // nlohmann::json userinfo = nlohmann::json::parse(redis_str);
    if(redis.HashExit("userinfo",id)!=1)
    {
        std::cout << "该id账号不存在 " << std::endl;
        Sen s;
        s.send_ok(fd, NEXIST);
        std::cout << NEXIST << std::endl;
        return ;
    }
    std::string redis_str = redis.HashGet("userinfo",id);
    nlohmann::json userinfo = nlohmann::json::parse(redis_str);
    if(userinfo["status"]==ONLINE)
    {
        std::cout << " 该账号已经登陆 : " << std::endl;
        Sen s;
        s.send_ok(fd, ALREADYLOGIN);
    }
    else 
    {
        if(password!=userinfo["password"])
        {
            std::cout << "密码错误 " << std::endl;
            Sen s;
            s.send_ok(fd, PASSWORDERROR);
            std::cout << "\033[31m" << PASSWORDERROR << std::endl;
        }
        else 
        {
            std::cout << "登录成功 " << std::endl;
            Sen s;
            s.send_ok(fd, SUCCESS);
            //修该状态
            userinfo["status"]=ONLINE;
            redis.HashSet("userinfo",id,userinfo.dump());
            s.send_cil(fd,redis_str);
            //修改在线列表
            redis.Insert("online",id);
            //修改在线列表
        } 
    }
}

std::string m_rand() 
{
    std::string str;
    std::random_device rd;  // 使用硬件产生真正随机数的种子 会使用硬件的随机数生成器来产生真正的随机数
    std::mt19937 gen(rd()); // 使用 Mersenne Twister 算法作为随机数引擎  可以产生随即平均分布良好且周期很长的随机数
    std::uniform_int_distribution<int> dist(0, 9); // 定义均匀分布在 [0, 9] 范围内的随机数分布器
    //是标准库中的随机数分布器模板，用于生成随机的整数随机数
    for (int i = 0; i < 10; i++) 
    {
        int random_digit = dist(gen);  // 生成随机数字
        if(random_digit == 0 && i==0)
        {
            random_digit = 1;
        }
        char digit_char = '0' + random_digit;  // 将随机数字转换为对应的字符
        str += digit_char;  // 将字符追加到字符串末尾
    }
    return str;
}

void S_User::S_Signup(int fd,std::string str)
{
    nlohmann::json json = nlohmann::json::parse(str);
    S_User S_user;
    std::string username = json["username"];
    std::cout << "用户\033[41;32m " << username << " \033[0m正在注册 : " << std::endl;
    std::string password = json["password"];
    std::string questions = json["questions"];
    std::string que_ans = json["que_ans"];
    std::cout << "密码 : " << password << std::endl;
    std::cout << "问题 : " << questions << std::endl;
    std::cout << "答案 : " << que_ans << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;
    redisAsyncContext redis;
    if(redis.Ifexit("username",username))//redis的成员函数，看是否有该用户名，有的话注册失败
    {
        std::cout << "该用户名已经存在: " << std::endl;
        Sen s;
        s.send_ok(fd,EXIST);
    }
    else 
    {
        std::string id = m_rand();
        int n = redis.HashSet("userinfo",id,str);
        int m = redis.Insert("username",username);
        int o = redis.HashSet("id_name",id,username);
        int p = redis.HashSet("name_id",username,id);
        if(n == REDIS_REPLY_ERROR || m == REDIS_REPLY_ERROR || o == REDIS_REPLY_ERROR || p == REDIS_REPLY_ERROR)
        {
            std::cout << "注册失败" << std::endl;
            Sen s;
            s.send_ok(fd, FAILURE);
            return ;
        }
        else 
        {
            std::cout << "注册成功 " << std::endl;
            Sen s;
            s.send_ok(fd, SUCCESS);
            s.send_cil(fd, id); 
        }
    }
    return ;
}


void S_User::S_Signout(int fd,std::string str)
{
    nlohmann::json json = nlohmann::json::parse(str);
    std::string id = json["id"];
    std::string password = json["password"];
    std::cout << "用户\033[41;32m " << id << " \033[0m注销账户 : " << std::endl;
    std::cout << "密码 : " << password << std::endl;
    redisAsyncContext redis;
    
    if(redis.HashExit("userinfo",id)!=1)
    {
        std::cout << "该id账号不存在 " << std::endl;
        Sen s;
        s.send_ok(fd, NEXIST);
        return ;
    }
    std::string userinfo = redis.HashGet("userinfo",id);
    nlohmann::json getpassword = nlohmann::json::parse(userinfo);
    if(getpassword["status"]==ONLINE)
    {
        std::cout << "该账号已经登陆 : " << std::endl;
        Sen s;
        s.send_ok(fd, ALREADYLOGIN);
        return ;
    }
    else 
    {
        
        if(password!=getpassword["password"])
        {
            std::cout << "密码错误 " << std::endl;
            Sen s;
            s.send_ok(fd, PASSWORDERROR);
        }
        else 
        {
            // 注销操作：删除 "userinfo" 哈希表中指定 id 的数据
            int deleteUserInfo = redis.HashDele("userinfo", id);

            // 注销操作：从 "username" 集合中删除指定的用户名
            int deleteUserFromUsernameSet = redis.Delevalue("username", getpassword["username"]);

            // 注销操作：删除 "id_name" 哈希表中指定 id 的数据
            // int deleteIdName = redis.HashDele("id_name", id);

            // // 注销操作：删除 "name_id" 哈希表中指定用户名的数据
            // int deleteNameId = redis.HashDele("name_id", getpassword["username"]);
            //获取到好友列表，删除好友列表中的自己
            std::vector<std::string> friends = redis.Zrange(id,0,-1);
            for(auto I : friends)
            {
                redis.Zrem(I,id);
            }
            redis.Zclear(id);
            redis.Zclear(id+"chat");
            redis.Zclear(id+"Gchat");
            redis.Ltrim(id+"offline", 1 , 0 );

            std::cout << "注销成功 " << std::endl;
            Sen s;
            s.send_ok(fd, SUCCESS);
        }
    }
}

void S_User::S_FindA(int fd,std::string str)
{
    nlohmann::json json = nlohmann::json::parse(str);
    std::string id = json["id"];

    std::cout << "用户\033[41;32m " << id << " \033[0m : 正在找回密码 " << std::endl;
    redisAsyncContext redis;
    std::string userinfo = redis.HashGet("userinfo",id);
    nlohmann::json getquestions = nlohmann::json::parse(userinfo);
    std::cout << "用户名 : " << getquestions["questions"] << std::endl;
    Sen s;
    s.send_ok(fd, SUCCESS);
    s.send_cil(fd, getquestions["questions"]);
}
void S_User::S_FindB(int fd,std::string str)
{
    nlohmann::json json = nlohmann::json::parse(str);
    std::string id = json["id"];
    std::string que_ans = json["que_ans"];
    std::cout << "用户\033[41;32m " << id << " \033[0m验证密保答案 : " << std::endl;
    redisAsyncContext redis;
    std::string userinfo = redis.HashGet("userinfo",id);
    
    nlohmann::json getque_ans = nlohmann::json::parse(userinfo);
    std::string password = getque_ans["password"];
    if(que_ans!=getque_ans["que_ans"])
    {
        std::cout << "答案错误 " << std::endl;
        Sen s;
        s.send_ok(fd, PASSWORDERROR);
    }
    else 
    {
        std::cout << "答案正确 " << std::endl;
        Sen s;
        s.send_ok(fd, SUCCESS);
        s.send_cil(fd, password);
    }
}

void S_User::S_EXIT(int fd,std::string str)
{
    std::cout << "\033[41;32m 客户端" << fd << " \033[0m退出 " << std::endl;
    //close(fd);
}

void S_User::S_QUIT(int fd,std::string str)
{
    std::cout << str << std::endl;
    nlohmann::json json  = nlohmann::json::parse(str);
    std::string id = json["id"];
    std::cout << "用户\033[41;32m " << id << " \033[0m : 正在退出 " << std::endl;
    redisAsyncContext redis;
    std::string userinfo = redis.HashGet("userinfo",id);
    nlohmann::json getpassword = nlohmann::json::parse(userinfo);
    getpassword["status"]=OFFLINE;
    redis.HashSet("userinfo",id,getpassword.dump());
    Sen s;
    s.send_ok(fd, SUCCESS);
    std::cout << "用户\033[41;32m " << id << " \033[0m : 退出成功 " << std::endl;
    redis.Delevalue("online", id);
}

void S_User::S_AddFriend(int fd,std::string str)
{
    std::cout << str << std::endl;
    nlohmann::json js = nlohmann::json::parse(str);
    std::string friend_id = js["friend_id"];
    std::string id = js["id"];
    std::cout << id << "正在向  --->   " << friend_id << "  发送好友请求" << std::endl;
    redisAsyncContext redis;
    if(redis.HashExit("userinfo",friend_id)!=1)
    {
        std::cout << "用户不存在" << std::endl;
        Sen s;
        s.send_ok(fd, NEXIST);
    }
    else if(id==friend_id)
    {
        std::cout << "不能添加自己" << std::endl;
        Sen s;
        s.send_ok(fd, ISME);
        return;
    }
    else 
    {
        int status ;
        if(redis.Zstatus(id,friend_id,status))
        {
            std::cout << "已经是好友" << std::endl;
            Sen s;
            s.send_ok(fd, ISFRIEND);
        }
        else 
        {
            Sen s;
            nlohmann::json json = {
                {"msg",js["msg"]},
                {"status",FRIENDREQUEST},
                {"time",js["time"]}
                };
                std::string json_str = json.dump();
            bool ret=redis.HashSet(friend_id+"0", id,json_str);
            if(ret)
            {
                 std::cout << "Data inserted into Redis successfully" << std::endl;
            } 
            else  
            {
                std::cout << "Failed to insert data into Redis" << std::endl;
            }
            s.send_ok(fd, SUCCESS);
        }
    }
}

void S_User::S_ViewFriendinfo (int fd,std::string str)
{
    std::cout << str << std::endl;
    nlohmann::json js = nlohmann::json::parse(str);
    std::string friend_id = js["friend_id"];
    std::string id = js["id"];
    std::cout << id << "正在查看  --->   " << friend_id <<"  的信息" << std::endl;
    redisAsyncContext redis;
    if(redis.HashExit("userinfo",friend_id)!=1)
    {
        std::cout << "用户不存在" << std::endl;
        Sen s;
        s.send_ok(fd, NEXIST);
    }
    else 
    {
        if(!redis.Zexists(id,friend_id))
        {
            std::cout << "不是好友" << std::endl;
            Sen s;
            s.send_ok(fd, ISNFRIEND);
            return ;
        }
        else 
        {
            std::string userinfo = redis.HashGet("userinfo",friend_id);
            Sen s;
            s.send_ok(fd, SUCCESS);
            s.send_cil(fd, userinfo);
        }
    }
}

void S_User::S_Viewpersonalinfo(int fd,std::string str)
{
    std::cout << str << std::endl;
    nlohmann::json js = nlohmann::json::parse(str);
    std::string id = js["id"];
    std::cout << id << " 正在查看个人信息 " << std::endl;
    redisAsyncContext redis;
    std::string getinfo= redis.HashGet("userinfo",id);
    std::cout << getinfo << std::endl;
    Sen s;
    s.send_ok(fd, SUCCESS);
    s.send_cil(fd, getinfo);
}

void S_User::S_BlockFriend(int fd,std::string str)
{
    std::cout << str << std::endl;
    nlohmann::json js = nlohmann::json::parse(str);
    std::string id = js["id"];
    std::string friend_id = js["friend_id"];
    std::cout << id << " 正在屏蔽  --->   " << friend_id << std::endl;
    redisAsyncContext redis;
    if(redis.HashExit("userinfo",friend_id)!=1)
    {
        std::cout << "用户不存在" << std::endl;
        Sen s;
        s.send_ok(fd, NEXIST);
    }
    else 
    {
        if(!redis.Zexists(id, friend_id))
        {
            std::cout << "用户不是好友" << std::endl;
            Sen s;
            s.send_ok(fd, ISNFRIEND);
        }
        else
        {
            int status ;
            redis.Zstatus(id, friend_id,status );
            if(status == FRIEND)
            {
                std::cout << id << " 正在屏蔽  --->   " << friend_id << "成功" << std::endl;
                redis.Zadd(id, BLOCK, friend_id);
                Sen s;
                s.send_ok(fd, SUCCESS);
            }
            else if(status == BLOCK)
            {
                std::cout << "用户已屏蔽" << std::endl;
                Sen s;
                s.send_ok(fd, ISBLOCKFRIEND);
            }
        }
    }
}

void S_User::S_ViewFriends(int fd,std::string str)
{
    std::cout << str << std::endl;
    nlohmann::json js = nlohmann::json::parse(str);
    std::string id = js["id"];
    std::cout << "用户 " << id << "正在查看好友列表 :" << std::endl;
    redisAsyncContext redis;
    std::vector<std::string> OnlineFriends ;
    std::vector<std::string> OfflineFriends ;
    int start = 0;
    int stop = -1;//stop设置为-1,表示获取所有元素
    std::vector<std::string> friends = redis.Zrange(id, start,  stop);
    for(auto &friend_id : friends)
    {
        int status;
        
            if(redis.Zstatus(id, friend_id,status))
            {
                if(status == FRIEND)
                {
                    if(redis.Ifexit("online", friend_id))
                    {
                        OnlineFriends.push_back(friend_id);
                    }
                    else 
                    {
                        OfflineFriends.push_back(friend_id);
                    }
                }
                //
                //屏蔽好友不显示在好友列表里面
            }
        
    }
    if(OnlineFriends.size()==0 && OfflineFriends.size()==0)
    {
        std::cout << "用户 " << id << " 好友列表为空" << std::endl;
        Sen s;
        s.send_ok(fd, NOFRIEND);
    }
    else if(OnlineFriends.size()!=0 || OfflineFriends.size()!=0)
    {
        std::cout << "用户 " << id << " 不为空 :" << std::endl;
        nlohmann::json friends_json ;
        friends_json["online"]=OnlineFriends;
        friends_json["offline"]=OfflineFriends;
        Sen s;
        s.send_ok(fd,SUCCESS);
        s.send_cil(fd,friends_json.dump());
    }
}

void S_User::S_DeleFriends(int fd,std::string str)
{
    std::cout << str << std::endl;
    nlohmann::json js = nlohmann::json::parse(str);
    std::string id = js["id"];
    std::string friend_id = js["friend_id"];
    std::cout << id << " 正在删除好友  --->   " << friend_id << std::endl;
    redisAsyncContext redis;
    if(redis.HashExit("userinfo",friend_id)!=1)
    {
        std::cout << "用户不存在" << std::endl;
        Sen s;
        s.send_ok(fd, NEXIST);
    }
    else 
    {
        if(!redis.Zexists(id, friend_id))
        {
            std::cout << "用户不是好友 " << std::endl;
            Sen s;
            s.send_ok(fd,ISNFRIEND);
        }
        else 
        {
            if(redis.Zrem(id, friend_id))
            {
                std::cout << id << " 正在删除好友  --->   " << friend_id << "成功" << std::endl;
                std::cout << "正在从 " << friend_id << " 的好友列表中消除 " << id << std::endl;
                if(redis.Zrem(friend_id, id))
                {
                    std::cout << "消除成功" << std::endl;
                }
                else 
                {
                    std::cout << "消除失败" << std::endl;
                }
                Sen s;
                s.send_ok(fd, SUCCESS);
            }
            else 
            {
                std::cout << "删除失败" << std::endl;
                Sen s ;
                s.send_ok(fd,FAILURE);
            }
        }
    }
}

void S_User::S_ViewBlockFriends(int fd,std::string str)
{
    std::cout << str << std::endl;
    nlohmann::json js = nlohmann::json::parse(str);
    std::string id = js["id"];
    std::cout << id << " 正在查看屏蔽列表 :" << std::endl;
    redisAsyncContext redis;
    std::vector<std::string> BlockFriends ;
    int start = 0;
    int stop = -1;//stop设置为-1,表示获取所有元素
    std::vector<std::string> friends = redis.Zrange(id, start,  stop);
    for(auto &friend_id : friends)
    {
        int status ;
        if(redis.Zstatus(id, friend_id,status))
        {
            if(status == BLOCK)
            {
                BlockFriends.push_back(friend_id);
            }
        }
    }
    if(BlockFriends.size()!=0)
    {
        nlohmann::json friends_json ;
        friends_json["block"]=BlockFriends;
        Sen s;
        s.send_ok(fd,SUCCESS);
        s.send_cil(fd,friends_json.dump());
    }
    else 
    {
        std::cout << "用户没有屏蔽列表" << std::endl;
        Sen s;
        s.send_ok(fd,NOBLOCKFRIEND);
    }
}

void S_User::S_ViewFriendRequest(int fd,std::string str)
{
    std::cout << str << std::endl;
    nlohmann::json js = nlohmann::json::parse(str);
    std::string id = js["id"];
    redisAsyncContext redis;
    std::unordered_map<std::string,std::string> friends;
    friends = redis.HashGetAll(id+"0");
    if(friends.size()==0)
    {
        Sen s;
        s.send_ok(fd,NOFRIENDREQUSET);
    }
    else 
    {
        std::vector<std::string > friend_id;
        std::vector<std::string > friend_msg;
        redis.HashClear(id+"0");
        for(auto &it : friends)
        {
            friend_id.push_back(it.first);
            friend_msg.push_back(it.second);
        }
        Sen s;
        nlohmann::json js1 ;
        js1["id"]= friend_id;
        js1["msg"]= friend_msg;
        s.send_ok(fd,SUCCESS);
        s.send_cil(fd,js1.dump());
    }
}

void S_User::S_HandleFriendRequset(int fd,std::string str)
{
    std::cout << str << std::endl;
    nlohmann::json js = nlohmann::json::parse(str);
    std::string id = js["id"];
    std::string friend_id = js["friend_id"];
    int choice = js["choice"];
    std::cout << id << " 正在处理好友请求  --->   " << friend_id << std::endl;
    redisAsyncContext redis;
    if(choice==ACCEPT)
    {
        redis.Zadd(id,FRIEND,friend_id);
        std::cout << id << " 接受了 " << friend_id << " 的好友请求" << std::endl;
        redis.Zadd(friend_id+"Sign",FRIENDSIGN,id);
        redis.Zadd(friend_id,FRIEND,id);
        std::cout << friend_id << " 接受了 " << id << " 的好友请求" << std::endl;
        //redis.Zadd(friend_id,FRIEND,id);
    }
}

void S_User::S_PrivateChat(int fd,std::string str,int dest)
{
    std::cout << str << std::endl;
    nlohmann::json js = nlohmann::json::parse(str);
    std::string id = js["id"];
    std::string friend_id = js["friend_id"];
    std::cout << id << " 正在私聊  --->   " << friend_id << std::endl;
    redisAsyncContext redis;
    int status ;
    if(!redis.Zexists(id, friend_id))
    {
        std::cout << "用户不是好友" << std::endl;
        Sen s;
        s.send_ok(fd, NOFRIEND);
    }
    else if(redis.Zstatus(id, friend_id,status))
    {
        if(status == BLOCK)
        {
            Sen s;
            s.send_ok(fd, BLOCK);
        }
        else if(status ==FRIEND)
        {
            Sen s;
            s.send_ok(fd,SUCCESS);
            std::cout << id << " 正在与 " << friend_id << " 私聊" << std::endl;
            //std::string find_id ;
            // // if(redis.Zstatus(id+"chat",friend_id,find_id))
            // // {
            // //     redis.Zrem(id+"chat",friend_id);
            // // }
            redis.Zadd("chat",friend_id,id);
            //redis.HashSet("chat",id,friend_id);
            std::cout << "设置 " << id << " 和 " << friend_id << " 的记录表" << std::endl;
            Msg m(fd,dest,id,friend_id);
            std::vector<std::string> offline;
            std::string source_name;
            source_name = redis.HashGet("id_name",id);
            std::string dest = redis.HashGet("id_name",friend_id);
            offline = redis.Lrange(source_name+dest+"offline",0,-1);
            redis.Ltrim(source_name+dest+"offline",1,0);
            //redis.Insert("chat",id);
            m.send_off(offline);
        }
    }
}

void S_User::S_UnBlockFriend(int fd,std::string str)
{
    std::cout << str << std::endl;
    nlohmann::json js = nlohmann::json::parse(str);
    std::string id = js["id"];
    std::string friend_id = js["friend_id"];
    std::cout << id << " 正在取消屏蔽好友  --->   " << friend_id << std::endl;
    redisAsyncContext redis;
    int status ;
    if(redis.HashExit("userinfo",friend_id)!=1)
    {
        std::cout << "用户不存在" << std::endl;
        Sen s;
        s.send_ok(fd, NEXIST);
    }
    else if(redis.Zstatus(id, friend_id, status))
    {
        if(status == BLOCK)
        {
            redis.Zadd(id,FRIEND,friend_id);
            Sen s;
            s.send_ok(fd,SUCCESS);
        }
        else 
        {
            Sen s;
            s.send_ok(fd,FAILURE);
        }
    }
}

void S_User::S_Chat(int fd,std::string rec_quest,int dest)
{
    std::cout << rec_quest << std::endl;
    nlohmann::json js = nlohmann::json::parse(rec_quest);
    std::string id = js["id"];
    std::string friend_id = js["friend_id"];
    std::string time = js["time"];
    std::string msg = js["msg"];
    std::string message = time + ":" + msg;
    redisAsyncContext redis;
    // redis.Zadd("chat",friend_id,id);
    // std::cout << "设置 " << id << " 和 " << friend_id << " 的记录表" << std::endl;
    Msg m(fd,dest,id,friend_id);
    m.run(message);
}

void S_User::S_ExitChat(int fd,std::string str)
{
    redisAsyncContext redis;
    std::cout << str << std::endl;
    nlohmann::json js = nlohmann::json::parse(str);
    std::string id = js["id"];
    std::cout << id << " 正在退出聊天" << std::endl;
    redis.Zrem("chat",id);
    Sen s;
     //s.send_cil(fd,"exit");
    //s.send_ok(fd,SUCCESS);
    //redis.HashDele("chat", id);
    nlohmann::json js1 = {{"send","exit"}};
    s.send_cil(fd,js1.dump());
}


void S_User::S_Refresh(int fd,std::string str)
{
    std::cout << str << std::endl;
    nlohmann::json js = nlohmann::json::parse(str);
    std::string id = js["id"];
    std::cout << id << " 正在刷新" << std::endl;
    redisAsyncContext redis;
    // std::string source_name;
    // source_name = redis.HashGet("id_name",id);
    std::vector<std::string> offline;
    std::vector<std::string> Goffline;
    std::vector<std::string> Sign;
    // offline = redis.Lrange(source_name+"offline",0,-1);
    std::cout << "正在刷新获取消息" << std::endl;
    offline = redis.Zrange(id+"chat",0,-1);
    Goffline = redis.Zrange(id+"Gchat",0,-1);
    Sign = redis.Zrange(id+"Sign",0,-1);
    std::cout << "skld" << std::endl;
    int length = redis.Llen(id+"file_info1");
    std::cout << "文件数量为" << length << std::endl;
    std::vector<std::string> sign;
    std::vector<std::string> Gsign;
    std::vector<std::string> SSign;
    std::cout << "已获取到消息 " << std::endl;
    if(offline.size()==0 && Goffline.size()==0 &&Sign.size()==0&&length==0)
    {
        Sen s;
        std::cout << "没有通知消息" << std::endl;
        s.send_ok(fd,NOSIGN);
        return ;
    }
    for(int i = 0;i < offline.size();i++)
    {
        int status ;
        std::string send = offline[i];

        if(redis.Zstatus(id,send,status))
        {
            if(status == FRIEND)
            {
                int count ;
                redis.Zstatus(id+"chat",send,count);
                std::string message = send + ": 发来了 " + std::to_string(count) + "条消息";
                // Sen s;
                std::cout << message << std::endl;
                sign.push_back(message);
            }
        }
    }
    for(int i=0;i<Goffline.size();i++)
    {
        int status;
        std::string send = Goffline[i];
        int count ;
        redis.Zstatus(id+"Gchat",send,count);
        std::string message = send + ": 发来了 " + std::to_string(count) + "条消息";
        std::cout << message << std::endl;
        Gsign.push_back(message);
    }
    for(int i=0;i<Sign.size();i++)
    {
        int status;
        std::string send = Sign[i];
        if(redis.Zstatus(id+"Sign",send,status))    
        {
            if(status == FRIENDSIGN)
            {
                std::string message = send + "同意了好友请求" ;
                SSign.push_back(message);
            }
            else if(status == GROUPSIGN)
            {
                std::string message = "成功加入" + send;
                SSign.push_back(message);
            }
            
        }
    }
    redis.Zclear(id+"Sign");
    redis.Ltrim(id+"file_info1",1,0);
    redis.Zclear(id+"chat");
    redis.Zclear(id+"Gchat");
    Sen s;
    s.send_ok(fd,SUCCESS);
    nlohmann::json send_sign = {{"send",sign},
    {"Gsend",Gsign},
    {"Ssend",SSign},
    {"Length",length}
    };
    // for(int i = 0;i < sign.size();i++)
    // {
    //     std::cout << sign[i] << std::endl;
    // }
    s.send_cil(fd,send_sign.dump());
}

void S_User::S_CreateGroup(int fd,std::string str)
{
    std::cout << str << std::endl;
    nlohmann::json js = nlohmann::json::parse(str);
    std::string id = js["id"];
    std::string group_name = js["Group_name"];
    std::cout << id << " 正在创建群聊" << std::endl;
    redisAsyncContext redis;
    if(redis.Ifexit("Group_name",group_name))
    {
        std::cout << "群名已存在" << std::endl;
        Sen s;
        s.send_ok(fd,GROUPNAMEEXIST);
        return ;
    }
    else
    {
        nlohmann::json js1 = {
            {"group_name",group_name},
            {"group_owner",id}
        };
        std::cout << "群聊名称不存在 " << std::endl;
        std::string group_id = m_rand();
        int n = redis.HashSet("Group_info",group_id,js1.dump());
        int m = redis.Insert("Groups",group_id);
        int k = redis.Insert("Group_name",group_name);
        int l = redis.Zadd(group_id,OKSPEAK,id);
        int r = redis.HashSet("G_id_name",group_id,group_name);
        if(n==REDIS_REPLY_ERROR ||m==REDIS_REPLY_ERROR || k==REDIS_REPLY_ERROR || l==REDIS_REPLY_ERROR || r==REDIS_REPLY_ERROR)
        {
            std::cout << "创建群聊失败 " << std::endl;
            Sen s;
            s.send_ok(fd,FAILURE);
            return ;
        }
        else 
        {
            std::cout << "创建群聊成功 " << std::endl;
            Sen s;
            s.send_ok(fd,SUCCESS);
            s.send_cil(fd,group_id);
        }
    }
    return ;
}

void S_User::S_JoinGroup(int fd,std::string str,int dest)
{
    std::cout << str << std::endl;
    nlohmann::json js = nlohmann::json::parse(str);
    std::string id = js["id"];
    std::string Group_id = js["Group_id"];
    std::cout << id << "正在申请加入群聊" << std::endl;
    redisAsyncContext redis;
    if(!redis.Ifexit("Groups",Group_id))
    {
        std::cout << "群聊不存在" << std::endl;
        Sen s;
        s.send_ok(fd,GROUPIDNOTEXIST);
        return ;
    }
    else if(redis.Zexists(Group_id,id))
    {
        std::cout << "已经在群聊中" << std::endl;
        Sen s;
        s.send_ok(fd,ALREADYJOIN);
        return ;
    }
    std::string group_name;
    group_name = redis.HashGet("Group_info",Group_id);
    std::string owener;
    nlohmann::json js1 = nlohmann::json::parse(group_name);
    owener = js1["group_owner"];
    if(owener==id)
    {
        std::cout << "加入自己的群聊" << std::endl;
        Sen s;
        s.send_ok(fd,JOINOWNGROUP);
        return;
    }
    else 
    {
        nlohmann::json js2 ={
            {"msg",js["msg"]},
            {"time",js["time"]},
            {"group_id",Group_id}
        };
        redis.HashSet(owener+"group",id,js2.dump());
        //std::string send = "你管理的群聊群聊" + Group_id + "有申请加入" ;
        Sen s;
        s.send_ok(fd,SUCCESS);
    }
}

void S_User::S_QuitGroup(int fd,std::string str)
{
    std::cout << str << std::endl;
    nlohmann::json js = nlohmann::json::parse(str);
    std::string id = js["id"];
    std::string Group_id = js["Group_id"];
    std::cout << id << "正在退出群聊" << Group_id <<std::endl;
    redisAsyncContext redis;
    if(!redis.Ifexit("Groups",Group_id))
    {
        std::cout << "不存在该群组" << std::endl;
        Sen s;
        s.send_ok(fd,GROUPIDNOTEXIST);
        return;
    }
    if(!redis.Zexists(Group_id,id))
    {
        std::cout << "不在该群组中" << std::endl;
        Sen s;
        s.send_ok(fd,NOTINGROUP);
        return;
    }
    std::string owner;
    owner = redis.HashGet("Group_info",Group_id);
    std::string G_O_id ;
    nlohmann::json js1 = nlohmann::json::parse(owner);
    G_O_id = js1["group_owner"];
    if(G_O_id == id)
    {
        std::cout << "群主不能退出群聊" << std::endl;
        Sen s;
        s.send_ok(fd,NOTQUIT);
        return;
    }
    else 
    {
        redis.Zrem(Group_id,id);
        std::cout << "成功退出群聊" << std::endl;
        Sen s;
        s.send_ok(fd,SUCCESS);
    }
}

void S_User::S_ListGroup(int fd,std::string str)
{
    std::cout <<str << std::endl;
    nlohmann::json js = nlohmann::json::parse(str);
    std::string id = js["id"];
    std::unordered_map<std::string,std::string> group_list;
    redisAsyncContext redis;
    group_list= redis.HashGetAll("Group_info");
    std::vector<std::string > join;
    std::vector<std::string > j_owners;
    std::vector<std::string > nojoin;
    std::vector<std::string > no_owners;
    std::vector<std::string > creat;
    if(group_list.size()==0)
    {
        Sen s;
        s.send_ok(fd,NOGROUP);
        return;
    }
    for(auto &it :group_list)
    {
        if(redis.Zexists(it.first, id))
        {
            nlohmann::json js1 = nlohmann::json::parse(it.second);
            if(id==js1["group_owner"])
            {
                creat.push_back(it.first);
            }
            else 
            {
                join.push_back(it.first);
                j_owners.push_back(js1["group_owner"]);
            }
        }
        // else 
        // {
            
        //     nlohmann::json js1 = nlohmann::json::parse(it.second);
        //     nojoin.push_back(it.first);    
        //     no_owners.push_back(js1["group_owner"]);
           
            
        // }
    }

    nlohmann::json js2 ={
        {"join",join},
        {"join_owner",j_owners},
        // {"nojoin",nojoin},
        // {"nojoin_owner",no_owners},
        {"creat",creat}
    };
    Sen s;
    s.send_ok(fd,SUCCESS);
    s.send_cil(fd,js2.dump());
}

void S_User::S_MemberGroup(int fd,std::string str)
{
    std::cout << str << std::endl;
    nlohmann::json js = nlohmann::json::parse(str);
    std::string id = js["id"];
    std::string Group_id = js["Group_id"];
    std::cout << id << "正在查看群聊" << Group_id <<std::endl;
    redisAsyncContext redis;
    if(!redis.Ifexit("Groups",Group_id))
    {
        std::cout << "不存在该群组" << std::endl;
        Sen s;
        s.send_ok(fd,GROUPIDNOTEXIST);
        return;
    }
    else 
    {
        std::vector<std::string> member;
        member = redis.Zrange(Group_id,0,-1);
        nlohmann::json js2 ={
            {"member",member}
        };
        Sen s;
        s.send_ok(fd,SUCCESS);
        s.send_cil(fd,js2.dump());
        return ;
    }
}

void S_User::S_DeleGroup(int fd,std::string str)
{
    std::cout << str << std::endl;
    nlohmann::json js = nlohmann::json::parse(str);
    std::string id = js["id"];
    std::string Group_id = js["Group_id"];
    std::cout << id << "正在删除群聊" << Group_id <<std::endl;
    redisAsyncContext redis;
    if(!redis.Ifexit("Groups",Group_id))
    {
        
        std::cout << "不存在该群组" << std::endl;
        Sen s;
        s.send_ok(fd,GROUPIDNOTEXIST);
        return;
    }
    if(!redis.Zexists(Group_id,id))
    {
        std::cout << "该用户不在该群组中" << std::endl;
        Sen s;
        s.send_ok(fd,NOTINGROUP);
        return;
    }
    else 
    {
        std::string get_info = redis.HashGet("Group_info",Group_id);
        nlohmann::json js1 = nlohmann::json::parse(get_info);
        if(id!=js1["group_owner"])
        {
            std::cout << "该用户不是群主" << std::endl;
            Sen s;
            s.send_ok(fd,NOTGROUPOWNER);
            return;
        }
        else 
        {
            std::cout << "正在解散该群聊" << std::endl;
            redis.HashDele("Group_info", Group_id);
            redis.Delevalue("Groups", Group_id);
            redis.Delevalue("Group_name",js1["group_name"]);
            redis.Zclear(Group_id);
            redis.HashDele("G_id_name",Group_id);
            redis.DeleAll(Group_id+"manger");
            //清除消息
            redis.Ltrim(js1["group_name"],1,0);
            std::cout << "解散成功" << std::endl;
            Sen s;
            s.send_ok(fd,SUCCESS);
            return;
        }
    }
}

void S_User::S_HandleGroupRequset(int fd,std::string str)
{
    std::cout << str << std::endl;
    nlohmann::json js = nlohmann::json::parse(str);
    std::string id = js["id"];
    std::unordered_map<std::string,std::string> Group_requset;
    redisAsyncContext redis;
    Group_requset =  redis.HashGetAll(id+"group");
    std::vector<std::string> ids;
    std::vector<std::string> msgs;
    for(auto it = Group_requset.begin();it!=Group_requset.end();it++)
    {
        std::string Group_id = it->first;
        std::string msg = it->second;
        ids.push_back(Group_id);
        msgs.push_back(msg);
    }
    nlohmann::json js2 ={
        {"ids",ids},
        {"msgs",msgs}
    };
    Sen s;
    s.send_ok(fd,SUCCESS);
    s.send_cil(fd,js2.dump());
    redis.HashClear(id+"group");
}

void S_User::S_DoGroupRequest(int fd,std::string str)
{
    std::cout << str << std::endl;
    nlohmann::json js = nlohmann::json::parse(str);
    std::string id = js["id"];
    std::string Group_id = js["Group_id"];
    std::string msg = js["msg"];
    std::cout << id << "正在处理群聊请求" << std::endl;
    redisAsyncContext redis;
    if(msg=="agree")
    {
        // nlohmann::json js1= nlohmann::json::parse(msg);
        redis.Zadd(Group_id,OKSPEAK,id);
        std::string id_o = js["id_o"];
        redis.Zadd(id+"Sign",GROUPSIGN,Group_id);
        redis.HashDele(id_o+"group",id);
    }
    else 
    {
        std::cout << "拒绝" << std::endl;
        std::string id_o = js["id_o"];
        redis.HashDele(id_o+"group",id);
    }
    Sen s;
    s.send_ok(fd,SUCCESS);
}

void S_User::S_ChatGroup(int fd,std::string str)//用来获取到群聊的离线消息
{
    std::cout << str << std::endl;
    nlohmann::json js = nlohmann::json::parse(str);
    std::string id = js["id"];
    std::string group_id = js["group_id"];
    std::cout << id << "正在发送群聊消息" << std::endl;
    redisAsyncContext redis;
    if(!redis.Ifexit("Groups", group_id))
    {
        std::cout << "群聊不存在" << std::endl;
        Sen s;
        s.send_ok(fd,GROUPIDNOTEXIST);
        return;
    }
    if(!redis.Zexists(group_id,id))
    {
        std::cout << "你不在群聊中" << std::endl;
        Sen s;
        s.send_ok(fd,NOTINGROUP);
        return;
    }
    Sen s;
    s.send_ok(fd,SUCCESS);
    redis.Zadd("chat",group_id,id);
    // std::string find_id ;
    // if(redis.Zstatus(id+"Gchat",group_id,find_id))
    // {
    //     redis.Zrem(id+"Gchat",group_id);
    // }
    std::vector<std::string> offline;
    //std::string name = redis.HashGet("G_id_name",group_id);
    std::string name = redis.HashGet("id_name",id);
    offline = redis.Lrange(name+group_id+"offline",0,-1);
    Msg m(fd,0,id,group_id);
    m.send_off(offline);
    redis.Ltrim(name+group_id+"offline",1,0);
}

void S_User::S_GroupChat(int fd,std::string str,std::vector<int> &v,std::vector<std::string> &v2)
{
    std::cout << str << std::endl;
    nlohmann::json js = nlohmann::json::parse(str);
    std::string id = js["id"];
    std::string friend_id = js["friend_id"];
    std::string time = js["time"];
    std::string msg = js["msg"];
    std::string message = time + ":" + msg;
    redisAsyncContext redis;
    int status_b ;
    if(redis.Zstatus(friend_id,id,status_b))
    {
        if(status_b ==NOTSPEAK)
        {
            std::cout << "你已被禁言" << std::endl;
            Sen s;
            nlohmann::json j = {
                {"send","!"+message}
            };
            s.send_cil(fd,j.dump());
            return;
        }
    }
    nlohmann::json j = {
        {"send",message}
    };
    redis.Lpush(friend_id+"message",j.dump());
    if(redis.Llen(friend_id+"message")>=100)
    {
        Sql sql;
        sql.addChatName(friend_id+"message");
        std::vector<std::string> messages = redis.Lrange(friend_id+"message",0,-1);
        sql.insertMultipleChats(friend_id+"message",messages);
        redis.Ltrim(friend_id+"message",1,0);
    }
    for(int i=0;i<v.size();i++)
    {
        if(id==v2[i])
        {
            continue;
        }
        
        Msg m(fd,v[i],id,friend_id);
        m.run_Group(message,v2[i]);
    }
}

void S_User::S_DeleSomeone(int fd,std::string str)
{
    std::cout << str << std::endl;
    nlohmann::json js = nlohmann::json::parse(str);
    std::string id = js["id"];
    std::string user_id =js["User_id"];
    std::string Group_id = js["Group_id"];
    redisAsyncContext redis;
    if(!redis.Ifexit("Groups", Group_id))
    {
        std::cout << "群聊不存在" << std::endl;
        Sen s;
        s.send_ok(fd,GROUPIDNOTEXIST);
        return;
    }
    if(!redis.Zexists(Group_id,user_id))
    {
        std::cout << "该成员不在群聊中 " << std::endl;
        Sen s;
        s.send_ok(fd,NOTINGROUP);
        return;
    }
    std::cout << id << "正在管理群聊" << std::endl;
    std::string get_info = redis.HashGet("Group_info",Group_id);
    nlohmann::json js1 = nlohmann::json::parse(get_info);
    if(user_id==js1["group_owner"])
    {
        std::cout << "!!!倒反天罡!!!" << std::endl;
        Sen s;
        s.send_ok(fd,NB);
        return ;
    }
    if(redis.Ifexit(Group_id+"manger",user_id))
    {
        if(id!=js1["group_owner"])
        {
            std::cout << "没有权限" << std::endl;
            Sen s;
            s.send_ok(fd,NOHIGH);
            return ;
        }
        else 
        {
            if(redis.Ifexit(Group_id+"manger",id)||js1["group_owner"]==id)
            {
                std::cout << "正在删除该成员" << std::endl;
                redis.Zrem(Group_id,user_id);
                Sen s;
                s.send_ok(fd, SUCCESS);
                return ;
            }
        }
    }
    else 
    {
        if(redis.Ifexit(Group_id+"manger",id)||js1["group_owner"]==id)
        {
            std::cout << "正在删除该成员" << std::endl;
            redis.Zrem(Group_id,user_id);
            Sen s;
            s.send_ok(fd, SUCCESS);
            return ;
        }
        else 
        {
            std::cout << "没有权限" << std::endl;
            Sen s;
            s.send_ok(fd,NOHIGH);
            return ;
        }
        
    }
}

void S_User::S_NoSpeakSomeone(int fd,std::string str)
{
    std::cout << str << std::endl;
    nlohmann::json js = nlohmann::json::parse(str);
    std::string id = js["id"];
    std::string user_id =js["User_id"];
    std::string Group_id = js["Group_id"];
    redisAsyncContext redis;
    if(!redis.Ifexit("Groups", Group_id))
    {
        std::cout << "群聊不存在" << std::endl;
        Sen s;
        s.send_ok(fd,GROUPIDNOTEXIST);
        return;
    }
    if(!redis.Zexists(Group_id,user_id))
    {
        std::cout << "该成员不在群聊中 " << std::endl;
        Sen s;
        s.send_ok(fd,NOTINGROUP);
        return;
    }
    std::cout << id << "正在管理群聊" << std::endl;
    std::string get_info = redis.HashGet("Group_info",Group_id);
    nlohmann::json js1 = nlohmann::json::parse(get_info);
    if(user_id==js1["group_owner"])
    {
        std::cout << "!!!倒反天罡!!!" << std::endl;
        Sen s;
        s.send_ok(fd,NB);
        return ;
    }
    if(redis.Ifexit(Group_id+"manger",user_id))
    {
        if(id!=js1["group_owner"])
        {
            std::cout << "没有权限" << std::endl;
            Sen s;
            s.send_ok(fd,NOHIGH);
            return ;
        }
        else 
        {
            if(redis.Ifexit(Group_id+"manger",id)||js1["group_owner"]==id)
            {
                std::cout << "正在禁言该成员" << std::endl;
                redis.Zadd(Group_id,NOTSPEAK,user_id);
                Sen s;
                s.send_ok(fd, SUCCESS);
                return ;
            }
        }
    }
    else 
    {
        if(redis.Ifexit(Group_id+"manger",id)||js1["group_owner"]==id)
        {
            std::cout << "正在禁言该成员" << std::endl;
            redis.Zadd(Group_id,NOTSPEAK,user_id);
            Sen s;
            s.send_ok(fd, SUCCESS);
            return ;
        }
        else 
        {
            std::cout << "没有权限" << std::endl;
            Sen s;
            s.send_ok(fd,NOHIGH);
            return ;
        }   
    }
}

void S_User::S_SpeakSomeone(int fd,std::string str)
{
    std::cout << str << std::endl;
    nlohmann::json js = nlohmann::json::parse(str);
    std::string id = js["id"];
    std::string user_id =js["User_id"];
    std::string Group_id = js["Group_id"];
    redisAsyncContext redis;
    if(!redis.Ifexit("Groups", Group_id))
    {
        std::cout << "群聊不存在" << std::endl;
        Sen s;
        s.send_ok(fd,GROUPIDNOTEXIST);
        return;
    }
    if(!redis.Zexists(Group_id,user_id))
    {
        std::cout << "该成员不在群聊中 " << std::endl;
        Sen s;
        s.send_ok(fd,NOTINGROUP);
        return;
    }
    std::cout << id << "正在管理群聊" << std::endl;
    std::string get_info = redis.HashGet("Group_info",Group_id);
    nlohmann::json js1 = nlohmann::json::parse(get_info);
    if(user_id==js1["group_owner"])
    {
        std::cout << "!!!倒反天罡!!!" << std::endl;
        Sen s;
        s.send_ok(fd,NB);
        return ;
    }
    if(redis.Ifexit(Group_id+"manger",user_id))
    {
        if(id!=js1["group_owner"])
        {
            std::cout << "没有权限" << std::endl;
            Sen s;
            s.send_ok(fd,NOHIGH);
            return ;
        }
        else 
        {
            if(redis.Ifexit(Group_id+"manger",id)||js1["group_owner"]==id)
            {
                std::cout << "正在禁言该成员" << std::endl;
                redis.Zadd(Group_id,OKSPEAK,user_id);
                Sen s;
                s.send_ok(fd, SUCCESS);
                return ;
            }
        }
    }
    else 
    {
        if(redis.Ifexit(Group_id+"manger",id)||js1["group_owner"]==id)
        {
            std::cout << "正在禁言该成员" << std::endl;
            redis.Zadd(Group_id,OKSPEAK,user_id);
            Sen s;
            s.send_ok(fd, SUCCESS);
            return ;
        }
        else 
        {
            std::cout << "没有权限" << std::endl;
            Sen s;
            s.send_ok(fd,NOHIGH);
            return ;
        }
    }
}

void S_User::S_AddGroupManager(int fd,std::string str)
{
    std::cout << str << std::endl;
    nlohmann::json js = nlohmann::json::parse(str);
    std::string id = js["id"];
    std::string user_id =js["User_id"];
    std::string Group_id = js["Group_id"];
    redisAsyncContext redis;
    if(!redis.Ifexit("Groups", Group_id))
    {
        std::cout << "群聊不存在" << std::endl;
        Sen s;
        s.send_ok(fd,GROUPIDNOTEXIST);
        return;
    }
    if(!redis.Zexists(Group_id,user_id))
    {
        std::cout << "该成员不在群聊中 " << std::endl;
        Sen s;
        s.send_ok(fd,NOTINGROUP);
        return;
    }
    std::cout << id << "正在管理群聊" << std::endl;
    std::string get_info = redis.HashGet("Group_info",Group_id);
    nlohmann::json js1 = nlohmann::json::parse(get_info);
    if(id!=js1["group_owner"])
    {
        std::cout << "该用户不是群主" << std::endl;
        Sen s;
        s.send_ok(fd,NOTGROUPOWNER);
        return;
    }
    else 
    {
        Sen s;
        redis.Insert(Group_id+"manger",user_id);
        s.send_ok(fd,SUCCESS);
        return;
    }
}

void S_User::S_DeleGroupManager(int fd,std::string str)
{
    std::cout << str << std::endl;
    nlohmann::json js = nlohmann::json::parse(str);
    std::string id = js["id"];
    std::string user_id =js["User_id"];
    std::string Group_id = js["Group_id"];
    redisAsyncContext redis;
    if(!redis.Ifexit("Groups", Group_id))
    {
        std::cout << "群聊不存在" << std::endl;
        Sen s;
        s.send_ok(fd,GROUPIDNOTEXIST);
        return;
    }
    if(!redis.Zexists(Group_id,user_id))
    {
        std::cout << "该成员不在群聊中 " << std::endl;
        Sen s;
        s.send_ok(fd,NOTINGROUP);
        return;
    }
    std::cout << id << "正在管理群聊" << std::endl;
    std::string get_info = redis.HashGet("Group_info",Group_id);
    nlohmann::json js1 = nlohmann::json::parse(get_info);
    if(id!=js1["group_owner"])
    {
        std::cout << "该用户不是群主" << std::endl;
        Sen s;
        s.send_ok(fd,NOTGROUPOWNER);
        return;
    }
    else 
    {
        Sen s;
        redis.Delevalue(Group_id+"manger",user_id);
        s.send_ok(fd,SUCCESS);
        return;
    }
}
void S_User::S_IfFriend(int fd,std::string str)
{
    std::cout << str << std::endl;
    nlohmann::json js = nlohmann::json::parse(str);
    std::string id =js["id"];
    std::string friend_id = js["friend_id"];
    redisAsyncContext redis;
    int status ;
    if(redis.Zstatus(friend_id,id,status))
    {
        if(status ==BLOCK)
        {
            Sen s;
            s.send_ok(fd,BLOCK);
            return;
        }
        else {
            Sen s;
            s.send_ok(fd,SUCCESS);
        }
    }
    else 
    {
        Sen s;
        s.send_ok(fd,NOFRIEND);
        return;
    }
}

void S_User::S_IfInGroup(int fd,std::string str)
{
    std::cout << str << std::endl;
    nlohmann::json js = nlohmann::json::parse(str);
    std::string id =js["id"];
    std::string friend_id = js["friend_id"];
    redisAsyncContext redis;
    int status ;
    if(redis.Zstatus(friend_id,id,status))
    {
        if(status == NOTSPEAK)
        {
            std::cout << "该用户被禁言" << std::endl;
            Sen s;
            s.send_ok(fd,NOTSPEAK);
            return;
        }
        else {
            Sen s;
            s.send_ok(fd,SUCCESS);
            return;
        }
    }
    else 
    {
        std::cout << "该用户不在群聊中" << std::endl;
        Sen s;
        s.send_ok(fd,NOTINGROUP);
        return;
    }
}

void S_User::S_SendFile(int fd, std::string str, int dest)
{
    
    nlohmann::json js;
    try 
    {
        js = nlohmann::json::parse(str);
    } 
    catch (const nlohmann::json::parse_error& e) 
    {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        Sen s;
        s.send_ok(fd, FAILURE);
        return;
    }

    std::string id = js["id"];
    std::string fileName = js["file_name"];
    size_t fileSize = js["file_size"];
    std::string path = js["file_path"];
    std::string friend_id = js["friend_id"];
    std::cout << id << "将要向" << friend_id << "发送文件" << fileName << std::endl;

    std::filesystem::path dir = "../test/";
    if (!std::filesystem::exists(dir)) 
    {
        try 
        {
            std::filesystem::create_directories(dir);
        } 
        catch (const std::filesystem::filesystem_error& e) 
        {
            std::cerr << "Failed to create directories: " << e.what() << std::endl;
            Sen s;
            s.send_ok(fd, FAILURE);
            return;
        }
    }

    std::string creatFile = dir.string() + fileName;
    FILE *fp = fopen(creatFile.c_str(), "wb");
    if (fp == NULL) 
    {
        std::cerr << "Failed to open file for writing" << std::endl;
        Sen s;
        s.send_ok(fd, FAILURE);
        return;
    }

    int original_flags = fcntl(fd, F_GETFL, 0);
    if (original_flags == -1) 
    {
        std::cerr << "Failed to get file descriptor flags: " << strerror(errno) << std::endl;
        fclose(fp);
        Sen s;
        s.send_ok(fd, FAILURE);
        return;
    }

    if (fcntl(fd, F_SETFL, original_flags & ~O_NONBLOCK) == -1) 
    {
        std::cerr << "Failed to set file descriptor to blocking mode: " << strerror(errno) << std::endl;
        fclose(fp);
        Sen s;
        s.send_ok(fd, FAILURE);
        return;
    }

    int len;
    char buffer[1310720];
    off_t total_received = 0;

    try {
        while (total_received < fileSize) 
        {
            len = recv(fd, buffer, sizeof(buffer), 0);
            if (len <= 0) 
            {
                if (len < 0) 
                {
                    perror("recv");
                }
                fclose(fp);
                Sen s;
                s.send_ok(fd, FAILURE);
                return;
            }

            fwrite(buffer, 1, len, fp);
            total_received += len;

            // float progress = static_cast<float>(total_received) / fileSize * 100;
            // std::cout << "Progress: " << progress << "%" << std::endl;
        }
    } catch (...) {
        std::cerr << "An error occurred during file reception" << std::endl;
        fclose(fp);
        Sen s;
        s.send_ok(fd, FAILURE);
        return;
    }

    fclose(fp);

    if (fcntl(fd, F_SETFL, original_flags) == -1) 
    {
        std::cerr << "Failed to restore file descriptor flags: " << strerror(errno) << std::endl;
    }

    if (total_received == fileSize) 
    {
        std::cout << "File received successfully!" << std::endl;
        Sen s;
        s.send_ok(fd, SUCCESS);
    } 
    else 
    {
        std::cerr << "File size mismatch" << std::endl;
        Sen s;
        s.send_ok(fd, FAILURE);
    }
    std::string time = js["time"];
    nlohmann::json js2 = {
        {"file_name",fileName},
        {"file_size",fileSize},
        {"time",time},
        {"friend",id}
    };
    redisAsyncContext redis;
    if(redis.Ifexit("Groups",friend_id))
    {
        std::vector<std::string > ids;
        ids=redis.Zrange(friend_id, 0, -1);
        for(auto it = ids.begin();it != ids.end();it++)
        {
            std::string id = *it;
            redis.HashSet(id+"file_info",id,js2.dump());
            redis.Lpush(id+"file_info1","a");
        }
    }
    else 
    {
        redis.HashSet(friend_id+"file_info",id,js2.dump());
        redis.Lpush(friend_id+"file_info1","a");  
    }
    std::cout << "设置文件列表" << std::endl;
    return;
}

void S_User::S_MessageFile(int fd,std::string str)
{
    std::cout << str << std::endl;
    nlohmann::json js = nlohmann::json::parse(str);
    std::string id = js["id"];
    std::cout << "正在处理文件消息"  << std::endl;
    std::unordered_map<std::string, std::string> map;
    redisAsyncContext redis;
    map = redis.HashGetAll(id+"file_info");
    if(map.size()==0)
    {
        std::cout << "文件列表为空" << std::endl;
        Sen s;
        s.send_ok(fd,NOFILES);
        return;
    }
    std::vector<std::string> ids;
    std::vector<std::string> files;
    for(auto it = map.begin();it != map.end();it++)
    {
        ids.push_back(it->first);
        files.push_back(it->second);
    }
    nlohmann::json js2 = {
        {"ids",ids},
        {"files",files}
    };
    Sen s;
    s.send_ok(fd,SUCCESS);
    s.send_cil(fd,js2.dump());
    //redis.HashClear(id+"file_info");
    return;
}

void S_User::S_ReceiveFile(int fd,std::string str)
{
    std::cout <<str << std::endl;
    nlohmann::json js = nlohmann::json::parse(str);
    std::string id = js["id"];
    std::string file_name = js["file_name"];
    std::string path = "../test/"+file_name;
    std::cout << id << "正在接收文件 " << std::endl;
    std::filesystem::path filePath (path);
    if(!std::filesystem::exists(filePath))
    {
        std::cout << "文件不存在" << std::endl;
        Sen s;
        s.send_ok(fd,FILENOTEXISTS);
        return ;
    }
    if(!std::filesystem::is_regular_file(filePath))
    {
        std::cout << "文件不是普通文件" << std::endl;
        Sen s;
        s.send_ok(fd,FILENOTEXISTS);
        return ;
    }
    Sen s;
    s.send_ok(fd,SUCCESS);
    int file = open(path.c_str(),O_RDONLY);
    if(file == -1)
    {
        std::cout << "Fail to open file" << std::endl; 
        Sen s;
        s.send_ok(fd,FILENOTEXISTS);
        return ;
    }
    int original_flags = fcntl(fd,F_GETFL,0);
    if(original_flags == -1)
    {
        std::cout << "Failed to get file descriptor flags" << std::endl;
        Sen s;
        s.send_ok(fd,FILENOTEXISTS);
        return ;
    }
    if(fcntl(fd,F_SETFL,original_flags & ~O_NONBLOCK) == -1)
    {
        std::cout << "Failed to set file descriptor to blocking mode" << std::endl;
        Sen s;
        s.send_ok(fd,FAILURE);
        return ;
    }

    size_t file_size = std::filesystem::file_size(filePath);
    std::cout << "文件大小为" << file_size << std::endl;
    s.send_ok(fd,file_size);
    // Send initial file info
    const size_t BUFFER_SIZE = 1310720;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_sent;
    off_t offset = 0;
    size_t total_sent = 0;

   while (total_sent < file_size)
   {
        ssize_t bytes_to_send = std::min(BUFFER_SIZE, file_size - total_sent);
        bytes_sent = sendfile(fd, file, &offset, bytes_to_send);

        if (bytes_sent <= 0) 
        {
            std::cerr << "sendfile error" << std::endl;
            break;
        }

        total_sent += bytes_sent;
        // float progress = (float)total_sent / file_size * 100;
        // std::cout << "Progress: " << progress << "%" << std::endl;
    }
    if (fcntl(fd, F_SETFL, original_flags) == -1) 
    {
        std::cerr << "Failed to restore file descriptor flags: " << strerror(errno) << std::endl;
    }
    std::cout << id << "接收文件成功" << std::endl;
    redisAsyncContext redis;
    
}

void S_User::S_ShowHistory(int fd,std::string str)
{
    std::cout << str << std::endl;
    nlohmann::json js = nlohmann::json::parse(str);
    std::string id = js["id"];
    std::string find_id = js["Chat_id"];
    redisAsyncContext redis;
    std::cout << id << "正在查看历史消息" << std::endl;
    if(redis.Ifexit("Groups",find_id))
    {
        std::string status;
        if(redis.Zstatus(find_id,id,status))
        {
            if(!redis.L_Ifexist(find_id+"message"))
            {
                Sen s;
                s.send_ok(fd,NOHISTORY);
                return;
            }
            Sql sql;
            std::vector<std::string> messages1;
            std::vector<std::string> messages2;
            messages1=redis.Lrange(find_id+"message", 0,-1);
            messages2=sql.getChatHistory(find_id+"Message");
            nlohmann::json js1 = {{"redis",messages1},
            {"sql",messages2}};
            Sen s;
            s.send_ok(fd,SUCCESS);
            s.send_cil(fd,js1.dump());
            return ;
        }
        else 
        {
            Sen s;
            s.send_ok(fd,NOTINGROUP );
            return ;
        }
    }
    else if(redis.HashExit("userinfo",find_id))
    {
        std::string status;
        if(redis.Zstatus(find_id,id,status))
        {
            std::cout << "进入是否好友界面" << std::endl;
            std::string name1 = redis.HashGet("id_name",find_id);
            std::string name2 = redis.HashGet("id_name",id);
            std::cout << "在这1" << std::endl;
            std::string end = name1+name2;
            if(!redis.HashExit("Records", end))
            {
                Sen s;
                s.send_ok(fd,NOMESSAGES);
                return;
            }
            std::string records = redis.HashGet("Records",end);
            std::cout << "在这12" << std::endl;
            std::vector<std::string> messages1;
            std::vector<std::string> messages2;
            Sql sql;
            std::cout << "在这123" << std::endl;
            messages1=redis.Lrange(records+"Message", 0,-1);
            std::cout << "在这1234" << std::endl;
            messages2=sql.getChatHistory(records+"Message");
            nlohmann::json js1 ;
            js1["redis"]=messages1;
            js1["sql"]=messages2;
            Sen s;
            s.send_ok(fd,SUCCESS);
            s.send_cil(fd,js1.dump());
        }
        else 
        {
            std::cout << "进入是否好友界面2" << std::endl;
            Sen s;
            s.send_ok(fd,NOFRIEND);
            return ;
        }
    }
    else 
    {
        Sen s;
        s.send_ok(fd, NOID);
        return ;
    }
}