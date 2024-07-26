#include "../ser/message.hpp"
#include "../redis/redis.hpp"
#include <hiredis/hiredis.h>
#include <string>

void Msg::send_off(std::vector<std::string> &offline)
{
    if(offline.size() == 0)
    {
        Sen s;
        s.send_ok(this->m_source,NOOFFLINEMSG);
        //s.send_cil(this->m_dest,offline);
        std::cout << "没有离线消息" << std::endl;
    }
    else 
    {
        nlohmann::json j = {{"off_msg",offline}};
        Sen s;
        s.send_ok(this->m_source,SUCCESS);
        s.send_cil(this->m_source,j.dump());
        std::cout <<"离线消息发送成功 " << std::endl;
    }    
}

std::string m_rand_a() 
{
    std::string str;
    std::random_device rd;  // 使用硬件产生真正随机数的种子 会使用硬件的随机数生成器来产生真正的随机数
    std::mt19937 gen(rd()); // 使用 Mersenne Twister 算法作为随机数引擎  可以产生随即平均分布良好且周期很长的随机数
    std::uniform_int_distribution<int> dist(0, 9); // 定义均匀分布在 [0, 9] 范围内的随机数分布器
    //是标准库中的随机数分布器模板，用于生成随机的整数随机数
    for (int i = 0; i < 6; i++) 
    {
        int random_digit = dist(gen);  // 生成随机数字
        char digit_char = '0' + random_digit;  // 将随机数字转换为对应的字符
        str += digit_char;  // 将字符追加到字符串末尾
    }
    return str;
}

std::string get_Records(std::string &first,std::string &second)
{
    std::string id = m_rand_a();
    redisAsyncContext redis;
    if(redis.HashExit("Records", first))
    {
        return redis.HashGet("Records", first);
    }
    redis.HashSet("Records",first,id);
    redis.HashSet("Records", second, id);
    return id;
}
void Msg::run(std::string &message)
{
    //int fd = m_dest;
    redisAsyncContext redis;
    //std::cout << "dsadjlasjd" << std::endl;
    std::string friend_source = redis.HashGet("id_name", id_s);
    std::string friend_dest = redis.HashGet("id_name", id_d);
    std::cout << friend_source << std::endl;
    std::cout << friend_dest << std::endl;
    
        int status_b ;
        if(redis.Zstatus(id_d,id_s,status_b))
        {
            if(status_b ==BLOCK)
            {
                std::cout << "对方已屏蔽你" << std::endl;
                Sen s;
                nlohmann::json j = {
                    {"send","!"+message}
                };
                s.send_cil(this->m_source,j.dump());
                return;
            }
        }
        if(!redis.Zexists(id_d, id_s))
        {
            Sen s;
            nlohmann::json j = {
                {"send","?"+message}
            };
            s.send_cil(this->m_source, j.dump());
            return ;
        }
        //std::vector<std::string> offline;
        //offline = redis.Lrange(friend_source+"offline", 0, -1);
        //this->send_off(offline);
        //while(1)
        //{
        std::cout << "接收到 " <<friend_source << " 发送给 " << friend_dest << " 的消息 " << message << std::endl;
        std::string send = friend_source +message;
        nlohmann::json j = {
            {"send",send}};
        std::string first = friend_dest+friend_source ;
        std::string second = friend_source+friend_dest ;
        std::cout << j.dump() << std::endl;
        std::string records = get_Records(first,second);
        redis.Lpush(records+ "Message", j.dump());
        if(redis.Llen(records+ "Message") >= 100)
        {
            Sql sql;
            sql.addChatName(records + "Message");
            std::vector<std::string> messages = redis.Lrange(records + "Message", 0, -1);
            sql.insertMultipleChats(records + "Message", messages);
            redis.Ltrim(records+"Message",1, 0);
        }
        //std::cout << j.dump() << "    kashdkjaws" << std::endl;
        // std::cout << id_d << "               kashdkashdkj" << std::endl;
        std::string status;
        if (redis.Ifexit("online", id_d)) 
        {
            if (redis.Zstatus("chat", id_d,status)) 
            {
                std::cout << "Status found: " << status << std::endl;
                if (status == id_s) 
                {
                    Sen s;
                    std::cout << "直接发送" << std::endl;
                    s.send_cil(this->m_dest, j.dump());
                } 
                else 
                {
                    std::cout << "对方没有和你在聊天 " << std::endl;
                    redis.Lpush(friend_dest+friend_source + "offline", j.dump());
                    Sen s;
                    std::string message = "好友" + id_s + "给你发送了一条消息 !" ;
                    nlohmann::json ja = {
                        {"send",message}
                    };
                    s.send_cil(this->m_dest, ja.dump());
                    
                   // redis.Lpush(friend_dest + "offline", j.dump());
                    int status ;
                    if(redis.Zstatus(id_d+"chat",id_s,status))
                    {
                        status += 1;
                        redis.Zadd(id_d+"chat",status,id_s);
                    }
                    else
                    {
                        redis.Zadd(id_d + "chat",1,id_s);
                    }
                }
            }
            else 
            {
                std::cout << "对方在初始界面 " << std::endl;
                std::cout << "Adding to offline messages" << std::endl;
                redis.Lpush(friend_dest+friend_source + "offline", j.dump());
                Sen s;
                std::string message = "好友" + id_s + "给你发送了一条消息 !" ;
                nlohmann::json ja = {
                    {"send",message}
                };
                //s.send_cil(this->m_dest, ja.dump());
                //本来是要直接发送过去，但是没有线程接收
                redis.Lpush(friend_dest+friend_source + "offline", j.dump());
                // redis.Lpush(friend_dest + "online",j.dump());
                int status ;
                if(redis.Zstatus(id_d+"chat",id_s,status))
                {
                    status += 1;
                    redis.Zadd(id_d+"chat",status,id_s);
                }
                else
                {
                    redis.Zadd(id_d+ "chat",1,id_s);
                }
            }
        } 
        else 
        {
            std::cout << "Adding to offline messages" << std::endl;
            redis.Lpush(friend_dest+friend_source + "offline", j.dump());
            int status ;
            if(redis.Zstatus(id_d+"chat",id_s,status))
            {
                status += 1;
                redis.Zadd(id_d+"chat",status,id_s);
            }
            else
            {
                redis.Zadd(id_d+ "chat",1,id_s);
            }
        }
        //  while (true) 
        //  {
        //     int len = r.recv_cil(this->m_source, message);
        //     if (len > 0) 
        //     {
        //         std::cout << "接收发送消息" << std::endl;
        //         std::cout << friend_source << " 发送给 " << friend_dest << " 的消息 " << message << std::endl;
        //         // Handle sending message to destination here
        //     } 
        //     else if (len == -1) 
        //     {
        //         if (errno == EAGAIN || errno == EWOULDBLOCK) 
        //         {
        //             // No data available now, continue the loop to retry
        //             continue;
        //         } 
        //         else 
        //         {
        //             std::cerr << "Error receiving data: " << strerror(errno) << std::endl;
        //             break;
        //         }
        //     } 
        //     else 
        //     {
        //         std::cerr << "Client disconnected or error occurred: " << strerror(errno) << std::endl;
        //         break;
        //     }
        // }

        // std::cerr << "File descriptor is no longer valid: " << this->m_source << std::endl;
        // close(this->m_source);
    
}

