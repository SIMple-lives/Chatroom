#include "../head/head.hpp"
#include "../head/define.hpp"
#include "../cli/user_work.hpp"

class File 
{
public:
    File() = default ;
    File(int fd,std::string id,int port,std::string ip)
    {
        m_id = id;
        m_fd = fd;
        m_port = port;
        m_ip = ip;
    }
    void Menu();
    void Run();
    void Send_file();
    void Receive_file();
    void Message_file();
    void get_info();
    std::string getCurrentTime() ;
private:
    std::string m_id;
    int m_fd;
    int m_port;
    std::string m_ip;

    std::thread file_thread;
    std::mutex m_mutex;

    void m_start(std::string path,std::string id)
    {
        this->file_thread = std::thread(&File::do_File,this,path,id);
    }
    void do_File(std::string path,std::string id)
    {
        int newfd;
            newfd = socket(AF_INET, SOCK_STREAM, 0);
            if (newfd == -1) 
            {
                perror("Socket creation failed");
                throw std::runtime_error("Socket creation failed");
            }
            struct sockaddr_in server;
            memset(&server, 0, sizeof(server));
            server.sin_family = AF_INET;
            server.sin_port = htons(this->m_port);
            if (inet_pton(AF_INET, this->m_ip.c_str(), &server.sin_addr) <= 0) 
            {
                perror("Invalid socket address/Address not supported");
                throw std::invalid_argument("Invalid server address");
            }
            if (connect(newfd, (struct sockaddr*)&server, sizeof(server)) == -1) 
            {
                perror("Connection failed");
                throw std::runtime_error("Connection failed");
            }
            std::cout << "Connected to server " <<this->m_ip << ":" << this->m_port << std::endl;
            std::filesystem::path filePath(path);
            if (!std::filesystem::exists(filePath)) 
            {
                std::cout << "File does not exist" << std::endl;
                return;
            }
            if (!std::filesystem::is_regular_file(filePath)) 
            {
                std::cout << "This is not a regular file" << std::endl;
                return;
            }

        int file = open(path.c_str(), O_RDONLY );
        if (file == -1) 
        {
            std::cerr << "Failed to open file" << std::endl;
            return;
        }
                std::string time =getCurrentTime();
                size_t file_size = std::filesystem::file_size(filePath);
                std::string file_name = filePath.filename().string();
            // Send initial file info
            nlohmann::json js = {
                {"id", m_id},
                {"file_name", file_name},
                {"file_size", file_size},
                {"file_path", filePath.string()},
                {"friend_id", id},
                {REQUEST, SENDFILE},
                {"time",time}
            };
            
            Sen sender;
            sender.send_cil(newfd, js.dump());

            const size_t BUFFER_SIZE = 1310720;
            char buffer[BUFFER_SIZE];
            ssize_t bytes_sent;
            off_t offset = 0;
            size_t total_sent = 0;

        while (total_sent < file_size)
        {
                ssize_t bytes_to_send = std::min(BUFFER_SIZE, file_size - total_sent);
                bytes_sent = sendfile(newfd, file, &offset, bytes_to_send);

                if (bytes_sent <= 0) 
                {
                    std::cerr << "sendfile error" << std::endl;
                    break;
                }

                total_sent += bytes_sent;
                // float progress = (float)total_sent / file_size * 100;
                // std::cout << "Progress: " << progress << "%" << std::endl;
                
            }
            Rec r;
            int status = r.recv_ok(newfd);
            if(status == SUCCESS)
            {   
                std::cout << "File sent successfully!" << std::endl;
            }
            else {
                std::cerr << "File sending failed!" << std::endl;
            }
            close(file);
    }
};