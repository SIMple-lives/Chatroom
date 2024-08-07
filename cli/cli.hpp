#include "../head/head.hpp"
#include "../cli/User.hpp"

class cilent :public Menu
{
public:
    cilent() = default;
    cilent(const std::string& server_addr, int port) : m_socket_fd(-1), server_addr(server_addr) 
    {
        try 
        {
            ip=server_addr;
            this->port=port;
            m_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
            if (m_socket_fd == -1) 
            {
                perror("Socket creation failed");
                throw std::runtime_error("Socket creation failed");
            }

            
            struct sockaddr_in server;
            memset(&server, 0, sizeof(server));
            server.sin_family = AF_INET;
            server.sin_port = htons(port);
            if (inet_pton(AF_INET, server_addr.c_str(), &server.sin_addr) <= 0) 
            {
                perror("Invalid socket address/Address not supported");
                throw std::invalid_argument("Invalid server address");
            }
            if (connect(m_socket_fd, (struct sockaddr*)&server, sizeof(server)) == -1) 
            {
                perror("Connection failed");
                throw std::runtime_error("Connection failed");
            }

            std::cout << "Connected to server " << server_addr << ":" << port << std::endl;
            // int flag = fcntl(m_socket_fd, F_GETFL, 0);
            // fcntl(m_socket_fd, F_SETFL, flag | O_NONBLOCK);
        } 
        catch (const std::exception& e) 
        {
            std::cerr << "Exception caught during initialization: " << e.what() << std::endl;
            // Clean up and rethrow the exception
            if (m_socket_fd != -1) 
            {
                close(m_socket_fd);
                m_socket_fd = -1;
            }
            throw; // Rethrow the exception
        }
    }
    
    void run() 
    {
    try 
    {
        int choice = 1;
        do 
        {
            this->show_menu();
            std::string ch;
            std::cin >> ch;
            
            if (ch == "1")
            {
                system("clear");
                this->showLogin();
                this->m_users.Login(this->m_socket_fd,this->port,this->ip);
            }
            else if (ch == "2")
            {
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                system("clear");
                this->showEnroll();
                this->m_users.Singup(this->m_socket_fd);
            }
            else if (ch == "3")
            {
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                system("clear");
                this->showLogout();
                this->m_users.Logout(this->m_socket_fd);
            }
            else if (ch == "4")
            {
                system("clear");
                this->showExit();
                this->m_users.Exit(this->m_socket_fd);
                choice = 0;
            }
            else
            {
                system("clear");
                std::cout << "Invalid choice" << std::endl;
                std::cout << "IF YOU WANT TO EXIT (y/n)?" << std::endl;
                std::string ok;
                std::cin >> ok;
                if (ok=="y")
                {
                    system("clear");
                    this->showExit();
                    choice = 0;
                    this->m_users.Exit(this->m_socket_fd);
                }
            }
        } while (choice);
    } 
    catch (const std::exception& e) 
    {
        std::cerr << "Exception caught during run-time: " << e.what() << std::endl;
    }
}


    ~cilent()
    {
        if(m_socket_fd != -1)
        {
            close(m_socket_fd);
            m_socket_fd = -1;
        }
    }
private:
// void receiveMessages() 
// {
//         while (!stop_thread) 
//         {
//             char buffer[1024];
//             ssize_t bytes_received = recv(m_socket_fd, buffer, sizeof(buffer) - 1, 0);
//             if (bytes_received > 0) 
//             {
//                 buffer[bytes_received] = '\0';
//                 std::unique_lock<std::mutex> lock(mutex);
//                 messages.push(std::string(buffer));
//                 lock.unlock();
//                 cond_var.notify_one();
//             }
//             else if (bytes_received == 0) 
//             {
//                 std::cerr << "Server closed the connection" << std::endl;
//                 stop_thread = true;
//             } 
//             else 
//             {
//                 perror("recv failed");
//                 stop_thread = true;
//             }
//         }
//     }

//     void processMessages() 
//     {
//         std::unique_lock<std::mutex> lock(mutex);
//         while (!messages.empty()) 
//         {
//             std::string message = messages.front();
//             messages.pop();
//             lock.unlock();

//             // Process the message
//             try {
//                 nlohmann::json j = nlohmann::json::parse(message);
//                 std::string request = j[REQUEST];
//                 if (request == "MESSAGE") 
//                 {
//                     std::string sender = j["sender"];
//                     std::string content = j["content"];
//                     std::cout << "Message from " << sender << ": " << content << std::endl;
//                 } 
//                 else 
//                 {
//                     // Handle other types of messages
//                 }
//             } 
//             catch (const std::exception& e) 
//             {
//                 std::cerr << "Failed to parse message: " << e.what() << std::endl;
//             }

//             lock.lock();
//         }
//     }

private:
    Users m_users;
    int m_socket_fd;
    std::string server_addr;

    int port;
    std::string ip;

    // std::thread receive_thread;
    // std::atomic<bool> stop_thread;
    // std::mutex mutex;
    // std::condition_variable cond_var;
    // std::queue<std::string> messages;
};