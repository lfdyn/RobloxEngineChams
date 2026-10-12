#include "chams.h"
#include "layout.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>


namespace chams
{
    namespace
    {
        class Range final
        {
        public:
            std::uint64_t g_begin = 0;
            std::uint64_t g_end = 0;

            bool Valid(std::size_t stride, std::size_t limit) const
            {
                return g_end >= g_begin && (g_end - g_begin) % stride == 0 && (g_end - g_begin) / stride <= limit;
            }

            bool operator==(const Range&) const = default;
        };

        class Table final
        {
        public:
            std::uint64_t g_depth = 0;
            std::uint64_t g_active = 0;
            std::uint64_t g_fill = 0;
            std::uint64_t g_outline = 0;
            std::uint64_t g_next = 0;

            bool operator==(const Table&) const = default;
        };

        class Slot final
        {
        public:
            std::uint32_t g_depth = 0;
            std::uint8_t g_active = 0;
            std::array<float, 4> g_fill{};
            std::array<float, 4> g_outline{};

            bool operator==(const Slot&) const = default;
        };

        class Lease final
        {
        public:
            Table g_table;
            Slot g_original;
            Slot g_written;
        };

        class Entity final
        {
        public:
            std::uint64_t g_player = 0;
            std::uint64_t g_model = 0;
            std::uint64_t g_humanoid = 0;
            std::uint64_t g_part = 0;
            std::uint64_t g_cluster = 0;
            std::uint64_t g_address = 0;
            std::array<std::uint64_t, 2> g_geometry{};
            std::array<std::uint64_t, 2> g_techniques{};
            Range g_indices;

            bool operator==(const Entity&) const = default;
        };

        Slot Appearance(const Settings& settings)
        {
            Slot result;
            result.g_depth = settings.g_through_walls ? 0 : 1;
            result.g_active = 1;
            for (std::size_t index = 0; index < 4; ++index)
            {
                const auto fill = settings.g_fill[index];
                const auto outline = settings.g_outline[index];
                result.g_fill[index] = std::isfinite(fill) ? std::clamp(fill, 0.0f, 1.0f) : 0.0f;
                result.g_outline[index] = std::isfinite(outline) ? std::clamp(outline, 0.0f, 1.0f) : 0.0f;
            }
            return result;
        }
    }

    class Engine::State final
    {
    public:
        State(std::shared_ptr<Memory> memory, std::uint64_t base) : g_memory(std::move(memory)), g_base(base)
        {
        }

        bool Compatible() const
        {
            std::uint16_t magic = 0;
            std::uint32_t header = 0;
            std::uint32_t signature = 0;
            std::uint32_t timestamp = 0;
            std::uint32_t size = 0;
            std::uint64_t draw = 0;
            std::uint64_t submit = 0;
            return g_memory && g_memory->Read(g_base, magic) && magic == 0x5A4D
                && g_memory->Read(g_base + 0x3C, header) && header >= 0x40 && header <= 0x1000
                && g_memory->Read(g_base + header, signature) && signature == 0x4550
                && g_memory->Read(g_base + header + 8, timestamp) && timestamp == layout::g_timestamp
                && g_memory->Read(g_base + header + 0x50, size) && size == layout::g_image_size
                && g_memory->Read(g_base + layout::g_entity_vtable + 16, draw) && draw == g_base + 0x39AB5F0
                && g_memory->Read(g_base + layout::g_entity_vtable + 40, submit) && submit == g_base + 0x39ABB40;
        }

        bool Equals(std::uint64_t address, std::uint64_t value) const
        {
            std::uint64_t current = 0;
            return g_memory->Read(address, current) && current == value;
        }

