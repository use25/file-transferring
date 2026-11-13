#pragma once

#include <fstream>
#include <string_view>
#include <cstdint>

namespace FileTransferServiceHelper
{
bool LookupFile(std::string_view i_input, std::ifstream& o_fileP);

/*
Only call these functiond after LookupFile()
*/
bool GetFileSize(std::ifstream& i_fileP, uint64_t& o_size);
};