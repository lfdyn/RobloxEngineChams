#pragma once

#include <cstdint>

namespace chams::layout
{
    inline constexpr std::uint64_t g_children = 0x78;
    inline constexpr std::uint64_t g_class_descriptor = 0x18;
    inline constexpr std::uint64_t g_class_name = 0x8;
    inline constexpr std::uint64_t g_visual_engine_pointer = 0x8656E40;
    inline constexpr std::uint64_t g_fake_datamodel_pointer = 0x8BCFD50;
    inline constexpr std::uint64_t g_fake_datamodel_to_datamodel = 0x1F8;
    inline constexpr std::uint64_t g_model_instance = 0x288;
    inline constexpr std::uint64_t g_parent = 0x68;
    inline constexpr std::uint64_t g_local_player = 0x120;
    inline constexpr std::uint32_t g_timestamp = 0xB43CFB4C;
    inline constexpr std::uint32_t g_image_size = 0x9127000;
    inline constexpr std::uint64_t g_merge_flag = 0x8698948;
    inline constexpr std::uint64_t g_manager = 0xC00;
    inline constexpr std::uint64_t g_cluster_node = 0x180;
    inline constexpr std::uint64_t g_cluster_binding = 0x90;
    inline constexpr std::uint64_t g_cluster_vtable = 0x6D6FC88;
    inline constexpr std::uint64_t g_entity_vtable = 0x6D70CE8;
    inline constexpr std::uint64_t g_cluster_engine = 0x10;
    inline constexpr std::uint64_t g_cluster_entities = 0x48;
    inline constexpr std::uint64_t g_cluster_humanoid = 0xF8;
    inline constexpr std::uint64_t g_entity_cluster = 0x8;
    inline constexpr std::uint64_t g_entity_geometry = 0x20;
    inline constexpr std::uint64_t g_entity_techniques = 0x70;
    inline constexpr std::uint64_t g_entity_indices = 0x80;
    inline constexpr std::uint64_t g_entity_highlight = 0xB4;
    inline constexpr std::uint64_t g_contexts = 0x100;
    inline constexpr std::uint64_t g_context_stride = 9504;
    inline constexpr std::uint64_t g_fallback_depth = 312;
    inline constexpr std::uint64_t g_fallback_active = 1336;
    inline constexpr std::uint64_t g_fallback_fill = 1592;
    inline constexpr std::uint64_t g_fallback_outline = 5688;
    inline constexpr std::uint64_t g_context_depth = 32;
    inline constexpr std::uint64_t g_context_active = 1056;
    inline constexpr std::uint64_t g_context_fill = 1312;
    inline constexpr std::uint64_t g_context_outline = 5408;
    inline constexpr std::uint64_t g_global_next = 9896;
    inline constexpr std::uint64_t g_global_depth = 9900;
    inline constexpr std::uint64_t g_global_active = 10924;
    inline constexpr std::uint64_t g_global_fill = 11180;
    inline constexpr std::uint64_t g_global_outline = 15276;
}
