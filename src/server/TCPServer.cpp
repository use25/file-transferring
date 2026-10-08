#include "TCPServer.h"

#include <stdio.h>
#include <cstring>
#include <unistd.h>

#include "server/TCPConnectingClient.h"

TCPServer::TCPServer()
    : m_socketFd(socket(myTCPConfig::ADDR_FAMILY, SOCK_STREAM, IPPROTO_TCP))
{
    if (m_socketFd < 0)
    {
        perror("socket() failed");
        return;
    }

    if (myTCPConfig::OPT_REUSES_ADDRESS)
    {
        int opt = 1;
        setsockopt(m_socketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    }

    memset(&m_socketAddress, 0, sizeof(m_socketAddress));
    m_socketAddress.sin_family = myTCPConfig::ADDR_FAMILY;
    m_socketAddress.sin_addr.s_addr = INADDR_ANY;
    m_socketAddress.sin_port = htons(myTCPConfig::SERVER_PORT);
    
    if (bind(m_socketFd, (sockaddr*)&m_socketAddress, sizeof(m_socketAddress)) < 0)
    {
        perror("bind() failed");
        close(m_socketFd);
        return;
    }
}

TCPServer::~TCPServer()
{
    for (int i = 0; i < m_connectingClients.size(); i++)
    {
        m_connectingClients[i]->JoinThread();
    }
    close(m_socketFd);
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

void TCPServer::StartListen()
{
    printf("[server] Setting up listening\n");
    m_connectingClients.reserve(myTCPConfig::BACKLOG);
    if (listen(m_socketFd, myTCPConfig::BACKLOG) < 0)
    {
        perror("listen() failed");
        close(m_socketFd);
        return;
    }

    printf("[server] Start listening\n");
    m_thread = std::thread(&TCPServer::ListenThread, this);
}

void TCPServer::ListenThread()
{
    sockaddr_in clientAddress;
    socklen_t clientSizeLen = sizeof(clientAddress);

    int clientSocketFd;
    while (true)
    {
        // handshake
        printf("[server] Accepting client\n");
        clientSocketFd = accept(m_socketFd, (sockaddr*)&clientAddress, &clientSizeLen);
        if (clientSocketFd < 0)
        {
            perror("accept() failed");
            continue;
        }

        std::unique_ptr<TCPConnectingClient> newClient = std::make_unique<TCPConnectingClient>(*this, clientSocketFd, clientAddress);
        m_connectingClients.push_back(std::move(newClient));
        m_connectingClients.back()->StartThread();
        printf("[server] Started client\n");
    }
}

void TCPServer::JoinThread()
{
    m_thread.join();
}