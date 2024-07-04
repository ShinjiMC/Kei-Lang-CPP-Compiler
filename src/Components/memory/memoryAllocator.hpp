#pragma once

#include <cstdlib>
#include <cstddef>
#include <type_traits>

class MemoryAllocator
{
private:
    size_t m_size;
    std::byte *m_buffer;
    std::byte *m_offset;

public:
    inline explicit MemoryAllocator(size_t bytes)
        : m_size(bytes)
    {
        m_buffer = static_cast<std::byte *>(malloc(m_size));
        m_offset = m_buffer;
    }

    template <typename T>
    inline T *alloc()
    {
        void *offset = m_offset;
        m_offset += sizeof(T);
        return static_cast<T *>(offset);
    }

    inline MemoryAllocator(const MemoryAllocator &other) = delete;

    inline MemoryAllocator operator=(const MemoryAllocator &other) = delete;

    inline ~MemoryAllocator()
    {
        free(m_buffer);
    }
};