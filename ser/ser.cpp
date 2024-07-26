#include "../head/head.hpp"
#include "../ser/ser.hpp"
#include "../sql/sql.hpp"

int main(int argc, char **argv)
{
    int port = PORT;
    if(argc >= 2)
    {
        port = std::stoi(argv[1]);
    }
    Server m_serve (port);
    m_serve.run();
    return 0;
}