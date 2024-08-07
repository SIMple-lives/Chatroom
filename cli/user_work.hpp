#include "../head/head.hpp"
#include "../head/define.hpp"
#include "../cli/User.hpp"
#include "../cli/prich.hpp"
#include <boost/mpl/identity.hpp>
#include <mutex>

class User_work :public Menu
{
public:
    User_work() :m_fd(0) ,m_id("") ,m_running(true)
    {
        
        start();
    }
    User_work(int fd,std::string id) :m_fd(fd),m_id(id),m_running(true)
    {
        
         start();
    }

    ~User_work()
    {
        m_running = false;
        if(refreshThread.joinable())
        {
            refreshThread.join();
        }
        std::cout << "User_work is deleted" << std::endl;
    }
    virtual void show_friend(std::string id,std::string name ,std::string telephone,int status) override
    {
        system("echo \"Friend INFO\" | figlet | boxes -d c | lolcat");
        std::string info = name + " 's Information ";
        printCentered(info, 59);
        std::cout << std::setw(59) << std::setfill('-') << '-' << std::endl; // 横线分隔
        std::string id1 = "ID: " + id;
        std::string name1 = "Name: " + name;
        std::string telephone1 = "Telephone: " + telephone;
        std::string strstatus;
        if(status == OFFLINE)
        {
            strstatus="Offline";
        }
        else if(status == ONLINE)
        {
            strstatus="Online";
        }
        else 
        {
            strstatus="Error";
        }
        std::string status1 = "Status: " + strstatus;
        printCentered(id1, 59);
        printCentered(name1, 59);
        printCentered(telephone1, 59);
        printCentered(status1, 59);
        std::cout << std::setw(59) << std::setfill('-') << '-' << std::endl;
    }

    void show_Personalinfo(std::string id,std::string name ,std::string telephone,int status);

    virtual void showFriend() override
    {
        std::string title = "Add Friend";
        int title_length = title.length();
        int total_length = 60; // 总行宽

        system("echo \"ADD Friend\" | figlet | boxes -d c | lolcat");
        std::cout << std::setfill('=') << std::setw(total_length) << "" << std::endl;
        std::cout << std::setfill(' ') << std::setw((total_length + title_length) / 2) << std::right << title << std::endl;
        std::cout << std::setfill('=') << std::setw(total_length) << "" << std::endl;

        std::cout << std::left << std::setfill(' ') << std::setw(10) << "                 1. Input friend's ID" << std::endl;
        std::cout << std::left << std::setfill(' ') << std::setw(10) << "                 2. Say something to your friend" << std::endl;

        std::cout << std::setfill('=') << std::setw(total_length) << "" << std::endl;

    }

    virtual void showGroups() override
    {
        system("echo \"Groups\" | figlet | boxes -d c | lolcat");

        // 打印标题
        std::string title = "YourID" + std::string(" 's Groups"); // 假设你的ID是"YourID"
        printCentered(title, 40);
        std::cout << std::setw(40) << std::setfill('-') << '-' << std::endl; // 横线分隔

        // 打印选项
        std::cout << std::left << std::setw(25) << "1. 创建群组" << "2. 加入群组" << std::endl;
        std::cout << std::left << std::setw(25) << "3. 退出群组" << "4. 群组列表" << std::endl;
        std::cout << std::left << std::setw(25) << "5. 解散群组" << "6. 群组成员" << std::endl;
        std::cout << std::left << std::setw(25) << "7. 群聊" << "8. 退出" << std::endl;
        std::cout << std::left << std::setw(25) << "9. 群聊申请消息" << "10. 群聊管理" << std::endl;

        // 打印分隔线
        std::cout << std::setw(40) << std::setfill('-') << '-' << std::endl;
    }
    void AddFriend() ;
    void ViewFriendRequest() ;
    void PrivateChat() ;
    void ViewFriends() ;
    void ViewFriendInfo() ;
    void BlockFriend() ;
    void ViewBlockedList() ;
    void DeleteFriend() ;
    void ViewPersonalInfo() ;
    void GroupChat() ;
    void Refresh() ;
    void Quit();
    void handleFriendRequset(std::vector<std::string> &id,std::vector<std::string> &mag);
    void UnBlockFriend(std::string  &friend_id);
    void Creat_Group(std::string id,int fd);
    void Join_Group(std::string id,int fd);
    void Quit_Group(std::string id,int fd);
    void Chat_Group(std::string id,int fd);
    void List_Group(std::string id,int fd);
    void Member_Group(std::string id,int fd);
    void Dele_Group(std::string id,int fd);
    void Handle_Group_Requset(std::string id,int fd);
    void Manage_Group(std::string id,int fd);
    void show_History();
    // void receiveMessagesFromServer()
    // {
    //     while (true)
    //     {
    //         // 接收服务器发来的消息
    //         std::string message;
    //         Rec rec ;
    //         rec.recv_cil(m_fd,message);
    //         //std::cout << message << std::endl;

    //         nlohmann::json js = nlohmann::json::parse(message);
    //         std::string receive = js["send"];
    //         std::cout << receive << std::endl;
    //         // 使用 Rec 类或其他适当的方法从服务器接收消息
    //         // 假设接收到的消息保存在 message 中

    //         // 处理接收到的消息
    //         // 这里你可以根据接收到的消息类型进行适当的处理
    //         // 例如，如果消息是私聊消息，可以显示给用户
    //     }
    // }

    // void startMessageReceiver()
    // {
    //     // 创建并启动接收消息的线程
    //     messageReceiverThread = std::thread(&User_work::receiveMessagesFromServer, this);
    // }

private:

    std::string getCurrentTime() 
    {
        auto now = std::chrono::system_clock::now();
        std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
        
        std::tm now_tm = *std::localtime(&now_time_t);
        
        std::ostringstream oss;
        oss << std::put_time(&now_tm, "%Y-%m-%d+%H:%M:%S");
        return oss.str();
    }

    void printCentered(const std::string& str, int lineLength) 
    {
        if (str.length() >= lineLength) 
        {
            // 如果字符串长度大于等于行长度，直接打印字符串
            std::cout << str << std::endl;
            return;
        }

        int totalPadding = lineLength - str.length();  // 计算需要填充的总空间
        int leftPadding = totalPadding / 2;             // 左侧填充空间
        int rightPadding = totalPadding - leftPadding;  // 右侧填充空间

        // 构造居中打印的字符串
        std::string centeredStr;
        centeredStr.append(leftPadding, ' ');   // 左侧填充空格
        centeredStr.append(str);                // 添加字符串
        centeredStr.append(rightPadding, ' ');  // 右侧填充空格

        // 打印居中字符串
        std::cout << centeredStr << std::endl;
    }

    int m_fd;
    std::string m_id;
public:
    std::thread refreshThread;
    std::atomic<bool> m_running;
    std::mutex m_mutex;

    void start()
    {
        this->refreshThread = std::thread(&User_work::refresh_time,this);
    }
    void refresh_time()
    {
        while(m_running)
        {
            std::this_thread::sleep_for(std::chrono::seconds(3));
            std::unique_lock <std::mutex> lock(m_mutex);
            this->Refresh();
        }
    }
};
