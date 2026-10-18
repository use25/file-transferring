#include "TCPServer.h"

#include <stdio.h>
#include <cstring>
#include <unistd.h>
#include <algorithm>
#include <cerrno>

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
        return;
    }
    m_isBinded = true;
}

TCPServer::~TCPServer()
{
    if (m_socketFd >= 0)
    {
        close(m_socketFd);
    }
}

bool TCPServer::StartListen()
{
    if (!m_isBinded)
    {
        printf("[server] StartListen error: Server didn't bind successfully");
        return false;
    }

    printf("[server] Setting up listening\n");
    m_connectingClients.reserve(myTCPConfig::MAX_CLIENTS);
    m_pooledClients.reserve(myTCPConfig::MAX_CLIENTS);
    if (listen(m_socketFd, myTCPConfig::BACKLOG) < 0)
    {
        perror("listen() failed");
        return false;
    }

    printf("[server] Start listening\n");

    sockaddr_in clientAddress;
    socklen_t clientSizeLen;
    int clientSocketFd;
    
    while (true)
    {
        clientSizeLen = sizeof(clientAddress);
        // pooling disconnected client elements
        for (size_t i = 0; i < m_connectingClients.size();)
        {
            if (m_connectingClients[i]->GetStatus() != ClientStatus::DISCONNECTED)
            {
                i++;
                continue;
            }
            m_connectingClients[i]->Pool();
            m_pooledClients.push_back(std::move(m_connectingClients[i]));
            if (i < m_connectingClients.size() - 1)
            {
                m_connectingClients[i] = std::move(m_connectingClients.back());
            }
            m_connectingClients.pop_back();
        }
        
        // handshake
        printf("[server] Accepting client\n");
        clientSocketFd = accept(m_socketFd, (sockaddr*)&clientAddress, &clientSizeLen);
        if (clientSocketFd < 0)
        {
            perror("accept() failed");
            continue;
        }
        SetupClient(clientSocketFd, clientAddress);

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddress.sin_addr, client_ip, sizeof(client_ip));
        printf("[server] client connected: %s:%d\n", client_ip, ntohs(clientAddress.sin_port));
    }
    return true;
}

void TCPServer::SetupClient(const int i_socketFd, const sockaddr_in i_address)
{
    if (m_pooledClients.size() <= 0)
    {
        m_connectingClients.push_back(std::make_unique<TCPConnectingClient>(*this, i_socketFd, i_address));
    }
    else
    {
        m_connectingClients.push_back(std::move(m_pooledClients.back()));
        m_connectingClients.back()->Reuse(i_socketFd, i_address);
        m_pooledClients.pop_back();

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &i_address.sin_addr, client_ip, sizeof(client_ip));
        printf("[server] Setup client from reserved pool: %s:%d\n", client_ip, ntohs(i_address.sin_port));
    }
    m_connectingClients.back()->StartThread();
}