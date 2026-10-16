#include "TCPConnectingClient.h"

#include <stdio.h>
#include <unistd.h>
#include <cstring>

#include "TCPHelper.h"

TCPConnectingClient::TCPConnectingClient(const TCPServer& i_server, const int i_clientSocketFd, const sockaddr_in i_clientAddress)
    : m_server((TCPServer&)i_server)
    , m_socketFd(i_clientSocketFd)
    , m_socketAddress(i_clientAddress)
    , m_status(ClientStatus::IDLE)
{
}

TCPConnectingClient::~TCPConnectingClient()
{
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

void TCPConnectingClient::StartThread()
{
    m_thread = std::thread(&TCPConnectingClient::MainThread, this);
}

void TCPConnectingClient::MainThread()
{
    memset(m_buffer, 0, myTCPConfig::BUFFER_SIZE);
    while (true)
    {
        size_t len_from_client;
        TCPHelper::ReturnCode result;

        m_status = ClientStatus::READ_ONGOING;
        result = TCPHelper::ReadN(m_socketFd, myTCPConfig::BUFFER_SIZE, m_buffer, len_from_client);
        if (result < TCPHelper::ReturnCode::SUCCESS)
        {
            perror("[server] read from client failed");
            break;
        }

        printf("[server] Received message from client: %s\n", m_buffer);
        // Sending back exactly the same buffer/message to client for now
        m_status = ClientStatus::WRITE_ONGOING;
        result = TCPHelper::WriteN(m_socketFd, m_buffer, len_from_client);
        if (result < TCPHelper::ReturnCode::SUCCESS)
        {
            perror("[server] write to client failed");
            break;
        }

        // Clean up before next read session
        memset(m_buffer, 0, len_from_client);
    }

    close(m_socketFd);

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &m_socketAddress.sin_addr, client_ip, sizeof(client_ip));
    printf("[server] client disconnected: %s:%d\n", client_ip, ntohs(m_socketAddress.sin_port));
}

ClientStatus TCPConnectingClient::GetStatus()
{
    return m_status;
}