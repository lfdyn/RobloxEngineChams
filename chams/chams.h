#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>

#include "memory.h"

namespace chams
{
    class Settings final
    {
    public:
        bool g_enabled = true;
        bool g_through_walls = true;
        std::array<float, 4> g_fill = { 0.32f, 0.65f, 1.0f, 0.4f };
        std::array<float, 4> g_outline = { 0.75f, 0.88f, 1.0f, 1.0f };
    };

    enum class Status
    {
        Disabled,
        UnsupportedVersion,
        WaitingForGame,
        WaitingForCharacters,
        NoAvailableIds,
        MemoryUnavailable,
        Active
    };

    class Engine final
    {
    public:
        Engine(std::shared_ptr<Memory> memory, std::uint64_t base);
        ~Engine();
        Engine(const Engine&) = delete;
        Engine& operator=(const Engine&) = delete;
        void Update(const Settings& settings);
        void Stop();
        Status GetStatus() const;
        std::size_t GetEntityCount() const;
        std::size_t GetPlayerCount() const;
        static const char* Describe(Status status);

    private:
        class State;
        std::unique_ptr<State> g_state;
    };

    inline Settings g_settings;
    inline std::unique_ptr<Engine> g_engine;
}
