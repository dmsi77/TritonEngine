// buffer.hpp

#pragma once

#include "object.hpp"
#include "types.hpp"

namespace realware
{
    namespace utils
    {
        class cBuffer : public cObject
        {
        public:
            explicit cBuffer() = default;
            ~cBuffer() = default;

        private:
            types::usize _byteSize = 0;
            void* _data = nullptr;
        };
    }
}