        bool Children(std::uint64_t instance, std::vector<std::uint64_t>& result) const
        {
            result.clear();
            std::uint64_t container = 0;
            Range range;
            if (!g_memory->Read(instance + layout::g_children, container))
            {
                return false;
            }
            if (!container)
            {
                return true;
            }
            if (!g_memory->Read(container, range) || !range.Valid(16, 4096))
            {
                return false;
            }
            std::vector<std::array<std::uint64_t, 2>> entries((range.g_end - range.g_begin) / 16);
            if (!entries.empty() && !g_memory->ReadBytes(range.g_begin, entries.data(), entries.size() * 16))
            {
                return false;
            }
            Range checked;
            if (!g_memory->Read(container, checked) || checked != range || !Equals(instance + layout::g_children, container))
            {
                return false;
            }
            for (const auto& entry : entries)
            {
                if (entry[0])
                {
                    result.push_back(entry[0]);
                }
            }
            return true;
        }

        std::string ClassName(std::uint64_t instance) const
        {
            std::uint64_t descriptor = 0;
            std::uint64_t string = 0;
            std::uint64_t length = 0;
            std::uint64_t capacity = 0;
            if (!g_memory->Read(instance + layout::g_class_descriptor, descriptor) || !descriptor
                || !g_memory->Read(descriptor + layout::g_class_name, string) || !string
                || !g_memory->Read(string + 16, length) || length > 96
                || !g_memory->Read(string + 24, capacity) || capacity < length)
            {
                return {};
            }
            auto data = string;
            if (capacity >= 16 && !g_memory->Read(string, data))
            {
                return {};
            }
            std::string result(static_cast<std::size_t>(length), char{});
            if (length && !g_memory->ReadBytes(data, result.data(), result.size()))
            {
                return {};
            }
            return result;
        }

        bool SessionLive() const
        {
            std::uint64_t fake = 0;
            std::uint8_t merged = 0;
            return g_engine && g_manager && g_datamodel
                && Equals(g_base + layout::g_visual_engine_pointer, g_engine)
                && Equals(g_engine + layout::g_manager, g_manager)
                && g_memory->Read(g_base + layout::g_fake_datamodel_pointer, fake) && fake
                && Equals(fake + layout::g_fake_datamodel_to_datamodel, g_datamodel)
                && g_memory->Read(g_base + layout::g_merge_flag, merged) && merged == g_merged;
        }

        bool Tables(std::vector<Table>& tables) const
        {
            tables.clear();
            if (!SessionLive())
            {
                return false;
            }
            if (g_merged)
            {
                tables.push_back({ g_manager + layout::g_global_depth, g_manager + layout::g_global_active,
                    g_manager + layout::g_global_fill, g_manager + layout::g_global_outline, g_manager + layout::g_global_next });
                return true;
            }
            tables.push_back({ g_manager + layout::g_fallback_depth, g_manager + layout::g_fallback_active,
                g_manager + layout::g_fallback_fill, g_manager + layout::g_fallback_outline, 0 });
            Range contexts;
            if (!g_memory->Read(g_manager + layout::g_contexts, contexts) || !contexts.Valid(layout::g_context_stride, 32))
            {
                return false;
            }
            for (auto context = contexts.g_begin; context < contexts.g_end; context += layout::g_context_stride)
            {
                tables.push_back({ context + layout::g_context_depth, context + layout::g_context_active,
                    context + layout::g_context_fill, context + layout::g_context_outline, context });
            }
            Range checked;
            return g_memory->Read(g_manager + layout::g_contexts, checked) && contexts == checked;
        }

        bool ReadSlot(const Table& table, std::uint32_t id, Slot& slot) const
        {
            return g_memory->Read(table.g_depth + id * 4, slot.g_depth)
                && g_memory->Read(table.g_active + id, slot.g_active)
                && g_memory->Read(table.g_fill + id * 16, slot.g_fill)
                && g_memory->Read(table.g_outline + id * 16, slot.g_outline);
        }

        bool Unallocated(const Table& table, std::uint32_t id) const
        {
            std::uint32_t next = 0;
            return !table.g_next || (g_memory->Read(table.g_next, next) && next <= id);
        }

        template <typename T>
        bool Replace(std::uint64_t address, const T& expected, const T& desired)
        {
            T current{};
            return g_memory->Read(address, current) && current == expected && g_memory->Write(address, desired);
        }

