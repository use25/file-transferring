#pragma once
#include <arpa/inet.h>
#include <atomic>
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
    DISCONNECTED = 5,
    POOLED = 6
};

class TCPConnectingClient
{
public:
    TCPConnectingClient(const TCPServer& i_server, const int i_clientSocketFd, const sockaddr_in i_clientAddress);
    ~TCPConnectingClient();

    // This is for pooling function from TCPServer
    void Reuse(const int i_clientSocketFd, const sockaddr_in i_clientAddress);
    void Pool();

    void StartThread();
    void MainThread();

    ClientStatus GetStatus();

private:
    void Cleanup();

private:
    TCPServer& m_server;
    int m_socketFd;
    sockaddr_in m_socketAddress;
    std::thread m_thread;
    std::atomic<ClientStatus> m_status;
    char m_buffer[myTCPConfig::BUFFER_SIZE];
};