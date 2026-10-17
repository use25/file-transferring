#include <csignal>

#include "server/TCPConnectingClient.h"
#include "server/TCPServer.h"

int main()
{
    setvbuf(stdout, nullptr, _IOLBF, 0);
    std::signal(SIGPIPE, SIG_IGN);

    TCPServer myServer = TCPServer();
    myServer.StartListen();
    return 0;
}