#include "../head/head.hpp"
#include "../head/define.hpp"

class Sql 
{
public:
    Sql() : connection(nullptr) 
    {
        this->connection = mysql_init(nullptr);
        if (this->connection == nullptr) 
        {
            std::cerr << "mysql: init failed" << std::endl;
            return;
        }

        if (!mysql_real_connect(this->connection, "localhost", "root", "123456", "chat", 3306, nullptr, 0)) 
        {
            std::cerr << "mysql: connect failed" << mysql_error(this->connection) << std::endl;
            mysql_close(this->connection);
            return;
        }
         
        if (!createChatNamesTable()) {
            std::cerr << "mysql: create table chat_names failed" << std::endl;
            mysql_close(this->connection);
            return;
        }
        std::cout << "mysql: connect success" << std::endl;
    }

    bool addChatName(const std::string &chat_name) 
    {
        std::string query = "INSERT INTO chat_names (name) VALUES('" + chat_name + "')";
        if (mysql_query(this->connection, query.c_str())) 
        {
            std::cerr << "mysql: insert chat name failed " << mysql_error(this->connection) << std::endl;
            return false;
        }
        return true;
    }

    bool insertChat(const std::string &table_name, const std::string &message_text) 
    {
        if (!createChatHistoryTable(table_name)) 
        {
            std::cerr << "mysql: create chat history table failed" << std::endl;
            return false;
        }
        std::string query = "INSERT INTO " + table_name + " (message_text) VALUES('" + message_text + "')";
        if (mysql_query(this->connection, query.c_str())) 
        {
            std::cerr << "mysql: insert failed " << mysql_error(this->connection) << std::endl;
            return false;
        }
        return true;
    }

    bool insertMultipleChats(const std::string &table_name, const std::vector<std::string> &messages) {
        if (!createChatHistoryTable(table_name)) 
        {
            std::cerr << "mysql: create chat history table failed" << std::endl;
            return false;
        }
        for (const auto &message : messages) 
        {
            std::string query = "INSERT INTO " + table_name + " (message_text) VALUES('" + message + "')";
            if (mysql_query(this->connection, query.c_str())) 
            {
                std::cerr << "mysql: insert failed " << mysql_error(this->connection) << std::endl;
                return false;
            }
        }
        return true;
    }
    std::vector<std::string> getChatHistory(const std::string &table_name) 
    {
        std::vector<std::string> messages;
        std::string query = "SELECT message_text FROM " + table_name;
        if (mysql_query(this->connection, query.c_str())) 
        {
            std::cerr << "mysql: select failed " << mysql_error(this->connection) << std::endl;
            return std::vector<std::string>();
        }

        MYSQL_RES *result = mysql_store_result(this->connection);
        if (result == nullptr) 
        {
            std::cerr << "mysql: store result failed " << mysql_error(this->connection) << std::endl;
            return messages;
        }

        MYSQL_ROW row;
        while ((row = mysql_fetch_row(result))) 
        {
            messages.push_back(row[0]);
        }

        mysql_free_result(result);
        return messages;
    }
    
    ~Sql() 
    {
        if (this->connection != nullptr) 
        {
            mysql_close(this->connection);
        }
    }

private:
    MYSQL *connection; // 数据库连接

     bool createChatHistoryTable(const std::string &table_name) {
        std::string query = "CREATE TABLE IF NOT EXISTS " + table_name + R"(
            (
                id INT AUTO_INCREMENT PRIMARY KEY,
                message_text TEXT NOT NULL,
                timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        )";
        if (mysql_query(this->connection, query.c_str())) {
            std::cerr << "mysql: table creation failed " << mysql_error(this->connection) << std::endl;
            return false;
        }
        return true;
    }
    bool createChatNamesTable() 
    {
        const char *query = R"(CREATE TABLE IF NOT EXISTS chat_names(
            id INT AUTO_INCREMENT PRIMARY KEY,
            name VARCHAR(255) UNIQUE NOT NULL
        ))";
        if (mysql_query(this->connection, query)) 
        {
            std::cerr << "mysql: table creation failed " << mysql_error(this->connection) << std::endl;
            return false;
        }
        return true;
    }
};
