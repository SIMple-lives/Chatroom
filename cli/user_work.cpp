#include "../cli/user_work.hpp"


void User_work::Quit()
{
    Sen s;
    nlohmann::json j = {{REQUEST,USEREXIT},
        {"id",this->m_id}};
    s.send_cil(m_fd, j.dump());
    Rec r;
    int status=r.recv_ok(this->m_fd);
    if(status == SUCCESS)
    {
        system("echo \"QUIT SUCCESS\" | figlet | boxes -d c | lolcat");
    }
}

void User_work::AddFriend()
{
    this->showFriend();
    std::cout << "Enter the id of the friend you want to add: ";
    std::string id;
    std::cin >> id;
    std::cout << "Enter the message: ";
    std::string msg;
    std::cin >> msg;
    Sen s;
    std::string time = getCurrentTime();
    nlohmann::json add_request = {{REQUEST,ADDFRIEND},
    {"friend_id",id},
    {"id",this->m_id},
    {"msg",msg},
    {"time",time}
    };
    s.send_cil(this->m_fd, add_request.dump());
    Rec r;
    int status=r.recv_ok(this->m_fd);
    if(status == SUCCESS)
    {
        std::cout << "Add friend request sent successfully" << std::endl;
    }
    else if(status == NEXIST)
    {
        std::cout << "不存在该用户" << std::endl;
    }
    else if(status == ISFRIEND)
    {
        std::cout << "已经是好友" << std::endl;
    }
    else if(status == ISME)
    {
        std::cout << "不能添加自己" << std::endl;
    }
    else
    {
        std::cout << "Add friend request failed" << std::endl;
    }
}

void User_work::ViewFriendRequest()
{
    nlohmann::json js = {{"id",this->m_id},
    {REQUEST,VIEWFRIENDSREQUSET},
    };
    Sen s;
    s.send_cil(this->m_fd,js.dump());
    Rec r;
    int status = r.recv_ok(this->m_fd);
    if(status == SUCCESS)
    {
        std::string str;
        r.recv_cil(this->m_fd,str);
        //std::cout << str << std::endl;
        nlohmann::json userinfo = nlohmann::json::parse(str);
        std::vector<std::string > friend_id;
        std::vector<std::string> friend_msg;
        friend_id=userinfo["id"];
        friend_msg=userinfo["msg"];
        
        
        this->handleFriendRequset(friend_id,friend_msg);
    }
    else if(status == NOFRIENDREQUSET) 
    {
        std::cout << "No friend request" << std::endl;
    }
    else
    {

    }
}

void User_work::handleFriendRequset(std::vector<std::string> &id,std::vector<std::string> &mag)
{
    int size = id.size();
    system("echo \"handle friend request\" | figlet | boxes -d c | lolcat");
    std::cout << "----------------------------------------------------------------------------" << std::endl;
    for(int i = 0; i < size; i++)
    {
        std::cout <<"第 " << i+1 << "申请 : " << id[i] << ": " <<  std::endl;
        std::string ret = mag[i];
        nlohmann::json js = nlohmann::json::parse(ret);
        std::string time = js["time"];
        std::string msg = js["msg"];
        std::cout << time << ":  " << msg << std::endl;
        std::cout << "----------------------------------------------------------------------------" << std::endl;
        std::cout << "Enter 1 to accept, Enter 2 to reject, Enter 3 to ignore: ";
        Sen s;
        std::cout << "Enter your choice: ";
        std::string  choice ;
        std::cin >> choice;
        int ch;
        if(choice == "1")
        {
            ch =ACCEPT;
        }
        else if(choice == "2")
        {
            ch =REFUSE;
        }
        else 
        {
            ch =IGNORE;
        }
        nlohmann::json js1 = {{"id",this->m_id},
        {REQUEST,HANDLEFRIENDREQUSET},  
        {"friend_id",id[i]},
        {"choice",ch}};
        s.send_cil(this->m_fd,js1.dump());
    }
}