        void RestoreSlot(const Lease& lease)
        {
            std::vector<Table> tables;
            if (!Tables(tables) || std::find(tables.begin(), tables.end(), lease.g_table) == tables.end()
                || !Unallocated(lease.g_table, g_id))
            {
                return;
            }
            const auto& table = lease.g_table;
            Slot current;
            if (ReadSlot(table, g_id, current) && current == lease.g_written)
            {
                Replace(table.g_active + g_id, lease.g_written.g_active, lease.g_original.g_active);
                Replace(table.g_depth + g_id * 4, lease.g_written.g_depth, lease.g_original.g_depth);
                Replace(table.g_fill + g_id * 16, lease.g_written.g_fill, lease.g_original.g_fill);
                Replace(table.g_outline + g_id * 16, lease.g_written.g_outline, lease.g_original.g_outline);
            }
        }

        bool Registered(std::array<bool, 256>& used) const
        {
            used.fill(false);
            if (g_merged)
            {
                return true;
            }
            std::uint64_t head = 0;
            std::uint64_t node = 0;
            if (!g_memory->Read(g_manager + 24, head) || !head || !g_memory->Read(head, node))
            {
                return false;
            }
            for (std::size_t count = 0; node != head; ++count)
            {
                std::uint64_t entry = 0;
                std::uint32_t id = 0;
                if (count >= 4096 || !node || !g_memory->Read(node + 24, entry) || !entry
                    || !g_memory->Read(entry + 112, id) || !g_memory->Read(node, node))
                {
                    return false;
                }
                if (id < used.size())
                {
                    used[id] = true;
                }
            }
            return Equals(g_manager + 24, head);
        }

        bool Acquire(const std::vector<Table>& tables, const Slot& appearance)
        {
            std::array<bool, 256> used{};
            if (!Registered(used))
            {
                return false;
            }
            for (std::uint32_t candidate = 254; candidate >= 192; --candidate)
            {
                if (used[candidate])
                {
                    continue;
                }
                std::vector<Lease> leases;
                for (const auto& table : tables)
                {
                    Slot original;
                    if (!Unallocated(table, candidate) || !ReadSlot(table, candidate, original) || original.g_active
                        || original.g_fill[3] != 0.0f || original.g_outline[3] != 0.0f)
                    {
                        break;
                    }
                    leases.push_back({ table, original, appearance });
                }
                if (leases.size() != tables.size())
                {
                    continue;
                }
                g_id = candidate;
                for (const auto& lease : leases)
                {
                    if (!SessionLive())
                    {
                        Stop();
                        return false;
                    }
                    const auto& table = lease.g_table;
                    auto pending = lease;
                    pending.g_written = lease.g_original;
                    g_leases.push_back(pending);
                    auto& written = g_leases.back().g_written;
                    if (!Replace(table.g_depth + g_id * 4, written.g_depth, appearance.g_depth))
                    {
                        Stop();
                        return false;
                    }
                    written.g_depth = appearance.g_depth;
                    if (!Replace(table.g_fill + g_id * 16, written.g_fill, appearance.g_fill))
                    {
                        Stop();
                        return false;
                    }
                    written.g_fill = appearance.g_fill;
                    if (!Replace(table.g_outline + g_id * 16, written.g_outline, appearance.g_outline))
                    {
                        Stop();
                        return false;
                    }
                    written.g_outline = appearance.g_outline;
                    if (!Replace(table.g_active + g_id, written.g_active, appearance.g_active))
                    {
                        Stop();
                        return false;
                    }
                    written.g_active = appearance.g_active;
                }
                return true;
            }
            return false;
        }

        bool LeasesLive(const std::vector<Table>& tables) const
        {
            std::array<bool, 256> used{};
            if (tables.size() != g_leases.size() || !Registered(used) || used[g_id])
            {
                return false;
            }
            for (std::size_t index = 0; index < tables.size(); ++index)
            {
                Slot current;
                if (tables[index] != g_leases[index].g_table || !Unallocated(tables[index], g_id)
                    || !ReadSlot(tables[index], g_id, current) || current != g_leases[index].g_written)
                {
                    return false;
                }
            }
            return true;
        }

