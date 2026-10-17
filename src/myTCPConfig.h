#pragma once
#include <sys/socket.h>

namespace myTCPConfig
{
    constexpr const char* SERVER_IP = "127.0.0.1";
    constexpr int SERVER_PORT = 5000;
    constexpr int BUFFER_SIZE = 1024;
    constexpr int ADDR_FAMILY = AF_INET;
    constexpr int BACKLOG = 6; // How many clients waiting for accept at the same time
    constexpr int MAX_CLIENTS = 5;
    constexpr bool OPT_REUSES_ADDRESS = true;
} // namespace myTCPConfig