#include "../cil/file.hpp"
#include <cstddef>
#include <iostream>
#include <sys/types.h>



void File::Menu()
{
    std::cout << "____________________________________________________________________" << std::endl;
    std::cout << "\033[34m\t ________      _____        __             ______\033[0m" << std::endl;
    std::cout << "\033[31m\t/\\   ____\\     \\_  _\\      /\\ \\           /\\  ___\\ \033[0m" << std::endl;
    std::cout << "\033[32m\t\\ \\  ___\\        \\  \\      \\ \\ \\_____     \\ \\  __\\ \033[0m" << std::endl;
    std::cout << "\033[33m\t \\ \\             _\\  \\_     \\ \\______\\     \\ \\_____\\\033[0m" << std::endl;
    std::cout << "\033[34m\t  \\/             \\_____\\     \\/______/      \\/_____/\033[0m" << std::endl;
    std::cout << "____________________________________________________________________" << std::endl;
    std::cout << "\033[47;33m                              1.Send                               \033[0m"  << std::endl;
    std::cout << "\033[47;35m                              2.Receive                            \033[0m"  << std::endl;
    std::cout << "\033[47;31m                              3.Message                            \033[0m"  << std::endl;
    std::cout << "\033[47;34m                              4.Exit                               \033[0m"  << std::endl;
    std::cout << "____________________________________________________________________" << std::endl;
    std::cout << "Enter your choice: ";
}

void File::Run()
{
    int choice = 1 ;
    std::string ch;
    do{
        Menu();
        std::cin >> ch;
        if(ch == "1")
        {
           this->Send_file();
        }
        else if(ch == "2")
        {
            //Receive();
            Receive_file();
        }
        else if(ch == "3")
        {
            Message_file();
        }
        else if(ch == "4")
        {
            choice = 0;
            std::cout << "Exiting..." << std::endl;
        }
        else
        {
            std::cout << "Invalid choice!" << std::endl;
        }
    }while(choice);
}

void File::get_info()
{
     nlohmann::json j = {{REQUEST,VIEWFRIENDS},
        {"id",this->m_id}};
        Sen s;
        s.send_cil(this->m_fd,j.dump());
        Rec r;
        int status = r.recv_ok(this->m_fd); 
        std::vector<std::string> online_friends;
        std::vector<std::string> offline_friends;
        std::string str;
        r.recv_cil(this->m_fd,str);
        nlohmann::json friends = nlohmann::json::parse(str);
        online_friends = friends["online"];
        offline_friends = friends["offline"];
        if(online_friends.size()!=0)
        {
            system("echo \"Online Friends\" | figlet | boxes -d c | lolcat");
            //分割线
             std::cout << "----------------------------------------" << std::endl; 
        }
        for(auto i:online_friends)
        {
            std::cout << i << std::endl;
        }
        if(offline_friends.size()!=0)
        {
            system("echo \"Offline Friends\" | figlet | boxes -d c | lolcat");
            //分割线
             std::cout << "----------------------------------------" << std::endl;
        }
        for(auto i:offline_friends)
        {
            std::cout << i << std::endl;
        }
        if(online_friends.size()!=0 || offline_friends.size()!=0)
        {
            std::cout << "----------------------------------------" << std::endl;
        }
        nlohmann::json List_group_request ={
        {REQUEST,LISTGROUP},
        {"id",this->m_id}
    };
   
    s.send_cil(this->m_fd,List_group_request.dump());
    status = r.recv_ok(this->m_fd);
    if(status == NOGROUP)
    {
        std::cout << "无群聊" << std::endl;
    }
    else if(status == SUCCESS)
    {
        std::vector<std::string > join;
        std::vector<std::string > j_owners;
        std::vector<std::string > nojoin;
        std::vector<std::string > no_owners;
        std::vector<std::string > creat;
        std::string get_info ;
        r.recv_cil(this->m_fd,get_info);
        nlohmann::json get_info_json = nlohmann::json::parse(get_info);
        join = get_info_json["join"];
        j_owners = get_info_json["join_owner"];
        // nojoin = get_info_json["nojoin"];
        // no_owners = get_info_json["nojoin_owner"];
        creat = get_info_json["creat"];
        if(creat.size()!=0)
        {
            system("echo \"CREATE\" | figlet | boxes -d c | lolcat");
        }
        for(auto i : creat)
        {
            std::cout << i << std::endl;
        }
        if(join.size()!=0)
        {
            system("echo \"JOIN\" | figlet | boxes -d c | lolcat");
        }
        for(int i =0;i<join.size();i++)
        {
            std::cout << join[i]<< std::endl;
            std::cout << j_owners[i] << std::endl;
        }
        // if(nojoin.size()!=0)
        // {
        //     system("echo \"NOJOIN\" | figlet | boxes -d c | lolcat");
        // }
        // for(int i=0;i<nojoin.size();i++)
        // {
        //     std::cout << nojoin[i] << std::endl;
        //     std::cout << no_owners[i] << std::endl;
        // }
    }
}

std::string File::getCurrentTime() 
{
        auto now = std::chrono::system_clock::now();
        std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
        
        std::tm now_tm = *std::localtime(&now_time_t);
        
        std::ostringstream oss;
        oss << std::put_time(&now_tm, "%Y-%m-%d+%H:%M:%S");
        return oss.str();
}