        bool SetAppearance(const Slot& appearance)
        {
            for (auto& lease : g_leases)
            {
                const auto& table = lease.g_table;
                auto& written = lease.g_written;
                if (written == appearance)
                {
                    continue;
                }
                if (!SessionLive() || !Unallocated(table, g_id))
                {
                    return false;
                }
                if (!Replace(table.g_depth + g_id * 4, written.g_depth, appearance.g_depth))
                {
                    return false;
                }
                written.g_depth = appearance.g_depth;
                if (!Replace(table.g_fill + g_id * 16, written.g_fill, appearance.g_fill))
                {
                    return false;
                }
                written.g_fill = appearance.g_fill;
                if (!Replace(table.g_outline + g_id * 16, written.g_outline, appearance.g_outline))
                {
                    return false;
                }
                written.g_outline = appearance.g_outline;
            }
            return true;
        }

        bool Live(const Entity& entity) const
        {
            if (!SessionLive() || !Equals(entity.g_player + layout::g_model_instance, entity.g_model)
                || !Equals(entity.g_humanoid + layout::g_parent, entity.g_model)
                || !Equals(entity.g_part + layout::g_cluster_node, entity.g_cluster + layout::g_cluster_binding)
                || !Equals(entity.g_cluster, g_base + layout::g_cluster_vtable)
                || !Equals(entity.g_cluster + layout::g_cluster_engine, g_engine)
                || !Equals(entity.g_cluster + layout::g_cluster_humanoid, entity.g_humanoid)
                || !Equals(entity.g_address, g_base + layout::g_entity_vtable)
                || !Equals(entity.g_address + layout::g_entity_cluster, entity.g_cluster))
            {
                return false;
            }
            std::array<std::uint64_t, 2> geometry{};
            std::array<std::uint64_t, 2> techniques{};
            Range indices;
            Range entities;
            if (!g_memory->Read(entity.g_address + layout::g_entity_geometry, geometry) || geometry != entity.g_geometry
                || !g_memory->Read(entity.g_address + layout::g_entity_techniques, techniques) || techniques != entity.g_techniques
                || !g_memory->Read(entity.g_address + layout::g_entity_indices, indices) || indices != entity.g_indices
                || !g_memory->Read(entity.g_cluster + layout::g_cluster_entities, entities) || !entities.Valid(8, 512))
            {
                return false;
            }
            std::vector<std::uint64_t> addresses((entities.g_end - entities.g_begin) / 8);
            return !addresses.empty() && g_memory->ReadBytes(entities.g_begin, addresses.data(), addresses.size() * 8)
                && std::find(addresses.begin(), addresses.end(), entity.g_address) != addresses.end();
        }

        bool HasTechnique(const Entity& entity) const
        {
            Range techniques;
            if (!g_memory->Read(entity.g_techniques[0], techniques) || !techniques.Valid(136, 256))
            {
                return false;
            }
            for (auto address = techniques.g_begin; address < techniques.g_end; address += 136)
            {
                std::uint32_t pass = 0;
                if (!g_memory->Read(address + 4, pass))
                {
                    return false;
                }
                if (pass >= 9 && pass <= 12)
                {
                    return true;
                }
            }
            return false;
        }

