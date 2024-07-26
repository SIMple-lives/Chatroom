#include "../head/head.hpp"

class Menu
{
    public:
    void show_menu();
    void showLogin() ;  //登陆界面
    void showEnroll() ; //注册
    void showLogout();  //注销界面
    //virtual void show_Find(std::string password);
    void showExit();    //退出界面

    virtual void showUser() ;//登陆进入后的界面

    virtual void showFriend() ;//添加好友界面

    virtual void showGroups() ;//加入的群聊以及管理的界面

    virtual void show_friend(std::string id,std::string name ,std::string telephone,int status) ;// 好友信息界面

    void show_group() ;// 群聊信息界面

    void show_group_manager() ;// 群聊管理员的界面
};