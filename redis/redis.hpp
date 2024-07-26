#pragma once

#include "../head/head.hpp"

class redisAsyncContext
{
public:
    redisAsyncContext() ;
    ~redisAsyncContext();

    //哈希表的相关操作
    int HashSet(const std::string &key,const std::string &field,const std::string &value);//设置哈希表
    int HashExit(const std::string &key,const std::string &field);//判断哈希表是否存在
    std::string HashGet(const std::string &key,const std::string &field);//获取哈希表的值
    std::unordered_map<std::string, std::string> HashGetAll(const std::string &key);//获取全部值
    int HashDele(const std::string &key,const std::string &field);//删除哈希表中的字段
    int HashClear(const std::string &key);
    int HashLen(const std::string &key);
    //集合表的相关操作
    int Insert(const std::string &key,const std::string &field);//插入集合
    int Ifexit(const std::string &key,const std::string &field);//判断集合是否存在某个成员
    int Delevalue(const std::string &key, const std::string &value);//删除
    int DeleAll(const std::string &key);//删除集合
    //有序集合的相关操作
    int Zadd(const std::string &key,int score,const std::string &member);//向有序集合添加成员，或者更新已存在成员的状态
    int Zadd(const std::string &key,std::string score,const std::string &member);//向有序集合添加成员，或者更新已存在成员的状态
    std::vector<std::string> Zrange(const std::string &key,int start,int stop);//返回有序集合指定区间的成员
    int Zrem(const std::string &key,const std::string &member);//移除成员
    int Zstatus(const std::string &key,const std::string &member,int &status);//判断指定成员是否是有序集合的成员
    int Zstatus(const std::string &key,const std::string &member,std::string &status);
    bool Zexists(const std::string &key,const std::string &member);//判断有序集合是否存在某个成员
    int Zclear(const std::string &key);
    //有关列表的相关操作
    int Lpush(const std::string &key,const std::string &value);//将值插入列表头部
    int Llen(const std::string &key);//列表长度
    std::vector<std::string> Lrange(const std::string &key,int start,int stop);//返回列表指定区间的成员
    int Ltrim(const std::string &key,int start,int stop);//只保留指定区间内的元素
    std::string Lpop(const std::string &key);
    int L_Ifexist(const std::string &key);//判断列表是否存在
private:
    redisContext * m_connettion;
    redisReply * m_reply;
};

inline redisAsyncContext::redisAsyncContext() : m_connettion(nullptr) , m_reply(nullptr) 
{
    this->m_connettion = redisConnect("127.0.0.1",6379);
    if(m_connettion->err)
    {
        redisFree(m_connettion);
        perror("redis connect error");
        exit(-1);
    }
}
inline redisAsyncContext::~redisAsyncContext()
{
    if(m_connettion != nullptr)
    {
        redisFree(this->m_connettion);
        m_connettion = nullptr;
    }
}