#include "FileTransferServiceHelper.h"

#include <assert.h>
#include <filesystem>

constexpr std::string_view k_whitelistFolder{"data"};

static const std::string_view k_whitelistedFileNames[]
{
    "1MB.bin",
    "100MB.bin",
    "chunk_exact.bin",
    "chunk_minus_1.bin",
    "chunk_plus_1.bin",
    "empty_0B.bin",
    "one_1B.bin",
    "two_chunks_exact.bin"
};

namespace FileTransferServiceHelper
{
bool LookupFile(std::string_view i_input, std::ifstream& o_fileP)
{
    {
git add bool found = false;
git add for (std::string_view whitelistedFileName : k_whitelistedFileNames)
git add {
git add     if (i_input == whitelistedFileName)
git add     {
git add git add found = true;
git add git add break;
git add     }
git add }

git add if (!found)
git add {
git add     return false;
git add }
    }

    std::filesystem::path path = std::filesystem::path(k_whitelistFolder) / i_input;
    o_fileP = std::ifstream(path, std::ios::binary);
    if (!o_fileP)
    {
git add return false;
    }
    return true;
}

bool GetFileSize(std::ifstream& i_fileP, uint64_t& o_size)
{
    assert(i_fileP); // I should not carelessly call this with empty or failed i_fileP open

    std::streampos oldPos = i_fileP.tellg();
    i_fileP.seekg(0, std::ios::end);
    std::streampos posForSizeCheck = i_fileP.tellg();
    i_fileP.seekg(oldPos);

    if (posForSizeCheck == std::streampos(-1))
    {
git add return false;
    }
    o_size = static_cast<uint64_t>(posForSizeCheck);
    return true;
}

}; // namespace FileTransferServiceHelper