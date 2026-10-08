#pragma once
#include <arpa/inet.h>
#include <thread>

#include "myTCPConfig.h"
#include "server/TCPServer.h"

class TCPServer;

enum class ClientStatus
{
    UNKNOWN = 0,
    IDLE = 1,
    READ_ONGOING = 2,
    WAITING_FOR_SERVER = 3,
    WRITE_ONGOING = 4,
    CLOSE_ONGOING = 5,
};

class TCPConnectingClient
{
public:
    TCPConnectingClient(TCPServer& i_server, int i_clientSocketFd, sockaddr_in i_clientAddress);
    ~TCPConnectingClient();

    void StartThread();
    
    void MainThread();
    
    void JoinThread();
    ClientStatus GetStatus();

private:
    TCPServer& m_server;
    sockaddr_in m_socketAddress;
    int m_socketFd;
    std::thread m_thread;
    ClientStatus m_status;
    char m_buffer[myTCPConfig::BUFFER_SIZE];
};