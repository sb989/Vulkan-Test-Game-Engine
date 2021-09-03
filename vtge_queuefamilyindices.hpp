#ifndef __VTGE_QUEUEFAMILYINDICES_HPP__
#define __VTGE_QUEUEFAMILYINDICES_HPP__
#include <optional>

struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
        std::optional<uint32_t> transferFamily;
        bool isComplete(){
            return graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value();
        }
};

#endif