void User_work::PrivateChat()
{
    this->m_running = false;
    if(this->refreshThread.joinable())
    {
        std::cout << "正在关闭" << std::endl;
        this->refreshThread.join();
    }
    std::cout << "Enter the id of the friend you want to chat: ";
    std::string id;
    std::cin >> id;
    nlohmann::json js = {{"id",this->m_id},
    {REQUEST,PRIVATECHAT},
    {"friend_id",id}};
    Sen s;
    s.send_cil(this->m_fd,js.dump());
    
    Rec r;
    int status;
    status = r.recv_ok(this->m_fd);
    if(status == NOFRIEND)
    {
        std::cout << "该用户不是你的好友" << std::endl;
        std::cout << "请添加该好友 :" << id << std::endl;
    }
    else if(status == SUCCESS)
    {
        Prich p(this->m_fd,id,this->m_id);
        p.run();
    }
    else if(status == BLOCK)
    {
        std::cout << "你屏蔽了该用户 " << std::endl;
        std::cout << "是否取消屏蔽?  y/n" << std::endl;
        std::string choice;
        std::cin >> choice;
        if(choice == "y")
        {
            this->UnBlockFriend(id);
        }
    }
    this->m_running = true;
    this->start();
}

void User_work::UnBlockFriend(std::string  &friend_id)
{   
    nlohmann::json js = {
        {"id",this->m_id},
        {"friend_id",friend_id},
        {REQUEST,UNBLOCKFRIEND}
    };
    Sen s;
    s.send_cil(this->m_fd, js.dump());
    Rec r;
    int status ;
    status = r.recv_ok(this->m_fd);
    if(status == SUCCESS)
    {
        std::cout << "取消屏蔽成功" << std::endl;
    }
    else if(status == FAILURE)
    {
        std::cout << "取消屏蔽失败" << std::endl;
    }
    else {
    
    }
}

void User_work::GroupChat()
{
    std::string ch;
    int choice ;
    do
    {
        this->showGroups();
        std::cout << "Enter your choice: ";
        std::cin >> ch;
        if(ch =="1")
        {
            Creat_Group(this->m_id,this->m_fd);
        }
        else if(ch =="2")
        {
            Join_Group(this->m_id, this->m_fd);
        }
        else if(ch =="3")
        {
            Quit_Group(this->m_id, this->m_fd);
        }
        else if(ch =="4")
        {
            List_Group(this->m_id, this->m_fd);
        }
        else if(ch =="5")
        {
            Dele_Group(this->m_id, this->m_fd);
        }
        else if(ch =="6")
        {
            Member_Group(this->m_id,this->m_fd);
        }
        else if(ch =="7")
        {
            Chat_Group(this->m_id, this->m_fd);
        }
        else if(ch =="8")
        {
            choice =0 ;
        }
        else if(ch == "9" )
        {
            Handle_Group_Requset(this->m_id, this->m_fd);
        }
        else if(ch == "10")
        {
            Manage_Group(this->m_id, this->m_fd);
        }
        else 
        {
            std::cout << "Invalid choice" << std::endl;
            std::cout << "Do you want to continue? y/n" << std::endl;
            std::string ok;
            std::cin >> ok;
            if(ok=="y")
            {
                choice = 0;
            }
        }
    }while(choice);
}

void User_work::ViewBlockedList()
{
    system("echo \"VIEW BLOCKED LIST\" | figlet | boxes -d c | lolcat");
    nlohmann::json j = {{REQUEST,VIEWBLOCKFRIENDS},
        {"id",this->m_id}
    };
    Sen s;
    s.send_cil(this->m_fd, j.dump());
    Rec r;
    int status;
    status = r.recv_ok(this->m_fd);
    if(status == NOBLOCKFRIEND)
    {
        std::cout << "没有屏蔽的好友" << std::endl;
        std::cout << "please block a friend" << std::endl;
    }
    else if(status == SUCCESS)
    {
        std::string str;
        r.recv_cil(this->m_fd,str);
        nlohmann::json userinfo = nlohmann::json::parse(str);
        std::vector<std::string> block_friends = userinfo["block"];
        std::cout << "----------------------------------------------------------------------------" << std::endl;
        for(auto &i : block_friends)
        {
            std::cout << i << std::endl;
        }
        std::cout << "----------------------------------------------------------------------------" << std::endl;
    }
    else 
    {
        std::cout << "view blocked list failed " << std::endl;
    }
}

