#include "../head/head.hpp"
//#include "../cil/menu.hpp"
#include "../cli/cli.hpp"
#include <bits/posix_opt.h>
#include <csignal>
#include <exception>
#include <termios.h>
#include <unistd.h>

#define PORT 8080

int main(int argc,char** argv)
{
    //禁用EOF
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_cc[VEOF] = _POSIX_VDISABLE;
    tcsetattr(STDOUT_FILENO, TCSANOW, &term);


    //禁用ctrl+C ctr+z
    signal(SIGINT,SIG_IGN); //c
    signal(SIGTSTP,SIG_IGN); //z
    signal(SIGQUIT,SIG_IGN); // \
 
    std::string server_addr = "127.0.0.1";
    int port = port;
    if(argc>=2)
    {
        server_addr = argv[1];
    }
    if(argc>=3)
    {
        port = std::stoi(argv[2]);
    }
    try 
    {
        cilent _cilent(server_addr,port);
        _cilent.run();
    } 
    catch (const std::exception &e) 
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}