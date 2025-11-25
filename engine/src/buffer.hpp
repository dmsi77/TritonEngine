// buffer.hpp

#pragma once

#include "object.hpp"
#include "types.hpp"

namespace realware
{
    class cBuffer
    {
    public:
        explicit cBuffer() = default;
        ~cBuffer() = default;

    private:
        types::usize _byteSize = 0;
        void* _data = nullptr;
    };
}