#include "../head/head.hpp"
#include "../head/define.hpp"
#include "../sen_rec/sen_rec.hpp"
#include "../sql/sql.hpp"

class Msg
{
public:
    Msg() = default ;
    Msg(int source, int dest,std::string &ida,std::string &idb) :m_source(source),m_dest(dest),id_s(ida),id_d(idb)
    {}
    void send_off(std::vector<std::string> &offline);
    void run(std::string &message);
    void run_Group(std::string &message,std::string &id);
private:
    int m_source;
    int m_dest;
    std::string id_s;
    std::string id_d;
};