void User_work::ViewFriendInfo()
{
    std::cout << "Enter the id of the friend you want to view: ";
    std::string id;
    std::cin >> id;
    Sen s;
    nlohmann::json j = {{REQUEST,FRIENDINFO},
        {"id",this->m_id},
        {"friend_id",id}};
    s.send_cil(this->m_fd, j.dump());
    Rec r;
    int status=r.recv_ok(this->m_fd);
    if(status == SUCCESS)
    {
        std::string str;
        r.recv_cil(this->m_fd,str);
        nlohmann::json userinfo = nlohmann::json::parse(str);
        this->show_friend(id,userinfo["username"],userinfo["telephone"],userinfo["status"]);
    }
    else if(status == NEXIST)
    {
        std::cout << "不存在该用户" << std::endl;
    }
    else if(status == ISNFRIEND)
    {
        std::cout << "不是好友" << std::endl;
    }
    else
    {
        std::cout << "View friend info failed " << std::endl;
    }
}

void User_work::ViewPersonalInfo()
{
    nlohmann::json j = {{REQUEST,PERSONALINFO},
        {"id",this->m_id}};
    Sen s;
    s.send_cil(this->m_fd, j.dump());
    Rec r;
    int status=r.recv_ok(this->m_fd);
    if(status == SUCCESS)
    {
        std::string str;
        r.recv_cil(this->m_fd,str);
        nlohmann::json userinfo = nlohmann::json::parse(str);
        this->show_Personalinfo(this->m_id,userinfo["username"],userinfo["telephone"],userinfo["status"]);
    }
    else 
    {
        std::cout << "View personal info failed " << std::endl;
    }
}

void User_work::ViewFriends()
{
    nlohmann::json j = {{REQUEST,VIEWFRIENDS},
    {"id",this->m_id}};
    Sen s;
    s.send_cil(this->m_fd,j.dump());
    Rec r;
    int status = r.recv_ok(this->m_fd); 
    if(status == NOFRIEND)
    {
        std::cout << "You have no friends" << std::endl;
        std::cout << "please add some friends" << std::endl;
    }
    else if(status == SUCCESS)
    {
        std::vector<std::string> online_friends;
        std::vector<std::string> offline_friends;
        std::string str;
        r.recv_cil(this->m_fd,str);
        nlohmann::json friends = nlohmann::json::parse(str);
        online_friends = friends["online"];
        offline_friends = friends["offline"];
        system("echo \"Online Friends\" | figlet | boxes -d c | lolcat");
        //分割线
        std::cout << "----------------------------------------" << std::endl; 
        for(auto i:online_friends)
        {
            std::cout << i << std::endl;
        }
        std::cout << "----------------------------------------" << std::endl;
        system("echo \"Offline Friends\" | figlet | boxes -d c | lolcat");
        for(auto i:offline_friends)
        {
            std::cout << i << std::endl;
        }
        std::cout << "----------------------------------------" << std::endl;
    }
    else
    {
        std::cout << "View friends failed" << std::endl;
    }
}

void User_work::BlockFriend()
{
    system("echo \"Block Friend\" | figlet | boxes -d c | lolcat");
    std::cout << "Enter the id of the friend you want to block: ";
    std::string id;
    std::cin >> id;
    nlohmann::json blog_requset = {
        {REQUEST,BLOCKFRIEND},
        {"id",this->m_id},
        {"friend_id",id}
    };
    Sen s;
    s.send_cil(this->m_fd,blog_requset.dump());
    Rec r;
    int status = r.recv_ok(this->m_fd);
    if(status == SUCCESS)
    {
        std::cout << "Blocked successfully" << std::endl;
    }
    else if(status == NEXIST)
    {
        std::cout << "不存在该用户" << std::endl;
    }
    else if(status == ISNFRIEND)
    {
        std::cout << "不是好友" << std::endl;
    }
    else if(status == ISBLOCKFRIEND)
    {
        std::cout << "已经是黑名单" << std::endl;
    }
    else
    {
        std::cout << "Block friend failed" << std::endl;
    }
}