void Msg::run_Group(std::string &message,std::string &id)
{
    redisAsyncContext redis;
    //std::cout << "dsadjlasjd" << std::endl;
    
    std::string friend_source = redis.HashGet("id_name", id_s);
    std::string friend_dest = redis.HashGet("id_name", id);
    std::cout << friend_source << std::endl;
    std::cout << friend_dest << std::endl;
    int status_b ;
    if(redis.Zstatus(id_d,id_s,status_b))
    {
        if(status_b ==NOTSPEAK)
        {
            std::cout << "你已被禁言" << std::endl;
            Sen s;
            nlohmann::json j = {
                {"send","!"+message}
            };
            s.send_cil(this->m_source,j.dump());
            return;
        }
    }
    std::cout << "接收到 " <<friend_source << " 发送给 " << friend_dest << " 的消息 " << message << std::endl;
    std::string send = friend_source +message;
    std::cout << send << std::endl;
    nlohmann::json j = {
        {"send",send}};
       
    std::string status;
    if (redis.Ifexit("online", id)) 
    {
        std::cout << "dsaklsdj" << std::endl;
        if (redis.Zstatus("chat", id,status)) 
        {
            std::cout << "1" << std::endl;
            std::cout << "Status found: " << status << std::endl;
            if (status == id_d) 
            {
                Sen s;
                std::cout << "直接发送" << std::endl;
                s.send_cil(this->m_dest, j.dump());
            } 
            else  
            {
                std::cout << "对方没有在该群聊 " << std::endl;
                redis.Lpush(friend_dest + "offline", j.dump());
                Sen s;
                std::string message = "群聊" + id_d + "给你发送了一条消息 !" ;
                nlohmann::json ja = {
                    {"send",message}
                };
                s.send_cil(this->m_dest, ja.dump());
                    
                   // redis.Lpush(friend_dest + "offline", j.dump());
                int status ;
                if(redis.Zstatus(id+"Gchat",id_d,status))
                {
                    status += 1;
                    redis.Zadd(id+"Gchat",status,id_d);
                }
                else
                {
                    redis.Zadd(id + "Gchat",1,id_d);
                }
            }
        }
        else 
        {
            std::cout << "对方在初始界面 " << std::endl;
            std::cout << "Adding to offline messages" << std::endl;
            redis.Lpush(friend_dest+id_d + "offline", j.dump());
            Sen s;
            std::string message = "好友" + id_s + "给你发送了一条消息 !" ;
            nlohmann::json ja = {
                {"send",message}
            };
                //s.send_cil(this->m_dest, ja.dump());
                //本来是要直接发送过去，但是没有线程接收
            redis.Lpush(friend_dest + "offline", j.dump());
                // redis.Lpush(friend_dest + "online",j.dump());
            int status ;
            if(redis.Zstatus(id+"Gchat",id_d,status))
            {
                status += 1;
                redis.Zadd(id+"Gchat",status,id_d);
            }
            else
            {
                redis.Zadd(id+ "Gchat",1,id_d);
            }
        }
    } 
    else 
    {
        std::cout << "Adding to offline messages" << std::endl;
        redis.Lpush(friend_dest+id_d + "offline", j.dump());
        int status;
        if(redis.Zstatus(id+"Gchat",id_d,status))
        {
            status += 1;
            redis.Zadd(id+"Gchat",status,id_d);
        }
        else
        {
            redis.Zadd(id+ "Gchat",1,id_d);
        }
    }
}
