#include "../head/head.hpp"
#include "../cli/menu.hpp"
#include "../head/define.hpp"



class Users : public Menu
{
public:
    Users() : id(""), username(""), password(""), telephone(""), questions(""), que_ans("") {}

    void Singup(int fd);

    void Login(int fd,int port,std::string &ip);

    void Logout(int fd);

    void Find(int fd,std::string id);

    void Update(const std::string& id, const std::string& username, const std::string& password, const std::string& questions, const std::string& que_ans, const std::string& telephone);
    
    void Exit(int fd);

    void Work(int fd,int port,std::string &ip);

   
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


    virtual void showUser() override // 在成员函数的声明或者定义中，override 确保该函数为虚函数并覆写来自基类的虚函数
    {
        //system("clear");
        system("echo \"Interface \" | figlet | boxes -d c | lolcat") ;
        std::string name = this->username + " 'S interface'";
        printCentered(name, 50);
        std::cout << std::setw(50) << std::setfill('-') << '-' << std::endl; // 横线分隔
        std::cout << std::setw(50) << std::setfill('-') << '-' << std::endl; // 横线分隔
        std::cout << std::left << std::setw(25) << "          1. 添加好友" << "    2. 好友申请" << std::endl;
        std::cout << std::left << std::setw(25) << "          3. 好友私聊" << "    4. 好友列表" << std::endl;
        std::cout << std::left << std::setw(25) << "          5. 好友信息" << "    6. 屏蔽好友" << std::endl;
        std::cout << std::left << std::setw(25) << "          7. 屏蔽列表" << "    8. 删除好友" << std::endl;

        printCentered("9. 个人信息",48); 
        printCentered("10. 群聊", 44);
        printCentered("11. 刷新",44);
        printCentered("12. 文件",44);
        printCentered("13. 历史记录",48);
        printCentered("14. 退出",44);

        std::cout << std::setw(50) << std::setfill('-') << '-' << std::endl; // 横线分隔
        std::cout << std::setw(50) << std::setfill('-') << '-' << std::endl; // 横线分隔
        
    }

    explicit Users(const std::string &s)
    {
        try
        {
            nlohmann::json js = nlohmann::json::parse(s);
            id = js.at("id").get<std::string>();
            username = js.at("username").get<std::string>();
            password = js.at("password").get<std::string>();
            telephone = js.at("telephone").get<std::string>();
            if (js.find("questions") != js.end())
            {
                questions = js.at("questions").get<std::string>();
            }
            else
            {
                questions = ""; // 如果字段不存在，设置为空字符串
            }
            if (js.find("que_ans") != js.end())
            {
                que_ans = js.at("que_ans").get<std::string>();
            }
            else
            {
                que_ans = ""; // 如果字段不存在，设置为空字符串
            }
            status = js.at("status").get<int>();
        }
        catch (const std::exception &e) // 捕获JSON 解析或转换异常
        {
            throw std::invalid_argument("Invalid JSON string: " + std::string(e.what())); // invalid_argument 表示使用了无效的参数会抛出异常
        }

    }

    std::string to_string()
    {
        nlohmann::json js;
        js["id"] = id;
        js["name"] = username;
        js["password"] = password;
        js["telephone"] = telephone;
        js["questions"] = questions;
        js["que_ans"] = que_ans;
        js["status"] = status;
        std::string ret = js.dump('\t'); // 输出的以\t为缩进的字符串
        return ret;
    }
private:

    void setid(std::string id)
    {
        this->id = id;
    }
    
    void setusername(std::string username)
    {
        this->username = username;
    }
    void setpassword(std::string password)
    {
        this->password = password;
    }
    void settelephone(std::string telephone)
    {
        this->telephone = telephone;
    }
    void setquestions(std::string questions)
    {
        this->questions = questions;
    }
    void setque_ans(std::string que_ans)
    {
        this->que_ans = que_ans;
    }
    void setstatus(int status)
    {
        this->status = status;
    }
    std::string getid()
    {
        return id;
    }
    std::string getpassword()
    {
        return password;
    }
    std::string gettelephone()
    {
        return telephone;
    }
    std::string getquestions()
    {
        return questions;
    }
    std::string getque_ans()
    {
        return que_ans;
    }
    int getstatus()
    {
        return status;
    }
    std::string id;
    std::string username;
    std::string password;
    std::string telephone;
    std::string questions;
    std::string que_ans;
    int status;
};
