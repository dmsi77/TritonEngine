// memory_pool.hpp

#pragma once

#include <vector>
#include "object.hpp"
#include "types.hpp"

namespace realware
{
    struct sMemoryPoolAllocation
    {
        types::u8 _freeFlag = 0;
        types::u32 _allocationByteSize = 0;
        types::u32 _occupiedByteSize = 0;
        void* _address = nullptr;
    };

    class cMemoryPool : public iObject
    {
    public:
        explicit cMemoryPool(cContext* context, types::usize byteSize, types::usize allocs, types::usize alignment);
        ~cMemoryPool();

        inline virtual cType GetType() const override final { return cType("MemoryPool"); }

        void* Allocate(types::usize size);
        bool Free(void* address);

        template <typename T>
        T* New(types::usize size)
        {
            T* ptr = (T*)Allocate(sizeof(T));
            return new (ptr) T(this);
        }

        inline types::usize GetAllocCount() const { return _allocs.size(); }
        inline types::usize GetByteSize() const { return _byteSize; }
        inline types::usize GetBytesUsed() const { return _bytesOccupied - _bytesFreed; }
        inline types::usize GetLastFreedBytes() const { return _lastFreedBytes; }

    private:
        types::usize _byteSize = 0;
        void* _memory = nullptr;
        void* _lastAddress = nullptr;
        void* _maxAddress = nullptr;
        std::vector<sMemoryPoolAllocation> _allocs = {};
        types::usize _bytesOccupied = 0;
        types::usize _bytesFreed = 0;
        types::usize _lastFreedBytes = 0;
        types::usize _alignment = 0;
    };
}