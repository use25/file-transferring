#pragma once
#include <arpa/inet.h>
#include <vector>
#include <mutex>
#include <memory>
#include <thread>

#include "myTCPConfig.h"

class TCPConnectingClient;

class TCPServer
{
public:
    TCPServer();
    ~TCPServer();

    void StartListen();
    int GetSocketFd();

private:
    sockaddr_in m_socketAddress;
    int m_socketFd;
    std::vector<std::unique_ptr<TCPConnectingClient>> m_connectingClients;
};