        bool Collect(std::unordered_map<std::uint64_t, Entity>& result)
        {
            std::vector<std::uint64_t> children;
            if (!Children(g_datamodel, children))
            {
                return false;
            }
            std::uint64_t players = 0;
            for (const auto child : children)
            {
                if (ClassName(child) == "Players")
                {
                    players = child;
                    break;
                }
            }
            std::uint64_t local = 0;
            if (!players || !g_memory->Read(players + layout::g_local_player, local) || !local || !Children(players, children))
            {
                return false;
            }
            std::unordered_set<std::uint64_t> clusters;
            for (const auto player : children)
            {
                if (player == local || ClassName(player) != "Player")
                {
                    continue;
                }
                Entity entry;
                entry.g_player = player;
                std::vector<std::uint64_t> parts;
                if (!g_memory->Read(player + layout::g_model_instance, entry.g_model) || !entry.g_model || !Children(entry.g_model, parts))
                {
                    continue;
                }
                for (const auto part : parts)
                {
                    if (ClassName(part) == "Humanoid")
                    {
                        entry.g_humanoid = part;
                        break;
                    }
                }
                if (!entry.g_humanoid)
                {
                    continue;
                }
                for (const auto part : parts)
                {
                    const auto name = ClassName(part);
                    if (name != "Part" && name != "MeshPart" && name != "UnionOperation" && name != "WedgePart" && name != "CornerWedgePart")
                    {
                        continue;
                    }
                    std::uint64_t node = 0;
                    if (!g_memory->Read(part + layout::g_cluster_node, node) || node < 0x10000)
                    {
                        continue;
                    }
                    entry.g_part = part;
                    entry.g_cluster = node - layout::g_cluster_binding;
                    if (!clusters.insert(entry.g_cluster).second || !Equals(entry.g_cluster, g_base + layout::g_cluster_vtable)
                        || !Equals(entry.g_cluster + layout::g_cluster_engine, g_engine)
                        || !Equals(entry.g_cluster + layout::g_cluster_humanoid, entry.g_humanoid))
                    {
                        continue;
                    }
                    Range entities;
                    if (!g_memory->Read(entry.g_cluster + layout::g_cluster_entities, entities) || !entities.Valid(8, 512))
                    {
                        continue;
                    }
                    for (auto address = entities.g_begin; address < entities.g_end; address += 8)
                    {
                        if (g_memory->Read(address, entry.g_address) && entry.g_address
                            && Equals(entry.g_address, g_base + layout::g_entity_vtable)
                            && g_memory->Read(entry.g_address + layout::g_entity_geometry, entry.g_geometry)
                            && g_memory->Read(entry.g_address + layout::g_entity_techniques, entry.g_techniques)
                            && g_memory->Read(entry.g_address + layout::g_entity_indices, entry.g_indices)
                            && HasTechnique(entry))
                        {
                            result.emplace(entry.g_address, entry);
                        }
                    }
                }
            }
            return SessionLive();
        }

        void Stop()
        {
            for (const auto& [address, entity] : g_entities)
            {
                if (Live(entity))
                {
                    Replace(address + layout::g_entity_highlight, g_id, std::uint32_t{0});
                }
            }
            g_entities.clear();
            for (const auto& lease : g_leases)
            {
                RestoreSlot(lease);
            }
            g_leases.clear();
            g_id = 0;
            g_player_count = 0;
            g_status = Status::Disabled;
        }

