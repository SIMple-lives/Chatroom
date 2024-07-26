#include "../head/head.hpp"
#include <hiredis/hiredis.h>
#include <string>
#include "../redis/redis.hpp"

//redis 执行命令为 redisCommand(redisContext *c, const char *format, ...)
//第一个参数代表redisContext结构体指针，第二个参数代表命令
//哈希表的相关操作
int redisAsyncContext::HashDele(const std::string &key, const std::string &field)
{
    std::string cmd = "hdel " + key + " " + field;
    // this->m_reply = (redisReply*)redisCommand(this->m_connettion, cmd.c_str());
    this->m_reply = (redisReply*)redisCommand(this->m_connettion, "hdel %s %s", key.c_str(), field.c_str());
    int num = this->m_reply->type;
    freeReplyObject(this->m_reply);
    return num;
}

int redisAsyncContext::HashExit(const std::string &key, const std::string &field)
{
    std::string cmd = "hexists " + key + " " + field;
    // this->m_reply = (redisReply*)redisCommand(this->m_connettion, cmd.c_str());
    this->m_reply = (redisReply*)redisCommand(this->m_connettion, "hexists %s %s", key.c_str(), field.c_str());
    int num = this->m_reply->integer;
    freeReplyObject(this->m_reply);
    return num;
}


std::string redisAsyncContext::HashGet(const std::string &key,const std::string &field)
{
    std::string cmd = "hget " + key + " " + field;
    // this->m_reply = (redisReply*)redisCommand(this->m_connettion, cmd.c_str());
    this->m_reply = (redisReply*)redisCommand(this->m_connettion, "hget %s %s", key.c_str(), field.c_str());
    std::string ret = this->m_reply->str;
    freeReplyObject(this->m_reply);
    return ret;
}

int redisAsyncContext::HashSet(const std::string &key,const std::string &field, const std::string &value)
{
    std::string cmd = "hmset " + key + " " + field + " " + value ;
    // this->m_reply = (redisReply*)redisCommand(this->m_connettion, cmd.c_str());
    this->m_reply = (redisReply*)redisCommand(this->m_connettion, "hmset %s %s %s", key.c_str(), field.c_str(), value.c_str());
    int type = this->m_reply->type;
    freeReplyObject(this->m_reply);
    return type;
}

std::unordered_map<std::string, std::string> redisAsyncContext::HashGetAll(const std::string &key) 
{
    std::unordered_map<std::string, std::string> result;
    std::string cmd = "HGETALL " + key;
    // m_reply = (redisReply *)redisCommand(m_connettion, cmd.c_str());
    m_reply = (redisReply *)redisCommand(m_connettion, "HGETALL %s", key.c_str());
    if (m_reply->type == REDIS_REPLY_ARRAY) 
    {
        for (size_t i = 0; i < m_reply->elements; i += 2) 
        {
            result[m_reply->element[i]->str] = m_reply->element[i + 1]->str;
        }
    }
    freeReplyObject(m_reply);
    return result;
}

int redisAsyncContext::HashClear(const std::string &key) 
{
    std::string cmd = "DEL " + key;
    // m_reply = (redisReply *)redisCommand(m_connettion, cmd.c_str());
    m_reply = (redisReply *)redisCommand(m_connettion, "DEL %s", key.c_str());
    int type = m_reply->type;
    freeReplyObject(m_reply);
    return type;
}

int redisAsyncContext::HashLen(const std::string &key)
{
    m_reply = (redisReply *)redisCommand(m_connettion,"HLEN %s", key.c_str());
    freeReplyObject(m_reply);
    return m_reply->integer;
}

//集合的相关操作
int redisAsyncContext::Insert(const std::string &key, const std::string&field)
{
    std::string cmd = "sadd " + key + " " + field;
    // this->m_reply = (redisReply*)redisCommand(this->m_connettion, cmd.c_str());
    this->m_reply = (redisReply*)redisCommand(this->m_connettion, "sadd %s %s", key.c_str(), field.c_str());
    int type = this->m_reply->type;
    freeReplyObject(this->m_reply);
    return type;
}

int redisAsyncContext::Ifexit(const std::string &key, const std::string &field)
{
    std::string cmd = "sismember " + key + " " + field;
    // this->m_reply = (redisReply*)redisCommand(this->m_connettion, cmd.c_str());
    this->m_reply = (redisReply*)redisCommand(this->m_connettion, "sismember %s %s", key.c_str(), field.c_str());
    int type = this->m_reply->integer;
    freeReplyObject(this->m_reply);
    return type;
}

