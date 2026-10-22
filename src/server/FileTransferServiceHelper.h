#pragma once

#include <fstream>
#include <string_view>
#include <cstdint>

namespace FileTransferServiceHelper
{
// TODO: Proper padding and encoding this struct later
struct Protocol
{
    bool m_isApproved;
    uint64_t m_fileSize;
};
static_assert(sizeof(Protocol) == 16);

bool LookupFile(std::string_view i_input, std::ifstream& o_fileP);

/*
Only call these functiond after LookupFile()
*/
bool GetFileSize(std::ifstream& i_fileP, uint64_t& o_size);
};