#include <csignal>
#include <cstdlib>

#include "server/TCPConnectingClient.h"
#include "server/TCPServer.h"

int main()
{
    setvbuf(stdout, nullptr, _IOLBF, 0);
    std::signal(SIGPIPE, SIG_IGN);

    TCPServer myServer = TCPServer();
    if (!myServer.StartListen())
    {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}