int redisAsyncContext::Delevalue(const std::string &key, const std::string &value) 
{
    std::string cmd = "srem  " + key + "  " + value;
    // this->m_reply = (redisReply *)redisCommand(this->m_connettion, cmd.c_str());
    this->m_reply = (redisReply *)redisCommand(this->m_connettion, "srem %s %s", key.c_str(), value.c_str());
    int p = this->m_reply->type;
    freeReplyObject(this->m_reply);
    return p;
}   

int redisAsyncContext::DeleAll(const std::string &key)
{
    std::string cmd = "DEL " + key;
    // this->m_reply = (redisReply*)redisCommand(this->m_connettion, cmd.c_str());
    this->m_reply = (redisReply *)redisCommand(this->m_connettion, "DEL %s", key.c_str());
    int type = this->m_reply->type;
    freeReplyObject(this->m_reply);
    return type;
}

//有序集合的相关操作
int redisAsyncContext::Zadd(const std::string &key, int status,const std::string &member)
{
    std::string cmd = "zadd " + key + " " + std::to_string(status) + " " + member;
    // this->m_reply = (redisReply*)redisCommand(this->m_connettion, cmd.c_str());
    this->m_reply = (redisReply*)redisCommand(this->m_connettion, "zadd %s %d %s", key.c_str(), status, member.c_str());
    int type = this->m_reply->type;
    freeReplyObject(this->m_reply);
    return type;
}

int redisAsyncContext::Zadd(const std::string &key, std::string status,const std::string &member)
{
    std::string cmd = "zadd " + key + " " + status + " " + member;
    this->m_reply = (redisReply*)redisCommand(this->m_connettion, cmd.c_str());
    // this->m_reply = (redisReply*)redisCommand(this->m_connettion, "zadd %s %s %s", key.c_str(), status.c_str(), member.c_str());    
    int type = this->m_reply->type;
    freeReplyObject(this->m_reply);
    return type;
}
std::vector<std::string> redisAsyncContext::Zrange(const std::string &key, int start, int stop)
{
    std::string cmd = "zrange " + key + " " + std::to_string(start) + " " + std::to_string(stop);
    // this->m_reply = (redisReply *)redisCommand(this->m_connettion, cmd.c_str());
    this->m_reply = (redisReply *)redisCommand(this->m_connettion, "zrange %s %d %d", key.c_str(), start, stop);
    std::vector<std::string> members;
    if (this->m_reply->type == REDIS_REPLY_ARRAY)
    {
        for (size_t i = 0; i < this->m_reply->elements; ++i)
        {
            members.push_back(this->m_reply->element[i]->str);
        }
    }
    freeReplyObject(this->m_reply);
    return members;
}

int redisAsyncContext::Zrem(const std::string &key, const std::string &member)
{
    std::string cmd = "zrem " + key + " " + member;
    // this->m_reply = (redisReply *)redisCommand(this->m_connettion, cmd.c_str());
    this->m_reply = (redisReply *)redisCommand(this->m_connettion, "zrem %s %s", key.c_str(), member.c_str());
    int type = this->m_reply->type;
    freeReplyObject(this->m_reply);
    return type;
}

int redisAsyncContext::Zstatus(const std::string &key, const std::string &member, int &status)
{
    std::string cmd = "zscore " + key + " " + member;
    // this->m_reply = (redisReply *)redisCommand(this->m_connettion, cmd.c_str());
    this->m_reply = (redisReply *)redisCommand(this->m_connettion, "zscore %s %s", key.c_str(), member.c_str());
    if (this->m_reply->type == REDIS_REPLY_STRING)
    {
        status = std::stoi(this->m_reply->str);
        freeReplyObject(this->m_reply);
        return 1;
    }
    freeReplyObject(this->m_reply);
    return 0;
}

int redisAsyncContext::Zstatus(const std::string &key, const std::string &member, std::string &status)
{
    std::string cmd = "zscore " + key + " " + member;
    // this->m_reply = (redisReply *)redisCommand(this->m_connettion, cmd.c_str());
    this->m_reply = (redisReply *)redisCommand(this->m_connettion, "zscore %s %s", key.c_str(), member.c_str());
    
    if (!this->m_reply) 
    {
        std::cerr << "Command execution failed" << std::endl;
        return 0; 
        // 或者其他表示错误的值，例如 -1
    }

    if (this->m_reply->type == REDIS_REPLY_STRING) 
    {
        status = this->m_reply->str;
        freeReplyObject(this->m_reply);
        return 1;
    } 
    else 
    {
        // 处理非字符串类型的回复
        std::cerr << "Unexpected reply type: " << this->m_reply->type << std::endl;
        freeReplyObject(this->m_reply);
        return 0;
    }
}

