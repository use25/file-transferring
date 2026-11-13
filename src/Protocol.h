#pragma once

// TODO: Proper padding and encoding this struct later
struct Protocol
{
    bool m_isApproved;
    uint64_t m_fileSize;
};
static_assert(sizeof(Protocol) == 16);