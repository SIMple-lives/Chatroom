#include "../head/head.hpp"

class S_User
{
public:
    S_User() = default;
    ~S_User() = default;
    void S_Login(int fd,std::string str);
    void S_Signup(int fd,std::string str);
    void S_Signout(int fd,std::string str);
    void S_FindA(int fd,std::string str);
    void S_FindB(int fd,std::string str);
    void S_EXIT(int fd,std::string str);
    void S_QUIT(int fd,std::string str);
    void S_AddFriend(int fd,std::string str);
    void S_ViewFriendinfo (int fd,std::string str);
    void S_Viewpersonalinfo(int fd,std::string str);
    void S_BlockFriend(int fd,std::string str);
    void S_ViewFriends(int fd,std::string str);
    void S_DeleFriends(int fd,std::string str);
    void S_ViewBlockFriends(int fd,std::string str);
    void S_ViewFriendRequest(int fd,std::string str);
    void S_HandleFriendRequset(int fd,std::string str);
    void S_PrivateChat(int fd,std::string,int dest );
    void S_UnBlockFriend(int fd,std::string str);
    void S_Chat(int fd,std::string str,int dest);
    void S_ExitChat(int fd,std::string str);
    void S_Refresh(int fd,std::string std);
    void S_CreateGroup(int fd,std::string str);
    void S_JoinGroup(int fd,std::string str,int dest);
    void S_QuitGroup(int fd,std::string str);
    void S_ListGroup(int fd,std::string str);
    void S_MemberGroup(int fd,std::string str);
    void S_DeleGroup(int fd,std::string str);
    void S_HandleGroupRequset(int fd ,std::string str);
    void S_DoGroupRequest(int fd,std::string str);
    void S_ChatGroup(int fd,std::string str);
    void S_GroupChat(int fd,std::string str,std::vector<int> &v,std::vector<std::string> &v2);
    void S_DeleSomeone(int fd,std::string str);
    void S_NoSpeakSomeone(int fd,std::string str);
    void S_SpeakSomeone(int fd,std::string str);
    void S_AddGroupManager(int fd,std::string str);
    void S_DeleGroupManager(int fd,std::string str);
    void S_IfFriend(int fd,std::string str);
    void S_IfInGroup(int fd,std::string str);
    void S_SendFile(int fd,std::string str,int dest);
    void S_MessageFile(int fd,std::string str);
    void S_ReceiveFile(int fd,std::string str);
    void S_ShowHistory(int fd,std::string str);
private:
    
}; 