void User_work::DeleteFriend()
{
    system("echo \"Delete Friend\" | figlet | boxes -d c | lolcat");
    std::cout << "Enter the id of the friend you want to delete: ";
    std::string id ;
    std::cin >> id;
    nlohmann::json dele_request = {
        {REQUEST,DELEFRIEND},
        {"id",this->m_id},
        {"friend_id",id}
    };
    Sen s;
    s.send_cil(this->m_fd,dele_request.dump());
    Rec r;
    int status = r.recv_ok(this->m_fd);
    if(status == NEXIST)
    {
        std::cout << "不存在该用户" << std::endl;
    }
    else if(status == ISNFRIEND)
    {
        std::cout << "不是好友" << std::endl;
    }
    else if(status == SUCCESS)
    {
        std::cout << "Delete friend successfully" << std::endl;
    }
    else
    {
        std::cout << "Delete friend failed" << std::endl;
    }
}

void User_work::Refresh()
{
    nlohmann::json Refresh_request ={
        {"id",this->m_id},
        {REQUEST,REFRESH}
    };
    Sen s;
    s.send_cil(this->m_fd,Refresh_request.dump());
    Rec r;
    int status ;
    status = r.recv_ok(this->m_fd);
    if(status == NOSIGN)
    {
        
    }
    else if(status == SUCCESS)
    {
        std::string sign;
        r.recv_cil(this->m_fd,sign);
        // std::cout << sign << std::endl;
        nlohmann::json js = nlohmann::json::parse(sign);
        std::vector<std::string> msg = js["send"];
        std::vector<std::string> Gmsg = js["Gsend"];
        std::vector<std::string> Smsg = js["Ssend"];
        std::vector<std::string> Fmsg = js["Fr"];
        std::vector<std::string> F = js["FO"];
        int length = js["Length"];
        if(length != 0)
        {
            std::cout << "你有 " << length << " 个文件可以接收" << std::endl;
        }
        for(auto i:msg)
        {
            std::cout << i << std::endl;
        }
        for(auto i:Gmsg)
        {
            std::cout << i << std::endl;
        }
        for(auto i:Smsg)
        {
            std::cout << i << std::endl;
        }
        for(auto i:Fmsg)
        {
            std::cout << i << std::endl;
        }
        for(auto i:F)
        {
            std::cout << i << std::endl;
        }
    }
}

void User_work::show_Personalinfo(std::string id,std::string name ,std::string telephone,int status)
{
        system("echo \"Personal INFO\" | figlet | boxes -d c | lolcat");
        std::string info = name + " 's Information ";
        printCentered(info, 68);
        std::cout << std::setw(68) << std::setfill('-') << '-' << std::endl; // 横线分隔
        std::string id1 = "ID: " + id;
        std::string name1 = "Name: " + name;
        std::string telephone1 = "Telephone: " + telephone;
        std::string strstatus;
        if(status == ONLINE)
        {
            strstatus = "Online";
        }
        else if(status == OFFLINE)
        {
            strstatus = "Offline";
        }
        else
        {
            strstatus = "Unknown";
        } 
        std::string status1 = "Status: " + strstatus;
        printCentered(id1, 68);
        printCentered(name1, 68);
        printCentered(telephone1, 68);
        printCentered(status1, 68);
        std::cout << std::setw(68) << std::setfill('-') << '-' << std::endl;
}

void User_work::Creat_Group(std::string id,int fd)
{
    std::cout << "请输入群组名称" << std::endl;
    std::string name;
    std::cin >> name;
    nlohmann::json Creat_Group_request = {
        {"id",id},
        {REQUEST,CREATEGROUP},
        {"Group_name",name}
    };
    Sen s;
    s.send_cil(fd,Creat_Group_request.dump());
    Rec r;
    int status;
    status = r.recv_ok(fd);
    if(status == GROUPNAMEEXIST)
    {
        std::cout << "群名称已存在" << std::endl;
    }
    else if(status == SUCCESS)
    {
        std::string group_id;
        r.recv_cil(fd,group_id);
        std::cout << "创建群组成功,群组ID为" << group_id << std::endl;
        std::string  ok;
        do{
            std::cout << "是否记住创建信息" << std::endl;
            std::cout << "群组名称 \033[31m" << name << "\033[0m" << std::endl;
            std::cout << "群组ID \033[31m" << group_id << "\033[0m" << std::endl;
            std::cout << "Are You Remember ?(y/n)" << std::endl;
            std::cin >> ok;
        }while(ok!="y");
    }
    else if(status == FAILURE)
    {
        std::cout << "创建群组失败 " << std::endl;
    }
    else
    {
        std::cout << "创建群组失败" << std::endl;
    }
    return ;
}

