#include "TCPHelper.h"

#include <cassert>
#include <cerrno>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

namespace TCPHelper
{
ReturnCode HandleSocketError(const ssize_t i_result)
{
    if (i_result < 0)
    {
        switch (errno)
        {
        case ECONNRESET:
            perror("[TCPHelper] errno: ECONNRESET");
            return ReturnCode::ERROR_SOCKET_CLOSED;
            break;
        case EPIPE:
            perror("[TCPHelper] errno: EPIPE");
            return ReturnCode::ERROR_SOCKET_CLOSED;
            break;
        default: //NOTE: Breakpoint if this happens
            perror("[TCPHelper] unhandled errno received");
            return ReturnCode::ERROR_GENERIC;
            break;
        }
    }
    else if (i_result == 0)
    {
        printf("[TCPHelper] Socket closed normally\n");
        return ReturnCode::ERROR_SOCKET_CLOSED;
    }
    return ReturnCode::SUCCESS;
}

ReturnCode WriteN(const int i_sockFd, const void* i_buffer, const size_t i_len)
{
    assert(i_len > 0);
    ssize_t bytesSent;

    uint32_t lenNetwork = htonl(i_len);
    bytesSent = write(i_sockFd, &lenNetwork, sizeof(lenNetwork));

    if (ReturnCode socketErrorResult = HandleSocketError(bytesSent); socketErrorResult < ReturnCode::SUCCESS)
    {
        return socketErrorResult;
    }

    if (bytesSent != sizeof(lenNetwork))
    {
        printf("[TCPHelper] write length failed. bytesSent: %zd\n", bytesSent);
        return ReturnCode::ERROR_GENERIC;
    }

    size_t totalBytesSent = 0;
    do
    {
        bytesSent = write(i_sockFd, (const char*)i_buffer + totalBytesSent, i_len - totalBytesSent);
        if (ReturnCode socketErrorResult = HandleSocketError(bytesSent); socketErrorResult < ReturnCode::SUCCESS)
        {
            return socketErrorResult;
        }
        totalBytesSent += bytesSent;
    } while (totalBytesSent < i_len);
    return ReturnCode::SUCCESS;
}

ReturnCode ReadN(const int i_sockFd, const size_t i_maxLen, void* o_buffer, size_t& o_len)
{
    uint32_t lenNetwork;
    ssize_t bytesReceived = read(i_sockFd, &lenNetwork, sizeof(lenNetwork));

    if (ReturnCode socketErrorResult = HandleSocketError(bytesReceived); socketErrorResult < ReturnCode::SUCCESS)
    {
        return socketErrorResult;
    }
    if (bytesReceived != sizeof(lenNetwork))
    {
        printf("[TCPHelper] read length failed. bytesReceived: %zd\n", bytesReceived);
        return ReturnCode::ERROR_GENERIC;
    }

    o_len = ntohl(lenNetwork);
    if (o_len == 0)
    {
        printf("[TCPHelper] read length failed: Client informed incoming message with 0 bytes.\n");
        return ReturnCode::ERROR_GENERIC;
    }

    if (o_len > i_maxLen)
    {
        printf("[TCPHelper] read length failed: buffer length (%zu) is higher than max length (%zu)\n", o_len, i_maxLen);
        return ReturnCode::ERROR_MAX_LENGTH;
    }

    size_t totalBytesReceived = 0;
    do
    {
        bytesReceived = read(i_sockFd, (char*)o_buffer + totalBytesReceived, o_len - totalBytesReceived);
        if (ReturnCode socketErrorResult = HandleSocketError(bytesReceived); socketErrorResult < ReturnCode::SUCCESS)
        {
            return socketErrorResult;
        }
        totalBytesReceived += bytesReceived;
    } while (totalBytesReceived < o_len);
    return ReturnCode::SUCCESS;
}

} // namespace TCPHelper