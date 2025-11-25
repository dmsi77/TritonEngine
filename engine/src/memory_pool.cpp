// memory_pool.cpp

#include <iostream>
#include <cstdlib>
#include "memory_pool.hpp"
#include "log.hpp"

using namespace types;

namespace realware
{
    cMemoryPool::cMemoryPool(cContext* context, usize byteSize, usize allocs, usize alignment) : iObject(context)
    {
        if (alignment == 0)
            _memory = malloc(byteSize);
        else
            _memory = _aligned_malloc(byteSize, alignment);

        _byteSize = byteSize;
        _lastAddress = _memory;
        _maxAddress = (void*)(((usize)_memory) + byteSize);
        _allocs.reserve(allocs);
        _alignment = alignment;
    }

    cMemoryPool::~cMemoryPool()
    {
        if (_alignment == 0)
            free(_memory);
        else
            _aligned_free(_memory);

        _allocs.clear();
    }

    void* cMemoryPool::Allocate(usize size)
    {
#ifdef DEBUG
        _bytesOccupied += size;
#endif
        for (auto& alloc : _allocs)
        {
            if (alloc._freeFlag == 1 && alloc._allocationByteSize >= size)
            {
                alloc._freeFlag = 255;
                alloc._occupiedByteSize = size;

                return alloc._address;
            }

            if ((usize)(alloc._allocationByteSize - alloc._occupiedByteSize) >= size)
            {
                sMemoryPoolAllocation newAlloc;
                newAlloc._freeFlag = 0;
                newAlloc._allocationByteSize = alloc._allocationByteSize - alloc._occupiedByteSize;
                newAlloc._occupiedByteSize = size;
                newAlloc._address = (void*)((usize)alloc._address + (usize)alloc._occupiedByteSize);

                alloc._allocationByteSize = alloc._occupiedByteSize;

                _allocs.emplace_back(newAlloc);

                return newAlloc._address;
            }
        }

        if ((usize)_lastAddress + size >= (usize)_maxAddress)
        {
            Print("Error: memory pool byte size '" + std::to_string(_byteSize) + "' is not enough to allocate next '" + std::to_string(size) + "' bytes!");
                
            return nullptr;
        }

        sMemoryPoolAllocation newAlloc;
        newAlloc._freeFlag = 0;
        newAlloc._allocationByteSize = size;
        newAlloc._occupiedByteSize = size;
        newAlloc._address = _lastAddress;
        _allocs.emplace_back(newAlloc);

        _lastAddress = (void*)((usize)_lastAddress + size);

        return newAlloc._address;
    }

    bool cMemoryPool::Free(void* address)
    {
        for (auto& alloc : _allocs)
        {
            if (alloc._address == address)
            {
#ifdef DEBUG
                _bytesFreed += alloc.OccupiedByteSize;
                _lastFreedBytes = alloc.OccupiedByteSize;
#endif
                alloc._freeFlag = 1;
                alloc._occupiedByteSize = 0;

                return true;
            }
        }

        return false;
    }
}