void User_work::Join_Group(std::string id,int fd)
{
    std::cout << "请输入群组id" << std::endl;
    std::string group_id;
    std::cin >> group_id;
    std::cout << "请输入你的加入宣言" << std::endl;
    std::string msg;
    std::cin >> msg;
    std::string time;
    time = getCurrentTime();
    nlohmann::json Join_Group_request = {
        {"id",id},
        {REQUEST,JOINGROUP},
        {"Group_id",group_id},
        {"msg",msg},
        {"time",time}
    };
    Sen s;
    s.send_cil(fd,Join_Group_request.dump());
    Rec r;
    int status ;
    status = r.recv_ok(fd);
    if(status == GROUPIDNOTEXIST)
    {
        std::cout << "群组ID不存在" << std::endl;
        std::cout << "请重新输入" << std::endl;
    }
    else if(status == JOINOWNGROUP)
    {
        std::cout << "不能加入自己创建的群组" << std::endl;
        std::cout << "请重新输入" << std::endl;
    }
    else if(status == SUCCESS)
    {
        std::cout << "已发送群聊申请" << std::endl;
    }
    else if(status == ALREADYJOIN)
    {
        std::cout << "已加入该群组" << std::endl;
    }
    else 
    {
    
    }
}

void User_work::Quit_Group(std::string id,int fd)
{
    std::cout << "请输入群组id" << std::endl;
    std::string group_id;
    std::cin >> group_id;
    nlohmann::json Quit_Group_request = {
        {"id",id},
        {REQUEST,QUITGROUP},
        {"Group_id",group_id}
    };
    Sen s;
    s.send_cil(fd,Quit_Group_request.dump());
    Rec r;
    int status;
    status = r.recv_ok(fd);
    if(status == GROUPIDNOTEXIST)
    {
        std::cout << "群组ID不存在" << std::endl;
        std::cout << "请重新输入" << std::endl;
    }
    else if(status == NOTINGROUP)
    {
        std::cout << "你不在该群聊 " << std::endl;
        std::cout << "请重新输入" << std::endl;
    }
    else if(status == NOTQUIT)
    {
        std::cout << "你是群主，不能退出该群聊" << std::endl;
        std::cout << "请重新输入" << std::endl;
    }
    else if(status == SUCCESS) 
    {
        std::cout << "成功退出群聊" << std::endl;
    }
    else {
    
    }
}

void User_work::Handle_Group_Requset(std::string id,int fd)
{
    nlohmann::json handle_request = {
        {REQUEST,HANDLEGROUPREQUEST},
        {"id",id}
    };
    Sen s;
    s.send_cil(fd,handle_request.dump());
    Rec r;
    int status;
    status = r.recv_ok(fd);
    if(status == SUCCESS)
    {
        std::vector<std::string > ids;
        std::vector<std::string > msgs;
        std::string get_info ;
        r.recv_cil(fd,get_info);
        nlohmann::json get_info_json = nlohmann::json::parse(get_info);
        ids = get_info_json["ids"];
        msgs = get_info_json["msgs"];
        for(int i =0;i<ids.size();i++)
        {
            std::cout << ids[i] << std::endl;
            std::cout << msgs[i] << std::endl;
            std::cout << "是否同意(y/n)" << std::endl;
            std::string agree;
            std::cin >> agree;
            if(agree == "y")
            {
                nlohmann::json agree = nlohmann::json::parse(msgs[i]);
                nlohmann::json agree_request = {
                    {"id_o",id},
                    {"id",ids[i]},
                    {"Group_id",agree["group_id"]},
                    {"msg","agree"},
                    {REQUEST,DOGROUPREQUEST}
                };
                s.send_cil(fd,agree_request.dump());
                Rec r;
                int status;
                status = r.recv_ok(fd);
                if(status == SUCCESS)
                {
                    std::cout << "同意成功" << std::endl;
                }
                else 
                {
                    std::cout << "同意失败" << std::endl;
                }
            }
            else 
            {
                nlohmann::json agree = nlohmann::json::parse(msgs[i]);
                nlohmann::json agree_request = {
                    {"id_o",id},
                    {"id",ids[i]},
                    {"Group_id",agree["group_id"]},
                    {"msg","noagree"},
                    {REQUEST,DOGROUPREQUEST}
                };
                s.send_cil(fd,agree_request.dump());
                Rec r;
                int status;
                status = r.recv_ok(fd);
                if(status == SUCCESS)
                {
                    std::cout << "拒绝成功" << std::endl;
                }
                else 
                {
                    std::cout << "拒绝失败" << std::endl;
                }
            }
        }
    }
    else 
    {
        
    }
}

