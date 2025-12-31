#pragma once

#include <cstddef>
#include <cstdlib>
#include <memory>


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
        size_t space = m_capacity - static_cast<std::size_t>(m_offset - m_buffer);
        void* aligned_ptr = m_offset;
        if (std::align(alignof(T), sizeof(T), aligned_ptr, space) == nullptr) {
            return nullptr;
        }
        m_offset = static_cast<std::byte*>(aligned_ptr) + sizeof(T);
        return std::construct_at(static_cast<T*>(aligned_ptr));
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