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
    void ListenThread();
    void StopListen();
    void JoinThread();

    int GetSocketFd();
private:
    void QueueClient();
    void UnqueueClient();
    
private:
    sockaddr_in m_socketAddress;
    int m_socketFd;
    std::mutex m_mutex;
    std::thread m_thread;
    std::vector<std::unique_ptr<TCPConnectingClient>> m_connectingClients;
};