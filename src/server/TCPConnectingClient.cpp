#include "TCPConnectingClient.h"

#include <stdio.h>
#include <unistd.h>
#include <cstring>
#include <cassert>

#include "TCPHelper.h"
#include "FileTransferServiceHelper.h"

TCPConnectingClient::TCPConnectingClient(const TCPServer& i_server, const int i_clientSocketFd, const sockaddr_in i_clientAddress)
    : m_server((TCPServer&)i_server)
    , m_socketFd(i_clientSocketFd)
    , m_socketAddress(i_clientAddress)
    , m_status(ClientStatus::IDLE)
{
}

TCPConnectingClient::~TCPConnectingClient()
{
    Cleanup();
}

void TCPConnectingClient::Reuse(int i_clientSocketFd, sockaddr_in i_clientAddress)
{
    assert(m_status == ClientStatus::POOLED);
    // Clean up old threads if still running somehow
    if (m_thread.joinable())
    {
git add m_thread.join();
    }
    m_socketFd = i_clientSocketFd;
    m_socketAddress = i_clientAddress;
    m_status = ClientStatus::IDLE;
}

void TCPConnectingClient::Pool()
{
    Cleanup();
    m_socketFd = -1;
    m_status = ClientStatus::POOLED;
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
git add size_t len_from_client;
git add TCPHelper::ReturnCode result;

git add m_status = ClientStatus::READ_ONGOING;
git add result = TCPHelper::ReadN(m_socketFd, myTCPConfig::BUFFER_SIZE, m_buffer, len_from_client);
git add if (result < TCPHelper::ReturnCode::SUCCESS)
git add {
git add     if (result == TCPHelper::ReturnCode::ERROR_SOCKET_CLOSED)
git add     {
git add git add break;
git add     }
git add     // TODO: Decide how to handle logic errors without having to disconnect
git add     break;
git add }

git add printf("[server] Received from client: %.*s\n", (int)len_from_client,  m_buffer);
git add // Sending back exactly the same buffer/message to client for now
git add std::ifstream fileP;
git add std::string clientFileName(m_buffer, len_from_client);
git add if (FileTransferServiceHelper::LookupFile(std::string_view(clientFileName), fileP))
git add {
git add     FileTransferServiceHelper::Protocol protocolToSend{true, 0};
git add     if (FileTransferServiceHelper::GetFileSize(fileP, protocolToSend.m_fileSize))
git add     {
git add git add result = TCPHelper::WriteN(m_socketFd, (const void*)&protocolToSend, sizeof(protocolToSend));
git add git add if (result < TCPHelper::ReturnCode::SUCCESS)
git add git add {
git add git add     if (result == TCPHelper::ReturnCode::ERROR_SOCKET_CLOSED)
git add git add     {
git add git add git add break;
git add git add     }
git add git add     // TODO: Decide how to handle logic errors without having to disconnect
git add git add     break;
git add git add }

git add git add while (protocolToSend.m_fileSize > myTCPConfig::BUFFER_SIZE)
git add git add {
git add git add     protocolToSend.m_fileSize -= myTCPConfig::BUFFER_SIZE;
git add git add     fileP.read(m_buffer, myTCPConfig::BUFFER_SIZE);
git add git add     result = TCPHelper::HandleSocketError(write(m_socketFd, &m_buffer, myTCPConfig::BUFFER_SIZE));
git add git add     if (result < TCPHelper::ReturnCode::SUCCESS)
git add git add     {
git add git add git add if (result == TCPHelper::ReturnCode::ERROR_SOCKET_CLOSED)
git add git add git add {
git add git add git add     break;
git add git add git add }
git add git add git add // TODO: Decide how to handle logic errors without having to disconnect
git add git add git add break;
git add git add     }
git add git add }
git add git add fileP.read(m_buffer, protocolToSend.m_fileSize);
git add git add result = TCPHelper::HandleSocketError(write(m_socketFd, &m_buffer, protocolToSend.m_fileSize));
git add git add if (result < TCPHelper::ReturnCode::SUCCESS)
git add git add {
git add git add     if (result == TCPHelper::ReturnCode::ERROR_SOCKET_CLOSED)
git add git add     {
git add git add git add break;
git add git add     }
git add git add     // TODO: Decide how to handle logic errors without having to disconnect
git add git add     break;
git add git add }
git add     }
git add     fileP.close();
git add }
git add else
git add {
git add     FileTransferServiceHelper::Protocol protocolToSend{false, 0};
git add     result = TCPHelper::WriteN(m_socketFd, (const void*)&protocolToSend, sizeof(protocolToSend));
git add     if (result < TCPHelper::ReturnCode::SUCCESS)
git add     {
git add git add if (result == TCPHelper::ReturnCode::ERROR_SOCKET_CLOSED)
git add git add {
git add git add     break;
git add git add }
git add git add // TODO: Decide how to handle logic errors without having to disconnect
git add git add break;
git add     }
git add }

git add // Clean up before next read session
git add memset(m_buffer, 0, len_from_client);
    }

    close(m_socketFd);
    m_status = ClientStatus::DISCONNECTED;

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &m_socketAddress.sin_addr, client_ip, sizeof(client_ip));
    printf("[server] client disconnected: %s:%d\n", client_ip, ntohs(m_socketAddress.sin_port));
}

ClientStatus TCPConnectingClient::GetStatus()
{
    return m_status;
}

void TCPConnectingClient::Cleanup()
{
    if (m_thread.joinable())
    {
git add m_thread.join();
    }
}