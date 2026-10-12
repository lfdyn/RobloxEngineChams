#pragma once

#include <Windows.h>

#include "memory.h"

namespace chams
{
    class ProcessMemory final : public Memory
    {
    public:
        explicit ProcessMemory(HANDLE process)
        {
            DuplicateHandle(GetCurrentProcess(), process, GetCurrentProcess(), &g_process, 0, FALSE, DUPLICATE_SAME_ACCESS);
        }

        ~ProcessMemory() override
        {
            if (g_process)
            {
                CloseHandle(g_process);
            }
        }

        ProcessMemory(const ProcessMemory&) = delete;
        ProcessMemory& operator=(const ProcessMemory&) = delete;

        bool ReadBytes(std::uint64_t address, void* buffer, std::size_t size) const override
        {
            SIZE_T transferred = 0;
            return Valid(address, size) && ReadProcessMemory(g_process, reinterpret_cast<const void*>(address), buffer, size, &transferred) && transferred == size;
        }

        bool WriteBytes(std::uint64_t address, const void* buffer, std::size_t size) override
        {
            SIZE_T transferred = 0;
            return Valid(address, size) && WriteProcessMemory(g_process, reinterpret_cast<void*>(address), buffer, size, &transferred) && transferred == size;
        }

    private:
        static bool Valid(std::uint64_t address, std::size_t size)
        {
            return address >= 0x10000 && address < 0x0000800000000000 && size <= 0x0000800000000000 - address;
        }

        HANDLE g_process = nullptr;
    };
}
