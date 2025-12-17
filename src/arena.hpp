#pragma once

#include <cstddef>
#include <cstdlib>


class ArenaAllocator {
public:
    ArenaAllocator(size_t capacity)
        : m_capacity(capacity)
    {
        m_buffer = static_cast<std::byte*>(malloc(m_capacity));
        m_offset = m_buffer;
    }

    template<typename T>
    T* alloc(){
        void* offset = m_offset;
        m_offset += sizeof(T);
        return static_cast<T*>(offset);
    }

    ArenaAllocator(const ArenaAllocator&) = delete;
    ArenaAllocator& operator=(const ArenaAllocator&) = delete;

    ~ArenaAllocator()
    {
        free(m_buffer);
    }

private:
    size_t m_capacity;
    std::byte* m_buffer;
    std::byte* m_offset;
};