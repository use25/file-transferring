#pragma once

#include <cstddef>
#include <sys/types.h>

namespace TCPHelper
{

enum ReturnCode : int
{
    SUCCESS = 0,
    ERROR_GENERIC = -1,
    ERROR_MAX_LENGTH = -2,
    ERROR_SOCKET_CLOSED = -3
};

ReturnCode HandleSocketError(const ssize_t i_result);
ReturnCode WriteN(const int i_sockFd, const void* i_buffer, const size_t i_len);
ReturnCode ReadN(const int i_sockFd, const size_t i_maxLen, void* o_buffer, size_t& o_len);

} // namespace TCPHelper