void User_work::Chat_Group(std::string id,int fd)
{
    this->m_running = false;
    if(this->refreshThread.joinable())
    {
        std::cout << "正在关闭" << std::endl;
        this->refreshThread.join();
    }
    std::string group_id;
    std::cout << "请输入群聊id" << std::endl;
    std::cin >> group_id;
    nlohmann::json chat_group_request = {
        {"id",id},
        {"group_id",group_id},
        {REQUEST,CHATGROUP}
    };
    Sen s;
    s.send_cil(fd,chat_group_request.dump());
    Rec r;
    int status ;
    status = r.recv_ok(fd);
    if(status ==SUCCESS)
    {
        Prich p(fd,group_id,id);
        p.run_Group();
    }
    else if(status == GROUPIDNOTEXIST)
    {
        std::cout << "群聊不存在" << std::endl;
    }
    else if(status == NOTINGROUP)
    {
        std::cout << "未加入群聊" << std::endl;
    }
    else 
    {
    
    }
    this->m_running = true;
    this->start();
}

void User_work::List_Group(std::string id,int fd)
{
    nlohmann::json List_group_request ={
        {REQUEST,LISTGROUP},
        {"id",id}
    };
    Sen s;
    s.send_cil(fd,List_group_request.dump());
    Rec r;
    int status;
    status = r.recv_ok(fd);
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
        r.recv_cil(fd,get_info);
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
    }
}

void User_work::Dele_Group(std::string id,int fd)
{
    std::cout << "请输入群组id" << std::endl;
    std::string group_id;
    std::cin >> group_id;
    nlohmann::json Dele_group_request = {
        {"id",id},
        {REQUEST,DELEGROUP},
        {"Group_id",group_id}
    };
    Sen s;
    s.send_cil(fd,Dele_group_request.dump());
    Rec r;
    int status;
    status = r.recv_ok(fd);
    if(status == GROUPIDNOTEXIST)
    {
        std::cout << "群组不存在" << std::endl;
        std::cout << "请重新输入" << std::endl;
    }
    else if(status == NOTINGROUP)
    {
        std::cout << "你不在该群聊" << std::endl;
        std::cout << "请重新输入" << std::endl;
    }
    else if(status == NOTGROUPOWNER)
    {
        std::cout << "你不是群主，不能解散该群聊" << std::endl;
        std::cout << "请重新输入" << std::endl;
    }
    else if(status == SUCCESS)
    {
        std::cout << "成功解散群聊" << std::endl;
    }
    else {
    
    }
}

void User_work::Member_Group(std::string id,int fd)
{
    std::cout << "请输入群组id" << std::endl;
    std::string group_id;
    std::cin >> group_id;
    nlohmann::json Member_Group_request = {
        {"id",id},
        {REQUEST,MEMBERGROUP},
        {"Group_id",group_id}
    };
    Sen s;
    s.send_cil(fd,Member_Group_request.dump());
    Rec r;
    int status;
    status = r.recv_ok(fd);
    if(status == GROUPIDNOTEXIST)
    {
        std::cout << "群组ID不存在" << std::endl;
        std::cout << "请重新输入" << std::endl;
    }
    else if(status == SUCCESS)
    {
        std::string get_member;
        r.recv_cil(fd,get_member);
        nlohmann::json get_member_json = nlohmann::json::parse(get_member);
        std::vector<std::string> member = get_member_json["member"];
        
        if(member.size()!=0)
        {
            system("echo \"MEMBER\" | figlet | boxes -d c | lolcat");
        }
        for(auto i:member)
        {
            std::cout << i << std::endl;
        }
    }
}

