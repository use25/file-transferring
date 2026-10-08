#include "server/TCPConnectingClient.h"
#include "server/TCPServer.h"

int main()
{
    setvbuf(stdout, nullptr, _IOLBF, 0);

    TCPServer myServer = TCPServer();
    myServer.StartListen();
    myServer.JoinThread();
    return 0;
}