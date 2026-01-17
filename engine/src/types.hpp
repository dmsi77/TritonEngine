// types.hpp

#ifndef _DIMON2201_TYPES_H_
#define _DIMON2201_TYPES_H_

#include <cstdint>
#include <string>

namespace types
{
    using s8 = int8_t;
    using u8 = uint8_t;
    using s16 = int16_t;
    using u16 = uint16_t;
    using s32 = int32_t;
    using u32 = uint32_t;
    using s64 = int64_t;
    using u64 = uint64_t;
    using f32 = float;
    using f64 = double;
    using usize = size_t;
    using dword = u32;
    using qword = u64;

#if defined(__ILP32__) || defined(__arm__) || defined(_M_ARM) || defined(__i386__) || defined(_M_IX86) || defined(_X86_)
    // 32-bit architecture
    constexpr int CPU_ARCH = 32;
    using cpuword = dword;
#elif defined(__amd64__) || defined(_M_AMD64) || defined(_M_X64) || defined(__aarch64__) || defined(__ia64__) || defined(_M_IA64)
    // 64-bit architecture
    constexpr int CPU_ARCH = 64;
    using cpuword = qword;
#else
    // Unknown architecture
#error Unknown architecture
#endif

    using boolean = cpuword;

    constexpr boolean K_TRUE = 1;
    constexpr boolean K_FALSE = 0;
    constexpr usize K_USIZE_MAX = SIZE_MAX;
}

#endif