bool redisAsyncContext::Zexists(const std::string &key, const std::string &member)
{
    std::string cmd = "zrank " + key + " " + member;
    // this->m_reply = (redisReply *) redisCommand(this->m_connettion,cmd.c_str());
    this->m_reply = (redisReply *) redisCommand(this->m_connettion, "zrank %s %s", key.c_str(), member.c_str());
    bool exists = (this->m_reply->type != REDIS_REPLY_NIL);
    freeReplyObject(this->m_reply);
    return exists;
}

int redisAsyncContext::Zclear(const std::string &key)
{
    std::string cmd = "del " + key;
    // this->m_reply = (redisReply *) redisCommand(this->m_connettion,cmd.c_str());
    this->m_reply = (redisReply *) redisCommand(this->m_connettion, "del %s", key.c_str());
    int status = this->m_reply->type;
    freeReplyObject(this->m_reply);
    return status;
}

//列表的相关操作
int redisAsyncContext::Lpush(const std::string &key, const std::string &value)
{
    //std::string cmd = "lpush " + key + " " + value;
    this->m_reply = (redisReply *)redisCommand(this->m_connettion, "LPUSH %s %s", key.c_str(), value.c_str());
     // 检查 m_reply 是否为 NULL
    if (this->m_reply == NULL) {
        std::cerr << "Error: redisCommand returned NULL" << std::endl;
        return -1; // 或其他合适的错误值
    }

    // 检查 m_reply 类型
    if (this->m_reply->type != REDIS_REPLY_INTEGER) {
        std::cerr << "Error: Expected integer reply" << std::endl;
        freeReplyObject(this->m_reply);
        return -1; // 或其他合适的错误值
    }
    int num  = this->m_reply->integer;
    freeReplyObject(this->m_reply);
    std::cout << "G" << std::endl;
    return num;
}

int redisAsyncContext::Llen(const std::string &key)
{
    std::string cmd = "llen " + key;
    // this->m_reply = (redisReply *)redisCommand(this->m_connettion, cmd.c_str());
    this->m_reply = (redisReply *)redisCommand(this->m_connettion, "llen %s", key.c_str());
    int num = this->m_reply->integer;
    freeReplyObject(this->m_reply);
    return num;
}

std::vector<std::string> redisAsyncContext::Lrange(const std::string &key, int start, int stop)
{
    std::string cmd = "lrange " + key + " " + std::to_string(start) + " " + std::to_string(stop);
    // this->m_reply = (redisReply *)redisCommand(this->m_connettion, cmd.c_str());
    this->m_reply = (redisReply *)redisCommand(this->m_connettion, "lrange %s %d %d", key.c_str(), start, stop);
    std::vector<std::string> values;
    if(this->m_reply->type == REDIS_REPLY_ARRAY)
    {
        for(size_t i = 0; i < this->m_reply->elements; ++i)
        {
            values.push_back(this->m_reply->element[i]->str);
        }
    }
    freeReplyObject(this->m_reply);
    return values;
}

int redisAsyncContext::Ltrim(const std::string &key, int start, int stop)
{
    std::string cmd = "ltrim " + key + " " + std::to_string(start) + " " + std::to_string(stop);
    // this->m_reply = (redisReply *)redisCommand(this->m_connettion, cmd.c_str());
    this->m_reply = (redisReply *)redisCommand(this->m_connettion, "ltrim %s %d %d", key.c_str(), start, stop);
    int status = this->m_reply->type;
    freeReplyObject(this->m_reply);
    return status;
}

std::string redisAsyncContext::Lpop(const std::string &key)
{
    std::string cmd = "lpop " + key;
    // this->m_reply = (redisReply *)redisCommand(this->m_connettion, cmd.c_str());
    this->m_reply = (redisReply *)redisCommand(this->m_connettion, "lpop %s", key.c_str());
    std::string value ;
    if(this->m_reply->type == REDIS_REPLY_STRING)
    {
        value = this->m_reply->str;
    }
    freeReplyObject(this->m_reply);
    return value;
}

int redisAsyncContext::L_Ifexist(const std::string &key)
{
    std::string cmd = "exists " + key;
    // this->m_reply = (redisReply *)redisCommand(this->m_connettion, cmd.c_str());
    this->m_reply = (redisReply *)redisCommand(this->m_connettion, "exists %s", key.c_str());
    if(this->m_reply->type == REDIS_REPLY_INTEGER)
    {
        int status = this->m_reply->integer;
        freeReplyObject(this->m_reply);
        return status;
    }
    freeReplyObject(this->m_reply);
    return 0;
}