        void Update(const Settings& settings)
        {
            if (!settings.g_enabled)
            {
                Stop();
                return;
            }
            if (!Compatible())
            {
                Stop();
                g_status = Status::UnsupportedVersion;
                return;
            }
            std::uint64_t engine = 0;
            std::uint64_t manager = 0;
            std::uint64_t fake = 0;
            std::uint64_t datamodel = 0;
            std::uint8_t merged = 0;
            if (!g_memory->Read(g_base + layout::g_visual_engine_pointer, engine) || !engine
                || !g_memory->Read(engine + layout::g_manager, manager) || !manager
                || !g_memory->Read(g_base + layout::g_fake_datamodel_pointer, fake) || !fake
                || !g_memory->Read(fake + layout::g_fake_datamodel_to_datamodel, datamodel) || !datamodel
                || !g_memory->Read(g_base + layout::g_merge_flag, merged))
            {
                Stop();
                g_status = Status::WaitingForGame;
                return;
            }
            if (engine != g_engine || manager != g_manager || datamodel != g_datamodel || merged != g_merged)
            {
                Stop();
                g_engine = engine;
                g_manager = manager;
                g_datamodel = datamodel;
                g_merged = merged;
            }
            std::vector<Table> tables;
            std::unordered_map<std::uint64_t, Entity> entities;
            if (!Tables(tables) || !Collect(entities))
            {
                Stop();
                g_status = Status::MemoryUnavailable;
                return;
            }
            if (entities.empty())
            {
                Stop();
                g_status = Status::WaitingForCharacters;
                return;
            }
            const auto appearance = Appearance(settings);
            if (g_id && !LeasesLive(tables))
            {
                Stop();
                g_status = Status::WaitingForCharacters;
                return;
            }
            if (!g_id && !Acquire(tables, appearance))
            {
                g_status = Status::NoAvailableIds;
                return;
            }
            if (!SetAppearance(appearance))
            {
                Stop();
                g_status = Status::MemoryUnavailable;
                return;
            }
            for (auto iterator = g_entities.begin(); iterator != g_entities.end();)
            {
                const auto found = entities.find(iterator->first);
                if (found == entities.end() || found->second != iterator->second)
                {
                    if (Live(iterator->second))
                    {
                        Replace(iterator->first + layout::g_entity_highlight, g_id, std::uint32_t{0});
                    }
                    iterator = g_entities.erase(iterator);
                }
                else
                {
                    ++iterator;
                }
            }
            for (const auto& [address, entity] : entities)
            {
                std::uint32_t current = 0;
                if (!g_memory->Read(address + layout::g_entity_highlight, current))
                {
                    continue;
                }
                const auto owned = g_entities.contains(address);
                if (owned && current == g_id)
                {
                    continue;
                }
                if (owned)
                {
                    g_entities.erase(address);
                }
                if (!current && Live(entity) && Replace(address + layout::g_entity_highlight, std::uint32_t{0}, g_id))
                {
                    g_entities.emplace(address, entity);
                }
            }
            std::unordered_set<std::uint64_t> players;
            for (const auto& [address, entity] : g_entities)
            {
                players.insert(entity.g_player);
            }
            g_player_count = players.size();
            g_status = g_entities.empty() ? Status::WaitingForCharacters : Status::Active;
        }

        std::shared_ptr<Memory> g_memory;
        std::uint64_t g_base = 0;
        std::uint64_t g_engine = 0;
        std::uint64_t g_manager = 0;
        std::uint64_t g_datamodel = 0;
        std::uint8_t g_merged = 0;
        std::uint32_t g_id = 0;
        Status g_status = Status::WaitingForGame;
        std::size_t g_player_count = 0;
        std::vector<Lease> g_leases;
        std::unordered_map<std::uint64_t, Entity> g_entities;
    };

    Engine::Engine(std::shared_ptr<Memory> memory, std::uint64_t base) : g_state(std::make_unique<State>(std::move(memory), base))
    {
    }

    Engine::~Engine()
    {
        Stop();
    }

    void Engine::Update(const Settings& settings)
    {
        g_state->Update(settings);
    }

    void Engine::Stop()
    {
        g_state->Stop();
    }

    Status Engine::GetStatus() const
    {
        return g_state->g_status;
    }

    std::size_t Engine::GetEntityCount() const
    {
        return g_state->g_entities.size();
    }

    std::size_t Engine::GetPlayerCount() const
    {
        return g_state->g_player_count;
    }

    const char* Engine::Describe(Status status)
    {
        switch (status)
        {
        case Status::Disabled:
            return "disabled";
        case Status::UnsupportedVersion:
            return "unsupported client build";
        case Status::WaitingForGame:
            return "waiting for game";
        case Status::WaitingForCharacters:
            return "waiting for eligible characters";
        case Status::NoAvailableIds:
            return "no uncontested highlight slot";
        case Status::MemoryUnavailable:
            return "scene unavailable";
        case Status::Active:
            return "native highlight IDs applied";
        }
        return "unknown";
    }
}
