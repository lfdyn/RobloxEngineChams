#pragma once

#include <cstdint>

namespace chams::layout
{
    inline constexpr std::uint64_t g_children = 0x70;
    inline constexpr std::uint64_t g_class_descriptor = 0x18;
    inline constexpr std::uint64_t g_class_name = 0x8;
    inline constexpr std::uint64_t g_visual_engine_pointer = 0x8942E38;
    inline constexpr std::uint64_t g_fake_datamodel_pointer = 0x8EB6960;
    inline constexpr std::uint64_t g_fake_datamodel_to_datamodel = 0x1F8;
    inline constexpr std::uint64_t g_model_instance = 0x2A0;
    inline constexpr std::uint64_t g_parent = 0x60;
    inline constexpr std::uint64_t g_local_player = 0x118;
    inline constexpr std::uint32_t g_timestamp = 0x4DB2301;
    inline constexpr std::uint32_t g_image_size = 0x93E1000;
    inline constexpr std::uint64_t g_merge_flag = 0x8698948;
    inline constexpr std::uint64_t g_manager = 0xC28;
    inline constexpr std::uint64_t g_cluster_node = 0x180;
    inline constexpr std::uint64_t g_cluster_binding = 0x90;
    inline constexpr std::uint64_t g_cluster_vtable = 0x714B840;
    inline constexpr std::uint64_t g_entity_vtable = 0x714CAF0;
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