void File::Send_file()
{   

    this->get_info();
    int status ;
    Sen s;
    Rec r;
    std::cout << "请输入要发送的对象" << std::endl;
    std::string  a;
    std::cout << "1.私聊" << std::endl;
    std::cout << "2.群聊" << std::endl;
    std::cin >> a;
    std::string send_id;
    if(a=="1")
    {
        std::cout <<"Enter the id :" << std::endl;
        std::cin >> send_id;
        nlohmann::json js = {
            {"id",this->m_id},
            {"friend_id",send_id},
            {REQUEST,IFFRIEND}
        };
        s.send_cil(this->m_fd,js.dump());
    }
    else if(a=="2")
    {
        std::cout <<"Enter the id :" << std::endl;
        std::cin >> send_id;
        nlohmann::json js = {
            {"id",this->m_id},
            {"friend_id",send_id},
            {REQUEST,IFINGROUP}
        };
        s.send_cil(this->m_fd,js.dump());
    }
    else  
    {
        std::cout << "输入错误" << std::endl;
        return ;
    }
    status = r.recv_ok(this->m_fd);
    if(status == BLOCK || status == NOTSPEAK)
    {
        std::cout << "你被禁言了或屏蔽了" << std::endl;
        return;
    }
    else if(status == NOFRIEND)
    {
        std::cout << "你不是他的好友" << std::endl;
        return;
    }
    else if(status == NOTINGROUP) 
    {
        std::cout << "你不在该群聊" << std::endl;
        return;
    }
    else if(status ==SUCCESS)
    {
        std::cout << "请输入文件路径" << std::endl;
    }
    else
    {
    
    }
    std::string path;
    std::cout << "Enter the file path: "<< std::endl;
    std::cin >> path;
    this->m_start(path,send_id);
    this->file_thread.detach();
}



void File::Receive_file()
{
    std::cout << "请输入你要接收的文件名称" << std::endl;
    std::string file_name;
    std::cin >> file_name;
    nlohmann::json js = {
        {"id", this->m_id},
        {"file_name", file_name},
        {REQUEST, RECEIVEFILE}
    };
    Sen s;
    s.send_cil(this->m_fd, js.dump());
    Rec r;
    int status = r.recv_ok(this->m_fd);
    if (status == FILENOTEXISTS)
    {
        std::cout << "File not exists!" << std::endl;
        return;
    }
    else if (status == SUCCESS)
    {
        size_t status = r.recv_ok_Long(this->m_fd);
        std::cout << "文件大小为 " << status << std::endl;
        std::filesystem::path dir = "../test1/";
        if (!std::filesystem::exists(dir))
        {
            try
            {
                std::filesystem::create_directories(dir);
            }
            catch (const std::filesystem::filesystem_error& e)
            {
                std::cerr << "Failed to create directories: " << e.what() << std::endl;
                return;
            }
        }

        std::string creatFile = dir.string() + file_name;
        FILE* fp = fopen(creatFile.c_str(), "wb");
        if (fp == NULL)
        {
            std::cerr << "Failed to open file for writing" << std::endl;
            return;
        }

        int original_flags = fcntl(this->m_fd, F_GETFL, 0);
        if(original_flags == -1)
        {
            std::cout << "Failed to get file descriptor flags" << std::endl;
            Sen s;
            s.send_ok(this->m_fd,FILENOTEXISTS);
            return ;
        }
        if(fcntl(this->m_fd,F_SETFL,original_flags &O_NONBLOCK) == -1)
        {
            std::cout << "Failed to set file descriptor to blocking mode" << std::endl;
            Sen s;
            s.send_ok(this->m_fd,FAILURE);
            return ;
        }

        int len;
        // const size_t BUFFER_SIZE = 4096;
        char buffer[1310720];
        off_t total_received = 0;

        try
        {
            while (total_received < status)
            {
                len = recv(this->m_fd, buffer, sizeof(buffer), 0);
                if (len <= 0)
                {
                    if (len < 0)
                    {
                        perror("recv");
                    }
                    fclose(fp);
                    return;
                }

                fwrite(buffer, 1, len, fp);
                total_received += len;

                // float progress = static_cast<float>(total_received) / status * 100;
                // std::cout << "Progress: " << progress << "%" << std::endl;
            }
            if(fcntl(this->m_fd,F_SETFL,original_flags) == -1)
            {
                std::cout << "Failed to set file descriptor to blocking mode" << std::endl;
            }
        }
        catch (...)
        {
            std::cerr << "An error occurred during file reception" << std::endl;
            fclose(fp);
            return;
        }

        fclose(fp);
    }
    else
    {
        // Handle other status codes if needed
    }
}


void File::Message_file()
{
    nlohmann::json js ={
        {"id",this->m_id},
        {REQUEST,MESSAGEFILE}
    };
    Sen s;
    s.send_cil(this->m_fd, js.dump());
    Rec r;
    int status = r.recv_ok(this->m_fd);
    if(status == SUCCESS)
    {
        std::vector<std::string> ids;
        std::vector<std::string> files;
        std::string signs ;
        r.recv_cil(this->m_fd, signs);
        // std::cout << signs << std::endl;
        nlohmann::json js = nlohmann::json::parse(signs);
        ids = js["ids"];
        files = js["files"];
        if(ids.size() != 0)
        {
            std::cout << "You have " << ids.size() << " new file messages!" << std::endl;
            std::cout << "--------------------------------" << std::endl;
        }
        for(int i = 0; i < ids.size();i++)
        {   
            nlohmann::json js = nlohmann::json::parse(files[i]);
            std::string file_name = js["file_name"];
            std::string time  = js["time"];
            std::string friend_id = js["friend"];
            size_t size = js["file_size"];
            std::cout << friend_id << " " << time << ":" << std::endl;
            std::cout << "File Name: " << file_name << std::endl;
            std::cout << "File Size: " << size << " bytes" << std::endl;
            std::cout << "--------------------------------" << std::endl;
        }
    }
    else if(status == NOFILES)
    {
        std::cout << "No Filse!" << std::endl;
    }
    else 
    {
    
    }
}