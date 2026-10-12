#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace chams
{
    class Memory
    {
    public:
        virtual ~Memory() = default;
        virtual bool ReadBytes(std::uint64_t address, void* buffer, std::size_t size) const = 0;
        virtual bool WriteBytes(std::uint64_t address, const void* buffer, std::size_t size) = 0;

        template <typename T>
        bool Read(std::uint64_t address, T& value) const
        {
            static_assert(std::is_trivially_copyable_v<T>);
            return ReadBytes(address, &value, sizeof(value));
        }

        template <typename T>
        bool Write(std::uint64_t address, const T& value)
        {
            static_assert(std::is_trivially_copyable_v<T>);
            return WriteBytes(address, &value, sizeof(value));
        }
    };
}
