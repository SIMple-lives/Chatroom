#include "../head/head.hpp"
#include <cstddef>

class Sen
{ 
public:
    int writen(int fd,char *buf,int len);
    void send_cil(int fd,const std::string &buf);
    void send_ok(int fd,int status);
    void send_ok(int fd,size_t status);
};

class Rec
{
public:
    int readn(int fd,char *buf,int len);
    int recv_cil(int fd,std::string &buf);
    int recv_ok(int fd);
    size_t recv_ok_Long(int fd);
};

inline int Sen::writen(int fd,char *buf,int len)
{
    const char *p = buf;
    int count = len ;
    while(count > 0)
    {
        int ret = send(fd,p,count,0);
        if(ret <= -1)
        {
            if(errno == EINTR && ret == -1)
                continue;
            else
            {
                std::cerr << "Error sending data: " << strerror(errno) << std::endl;
                return -1;
            }
        }

        else if(ret == 0)
        {
            continue;
        }
        p += ret;
        count -= ret;
    }
    return len;
}

inline void Sen::send_cil(int fd,const std::string &buf)
{
    if(fd == -1 || buf.empty())
    {
        return ;
    }
    std::vector<char> data(buf.size()+4);
    int _len = htonl(buf.size());
    memcpy(data.data(),&_len,4);
    memcpy(data.data()+4,buf.c_str(),buf.size());
    int ret = writen(fd, data.data(),data.size());
    if(ret < 0)
    {
        std::cerr << " writing error" << std::endl;
        close(fd);
    }
}

inline void Sen::send_ok(int fd,int status)
{
    if(send(fd,&status,sizeof(int),0)==-1)
    {
        std::cerr << "Error sending data: " << strerror(errno) << std::endl;
        close(fd);
    }
    else 
    {
        std::cout << "Status sent successfully" << std::endl;
    }    
}

inline void Sen::send_ok(int fd,size_t status)
{
    if(send(fd,&status,sizeof(size_t),0)==-1)
    {
        std::cerr << "Error sending data: " << strerror(errno) << std::endl;
        close(fd);
    }
    else 
    {
        std::cout << "Status sent successfully" << std::endl;
    }    
}

inline int Rec::readn(int fd,char *buf,int len)
{
    char *p = buf;
    int count = len;
    while(count > 0)
    {
        int ret = recv(fd,p,count,0);
        if(ret == -1)
        {
            if(errno == EINTR && ret == -1)
                continue;
            else
            {
                std::cerr << "Error receiving data: " << strerror(errno) << std::endl;
                close(fd);
                return -1;
            }
        }
        else if(ret == 0)
        {
            return len - count;
        }
        p += ret;
        count -= ret;
    }
    return len - count;
}

inline int Rec::recv_cil(int fd,std::string &buf)
{
    int len = 0;
    readn(fd, (char *)&len, 4);
    len = ntohl(len);
    char *data = (char *)malloc(len + 1);
    int ret = readn(fd, data, len);  
    if (ret == 0)
    {
        printf("对方断开链接\n");
        return -1;
    }
    else if (len != ret)
    {
        printf("数据接收失败\n");
    }
    data[len] = '\0';
    buf = data;

    return ret;
}

inline int Rec::recv_ok(int fd) 
{
    int status;
    ssize_t recv_bytes = recv(fd, &status, sizeof(int), 0);
    if (recv_bytes == -1)
    {
        std::cout << "recv state failed" << std::endl;
    }
    else if (recv_bytes == 0) // 客户端断开连接
    {
        std::cout << "Connection closed by peer." << std::endl;
        close(fd);
    }
    return status;
}

inline size_t Rec::recv_ok_Long(int fd)
{
    size_t status;
    ssize_t recv_bytes = recv(fd, &status, sizeof(size_t), 0);
    if (recv_bytes == -1)
    {
        std::cout << "recv state failed" << std::endl;
    }
    else if (recv_bytes == 0) // 客户端断开连接
    {
        std::cout << "Connection closed by peer." << std::endl;
    }
    return status;
}
//rev_ok
