#include "../head/head.hpp"
#include "../ser/thread.hpp"
#include "../head/define.hpp"
#include "../sen_rec/sen_rec.hpp"
#include "../ser/User.hpp"
#include "../redis/redis.hpp"
#include <hiredis/hiredis.h>

#define PORT 8080

class Server 
{
    friend class S_User;
public:
    Server(int port = PORT)
        : m_lfd(createSocket()), m_efd(createEpoll()), m_shutdown(false), m_pool(10) 
    {
        signal(SIGPIPE, SIG_IGN); // Ignore SIGPIPE signals
        setSocketOptions(m_lfd);
        bindAddress(m_lfd, port);
        startListening(m_lfd);
        std::cout << "Server started" << std::endl;
        addToEpoll(m_lfd);
        m_pool.init(); // Initialize thread pool
    }

    ~Server() 
    {
        std::cout << "\033[31m 5 \033[0m" << std::endl;
        m_pool.shutdown(); // Shutdown thread pool
        close(m_efd); // Close epoll file descriptor
        close(m_lfd); // Close listening socket
    }

    void run() 
    {
        struct epoll_event resevent[1024];
        int size = sizeof(resevent) / sizeof(struct epoll_event);

        while (!m_shutdown) 
        {
            int num = epoll_wait(m_efd, resevent, size, -1);
            std::cout << "\033[31m 0 \033[0m" << std::endl;
            if(num == -1)
            {
                if(errno == EINTR) 
                {
                    continue;
                }
                perror("Epoll_wait failed");
                break;
            }
            for (int i = 0; i < num; ++i) 
            {
                int curfd = resevent[i].data.fd;
                if(resevent[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))//检测对应的描述符出现错误、挂起、连接断开或者半关闭等事件
                {
                    std::cout << "\033[31m 3 \033[0m" << std::endl;
                    handleDisconnect(curfd);
                }
                else if (curfd == m_lfd)  // 新的连接
                {
                    std::cout << "\033[31m 1 \033[0m" << std::endl;
                    handleNewConnection();
                } 
                else // 已有连接
                {
                    std::cout << "\033[31m 2 \033[0m" << std::endl;
                    configureKeepAlive(curfd);
                    handleClientRequest(curfd);
                }
            }
        }
    }
private:
    // std::mutex recv_file_mutex;
private:
    std::unordered_map<std::string,int> m_id_fd;
    int find_fd(std::string id)
    {
         auto it = m_id_fd.find(id);
        if (it != m_id_fd.end()) {
            return it->second;
        }
        return -1; 
    }
private:
    int m_lfd; // Listening socket
    int m_efd; // Epoll file descriptor
    bool m_shutdown; // Shutdown flag
    ThreadPool m_pool;
    
    int Group_fd(std::string str)
    {
        nlohmann::json j = nlohmann::json::parse(str);
        std::string id = j["id"];
        std::string Group_id  = j["Group_id"];
        redisAsyncContext redis;
        std::string get_owenr = redis.HashGet("Group_info",Group_id);
        nlohmann::json j1 = nlohmann::json::parse(get_owenr);
        std::string owner = j1["group_owner"];
        int ret = find_fd(owner);
        return ret;
    }
    int createSocket() 
    {
        int lfd = socket(AF_INET, SOCK_STREAM, 0);
        if (lfd == -1) 
        {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
        }
        return lfd;
    }

    int createEpoll()
    {
        //标志参数，0表示没有任何标志
        int efd = epoll_create1(0);
        if (efd == -1) 
        {
            perror("Epoll creation failed");
            exit(EXIT_FAILURE);
        }
        return efd;
    }

