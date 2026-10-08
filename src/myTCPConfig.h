#pragma once

namespace myTCPConfig
{
    constexpr const char* SERVER_IP = "127.0.0.1";
    constexpr int SERVER_PORT = 5000;
    constexpr int BUFFER_SIZE = 1024;
    constexpr int ADDR_FAMILY = AF_INET;
    constexpr int BACKLOG = 5;
    constexpr bool OPT_REUSES_ADDRESS = true;
} // namespace myTCPConfig