void User_work::Manage_Group(std::string id,int fd)
{
    std::cout << "请输入群组id" << std::endl;
    std::string group_id;
    std::cin >> group_id;
    std::cout << "请输入你要管理用户的id" << std::endl;
    std::string user_id;
    std::cin >> user_id;
    std::cout << "请输入你要管理的操作" << std::endl;
    std::cout << "1.踢出用户" << std::endl;
    std::cout << "2.禁言用户" << std::endl;
    std::cout << "3.解除禁言" << std::endl;
    std::cout << "4.添加管理员" << std::endl;
    std::cout << "5.取消管理员" << std::endl;
    std::string operation;
    std::cin >> operation;
    int ok;
    if(operation == "1")
    {
        ok = DELESOMEONE;
    }
    else if(operation == "2")
    {
        ok = NOSPEAKSOMEONE;
    }
    else if(operation == "3")
    {
        ok = SPEAKSOMEONE;
    }
    else if(operation == "4")
    {
        ok = ADDGROUPMANGER;
    }
    else if(operation == "5")
    {
        ok = DELEGROUPMANGER;
    }
    else 
    {
    
    }
    nlohmann::json Manage_Group_request = {
        {"id",id},
        {REQUEST,ok},
        {"Group_id",group_id},
        {"User_id",user_id}
        //{"Operation",operation}
    };
    Sen s;
    s.send_cil(fd,Manage_Group_request.dump());
    Rec r;
    int status;
    status = r.recv_ok(fd);
    if(status == GROUPIDNOTEXIST)
    {
        std::cout << "群组ID不存在" << std::endl;
        std::cout << "请重新输入" << std::endl;
    }
    else if(status == NOTINGROUP)
    {
        std::cout << "你不在该群聊" << std::endl;
        std::cout << "请重新输入" << std::endl;
    }
    else if(status == NOTGROUPOWNER)
    {
        std::cout << "你不是群主，不能进行该操作" << std::endl;
        std::cout << "请重新输入" << std::endl;
    }
    else if(status == SUCCESS)
    {
        std::cout << "成功进行该操作" << std::endl;
    }
    else if(status == NB)
    {
        std::cout << "你真的是倒反天罡，操作群主" << std::endl;
    }
    else if(status == NOHIGH)
    {
        std::cout << "你没有权限进行该操作" << std::endl;
    }
    // else if()
    // {
    
    // }
    else {
    
    }
}

void User_work::show_History()
{
    std::cout << "请输入聊天id:" << std::endl;
    std::string chat_id;
    std::cin >> chat_id;
    nlohmann::json show_history_request = {
        {"id",this->m_id},
        {REQUEST,SHOWHISTORY},
        {"Chat_id",chat_id}
    };
    Sen s;
    s.send_cil(this->m_fd,show_history_request.dump());
    Rec r;
    int status;
    status = r.recv_ok(this->m_fd);
    if(status == NOID)
    {
        std::cout << "没有该聊天id" << std::endl;
    }
    else if(status == NOTINGROUP)
    {
        std::cout << "你不在该群聊" << std::endl;
    }
    else if(status == NOFRIEND)
    {
        std::cout << "你不在该好友列表" << std::endl;
    }
    else if(status == NOMESSAGES)
    {
        std::cout << "你两好像不太熟，没有记录" << std::endl;
    }
    else if(status == NOHISTORY)
    {
        std::cout << "这个群聊好像不太活跃" << std::endl;
    }
    else if(status == SUCCESS)
    {
        std::string str;
        r.recv_cil(this->m_fd,str);
        nlohmann::json show_history_json = nlohmann::json::parse(str);
        std::vector<std::string> eraly = show_history_json["sql"];
        std::vector<std::string> now = show_history_json["redis"];
        if(eraly.size()!=0&&now.size()!=0)
        {
            std::cout << "--------------------------------------------------" << std::endl;
        }
        for(int i=eraly.size()-1;i>=0;i--)
        {
            //nlohmann::json show_history_json = nlohmann::json::parse(eraly[i]);
            std::cout << show_history_json["send"] << std::endl;
            std::cout << eraly[i] << std::endl;
        }
        for(int i=now.size()-1;i>=0;i--)
        {
            std::cout << "34646" << std::endl;
            nlohmann::json show_history_json = nlohmann::json::parse(now[i]);
            std::cout << show_history_json["send"] << std::endl;
        }
    }
    else 
    {
    
    }
}