    void setSocketOptions(int lfd) 
    {
        int opt = 1;
        //设置SO_REUSEADDR选项,允许端口被重复使用
        if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) 
        {
            perror("Setsockopt failed");
            exit(EXIT_FAILURE);
        }
    }

    void bindAddress(int lfd, int port) 
    {
        struct sockaddr_in saddr = {};
        saddr.sin_family = AF_INET;
        saddr.sin_port = htons(port);
        saddr.sin_addr.s_addr = htonl(INADDR_ANY);
        if (bind(lfd, (struct sockaddr *)&saddr, sizeof(saddr)) == -1) 
        {
            perror("Bind failed");
            exit(EXIT_FAILURE);
        }
    }

    void startListening(int lfd) 
    {
        if (listen(lfd, 128) == -1) 
        {
            perror("Listen failed");
            exit(EXIT_FAILURE);
        }
    }

    void addToEpoll(int lfd) 
    {
        struct epoll_event event = {};
        event.events = EPOLLIN | EPOLLET;
        event.data.fd = lfd;

        if (epoll_ctl(m_efd, EPOLL_CTL_ADD, lfd, &event) == -1) 
        {
            perror("Epoll_ctl failed");
            exit(EXIT_FAILURE);
        }
    }

    void handleDisconnect(int curfd) 
    {
        std::cout << "Client disconnected1: " << curfd << std::endl;
        epoll_ctl(m_efd, EPOLL_CTL_DEL, curfd, nullptr); // Remove from epoll
        
        for(auto it = m_id_fd.begin(); it != m_id_fd.end(); it++)
        {
            if(it->second == curfd)
            {
                std::cout << it->first << std::endl;
                redisAsyncContext redis;
                std::string get_status= redis.HashGet("userinfo", it->first);
                std::cout << get_status << std::endl;
                nlohmann::json js= nlohmann::json::parse(get_status);
                int status= js["status"];
                if(status == ONLINE)
                {
                    js["status"] = OFFLINE;
                    redis.HashSet("userinfo", it->first, js.dump());
                    redis.Delevalue("online", it->first);
                }
                m_id_fd.erase(it);
                break;
            }
        }
        close(curfd);
    }

    void handleNewConnection() 
    {
        struct sockaddr_in addr = {};
        socklen_t addr_len = sizeof(addr);
        int cfd = accept(m_lfd, (struct sockaddr *)&addr, &addr_len);
        if (cfd == -1) 
        {
            perror("Accept failed");
            return;
        }

        // 设置新连接的描述符为非阻塞模式
        int flag = fcntl(cfd, F_GETFL, 0);
        fcntl(cfd, F_SETFL, flag | O_NONBLOCK);
        if (flag == -1 || fcntl(cfd, F_SETFL, flag | O_NONBLOCK) == -1) 
        {
            perror("Fcntl failed");
            close(cfd);
            return;
        }

        // 增加新的文件描述符到epoll
        struct epoll_event event = {};
        event.events = EPOLLIN | EPOLLET;
        event.data.fd = cfd;

        if (epoll_ctl(m_efd, EPOLL_CTL_ADD, cfd, &event) == -1) 
        {
            perror("Epoll_ctl accept failed");
            close(cfd);
        }
        std::cout << "New connection accepted: " << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port) << std::endl;
    }

    void configureKeepAlive(int curfd) 
    {
        int keep_alive = 1;     // 开启TCP心跳机制
        int keep_idle = 60;     // 开始首次心跳监测的TCP空闲时间
        int keep_interval = 10; // 两次心跳检测的时间
        int keep_count = 5;     // 连接心跳检测失败次数后，判定连接失效

        // 开启TCP心跳机制
        if (setsockopt(curfd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keep_alive, sizeof(keep_alive)) == -1) 
        {
            perror("setsockopt SO_KEEPALIVE");
            return;
        }

        // 设置首次心跳检测前的TCP空闭时间
        if (setsockopt(curfd, IPPROTO_TCP, TCP_KEEPIDLE, (void *)&keep_idle, sizeof(keep_idle)) == -1) 
        {
            perror("setsockopt TCP_KEEPIDLE");
            return;
        }

        // 设置两次心跳检测的间隔时间
        if (setsockopt(curfd, IPPROTO_TCP, TCP_KEEPINTVL, (void *)&keep_interval, sizeof(keep_interval)) == -1) 
        {
            perror("setsockopt TCP_KEEPINTVL");
            return;
        }

        // 设置连续心跳检测失败次数后，判定连接失效
        if (setsockopt(curfd, IPPROTO_TCP, TCP_KEEPCNT, (void *)&keep_count, sizeof(keep_count)) == -1) 
        {
            perror("setsockopt TCP_KEEPCNT");
            return;
        } 

        std::cout << "TCP Keepalive 已配置" << std::endl;
    }

    void handleClientRequest(int curfd) 
    {
        std::cout << "Handling client request on fd: " << curfd << std::endl;
        try 
        {
            struct epoll_event temp;
            temp.events = EPOLLIN | EPOLLET;
            temp.data.fd = curfd;
          
            // 从epoll中删除当前文件描述符
            if (epoll_ctl(m_efd, EPOLL_CTL_DEL, curfd, &temp) == -1) 
            {
                perror("Epoll_ctl-DEL failed");
                exit(EXIT_FAILURE);
            }

            std::string rec_quest;
            Rec rec;
            int rec_quest_len = rec.recv_cil(curfd, rec_quest);
            std::cout << "Received message length: " << rec_quest_len << std::endl;
            if (rec_quest_len <= 0) 
            {
                //客户端断开连接
                handleDisconnect(curfd);
                return;
            }

            std::cout << "Received message: " << rec_quest << std::endl;

            nlohmann::json json_quest = nlohmann::json::parse(rec_quest);
            int quest = json_quest["request"];
            S_User s_user;

            auto task = [this, curfd, quest, rec_quest, &s_user,json_quest]() 
            {
                try 
                {
                    switch (quest) 
                    {
                        case LOGIN: 
                        {
                            s_user.S_Login(curfd, rec_quest); 
                            m_id_fd.insert(std::make_pair(json_quest["id"],curfd));
                            break;
                        }
                        case LOGOUT: 
                            s_user.S_Signout(curfd, rec_quest);
                            break;
                        case SIGNUP: 
                            s_user.S_Signup(curfd, rec_quest); 
                            break;
                        case FINDPASSWORDSA: 
                            s_user.S_FindA(curfd, rec_quest); 
                            break;
                        case FINDPASSWORDSB: 
                            s_user.S_FindB(curfd, rec_quest); 
                            break;
                        case EXIT: 
                            s_user.S_EXIT(curfd, rec_quest); 
                            break;
                        case CHAT: 
                            {
                                std::string id=json_quest["friend_id"];
                                int dest = find_fd(id);
                                s_user.S_Chat(curfd, rec_quest,dest);
                                break;
                            }
                        case ADDFRIEND: 
                            s_user.S_AddFriend(curfd, rec_quest); 
                            break;
                        case FRIENDINFO:
                            s_user.S_ViewFriendinfo(curfd, rec_quest); 
                            break;
                        case PERSONALINFO: 
                            s_user.S_Viewpersonalinfo(curfd, rec_quest); 
                            break;
                        case BLOCKFRIEND : 
                            s_user.S_BlockFriend(curfd, rec_quest); 
                            break;
                        case VIEWFRIENDS : 
                            s_user.S_ViewFriends(curfd, rec_quest); 
                            break;
                        case DELEFRIEND: 
                            s_user.S_DeleFriends(curfd, rec_quest); 
                            break;
                        case VIEWBLOCKFRIENDS:
                            s_user.S_ViewBlockFriends(curfd, rec_quest); 
                            break;
                        case VIEWFRIENDSREQUSET:
                            s_user.S_ViewFriendRequest(curfd,rec_quest);
                            break;
                        case HANDLEFRIENDREQUSET:
                            s_user.S_HandleFriendRequset(curfd, rec_quest);
                            break;
                        case UNBLOCKFRIEND:
                            s_user.S_UnBlockFriend(curfd, rec_quest);
                            break;
                        case PRIVATECHAT:
                        {
                            std::string id=json_quest["friend_id"];
                            int dest = find_fd(id);
                            s_user.S_PrivateChat(curfd,rec_quest,dest);
                            break;
                        }
                        case MESSAGEFILE:
                            s_user.S_MessageFile(curfd,rec_quest);
                            break;
                        case RECEIVEFILE:
                            s_user.S_ReceiveFile(curfd, rec_quest);
                            break;
                        case EXITCHAT:
                            s_user.S_ExitChat(curfd, rec_quest);
                            break;
                        case REFRESH:
                            s_user.S_Refresh(curfd, rec_quest);
                            break;
                        case CREATEGROUP:
                            s_user.S_CreateGroup(curfd, rec_quest);
                            break;
                        case JOINGROUP:
                        {   
                            int dest=Group_fd(rec_quest);
                            s_user.S_JoinGroup(curfd,rec_quest,dest);
                            break;
                        }
                        case QUITGROUP:
                            s_user.S_QuitGroup(curfd, rec_quest);
                            break;
                        case LISTGROUP:
                            s_user.S_ListGroup(curfd,rec_quest);
                            break;
                        case MEMBERGROUP:
                            s_user.S_MemberGroup(curfd, rec_quest);
                            break;
                        case DELEGROUP:
                            s_user.S_DeleGroup(curfd,rec_quest);
                            break;
                        case HANDLEGROUPREQUEST:
                            s_user.S_HandleGroupRequset(curfd,rec_quest);
                            break;
                        case DOGROUPREQUEST:
                            s_user.S_DoGroupRequest(curfd, rec_quest);
                            break;
                        case DELESOMEONE:
                            s_user.S_DeleSomeone(curfd,rec_quest);
                            break;
                        case NOSPEAKSOMEONE:
                            s_user.S_NoSpeakSomeone(curfd, rec_quest);
                            break;
                        case SPEAKSOMEONE:
                            s_user.S_SpeakSomeone(curfd, rec_quest);
                            break;
                        case ADDGROUPMANGER:
                            s_user.S_AddGroupManager(curfd, rec_quest);
                            break;
                        case DELEGROUPMANGER:
                            s_user.S_DeleGroupManager(curfd, rec_quest);
                            break;
                        case CHATGROUP:
                        {
                            s_user.S_ChatGroup(curfd, rec_quest);
                            break;
                        }
                        case SHOWHISTORY:
                            s_user.S_ShowHistory(curfd, rec_quest);
                            break;
                        case GROUPCHAT:
                        {
                            std::string group_id = json_quest["friend_id"];
                            std::vector<std::string> group_member;
                            redisAsyncContext redis;
                            group_member=redis.Zrange(group_id,0,-1);
                            std::vector<int> fd_group;
                            for(auto i:group_member)
                            {
                                int f = find_fd(i);
                                fd_group.push_back(f);  
                            }
                            s_user.S_GroupChat(curfd,rec_quest,fd_group,group_member);
                            break;
                        }
                        case IFFRIEND:
                            s_user.S_IfFriend(curfd, rec_quest);
                            break;
                        case IFINGROUP:
                            s_user.S_IfInGroup(curfd,rec_quest);
                            break;
                        case SENDFILE:
                        {
                            std::string id = json_quest["friend_id"];
                            int dest = find_fd(id);
                            //std::lock_guard<std::mutex> lock(recv_file_mutex);
                            s_user.S_SendFile(curfd,rec_quest,dest);
                            break;
                        }
                        case USEREXIT: 
                        {
                            s_user.S_QUIT(curfd,rec_quest); 
                            for(auto it = m_id_fd.begin(); it != m_id_fd.end(); it++)
                            {
                                if(it->second == curfd)
                                {
                                    m_id_fd.erase(it);
                                    break;
                                }
                            }
                            break;
                        }
                        default: 
                            std::cerr << "Unknown request" << std::endl;   
                            break;
                    }

                    // 重新添加到epoll
                    if (fcntl(curfd, F_GETFD) != -1) 
                    {
                        // Re-add to epoll
                        struct epoll_event event = {};
                        event.events = EPOLLIN;
                        event.data.fd = curfd;

                        if (epoll_ctl(m_efd, EPOLL_CTL_ADD, curfd, &event) == -1) 
                        {
                            perror("Epoll_ctl ADD failed");
                        }
                    } 
                    else 
                    {
                        std::cerr << "File descriptor is no longer valid: " << curfd << std::endl;
                        //handleClientRequest(curfd);
                    }
                } 
                catch (const std::exception &e) 
                {
                    std::cerr << "Exception in task: " << e.what() << std::endl;
                    close(curfd); // Close file descriptor
                    epoll_ctl(m_efd, EPOLL_CTL_DEL, curfd, nullptr); 
                }
            };
            m_pool.submit(task);
        } 
        catch (const std::exception &e) 
        {
            std::cerr << "Exception in handleClientRequest: " << e.what() << std::endl;
            close(curfd); // Close the file descriptor
            epoll_ctl(m_efd, EPOLL_CTL_DEL, curfd, nullptr); 
        }
    }
};
