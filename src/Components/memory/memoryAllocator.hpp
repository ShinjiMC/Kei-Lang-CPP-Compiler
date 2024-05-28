#pragma once

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

class MemoryAllocator {
private:
    size_t capacity_;
    std::vector<std::byte> buffer_;
    std::byte* current_;

public:
    explicit MemoryAllocator(size_t capacity)
        : capacity_(capacity)
        , buffer_(capacity)
        , current_(buffer_.data())
    {
    }
    MemoryAllocator(const MemoryAllocator&) = delete;
    MemoryAllocator& operator=(const MemoryAllocator&) = delete;

    template <typename T>
    [[nodiscard]] T* allocate()
    {
        size_t remainingBytes = capacity_ - static_cast<size_t>(current_ - buffer_.data());
        auto pointer = static_cast<void*>(current_);
        void* alignedAddress = std::align(alignof(T), sizeof(T), pointer, remainingBytes);
        if (alignedAddress == nullptr) {
            throw std::bad_alloc {};
        }
        current_ = static_cast<std::byte*>(alignedAddress) + sizeof(T);
        return static_cast<T*>(alignedAddress);
    }

    template <typename T, typename... Args>
    [[nodiscard]] T* construct(Args&&... args)
    {
        T* allocatedSpace = allocate<T>();
        return new (allocatedSpace) T(std::forward<Args>(args)...);
    }
};
