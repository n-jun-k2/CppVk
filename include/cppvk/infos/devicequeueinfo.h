#pragma once

#include "../vk.h"
#include "../type.h"

namespace cppvk {

  class DeviceQueueCreateInfo
  : public InfoWrap<VkDeviceQueueCreateInfo> {
  public:
    DeviceQueueCreateInfo(VkDeviceQueueCreateInfo& info)
    : InfoWrap<VkDeviceQueueCreateInfo>::InfoWrap(info){
      m_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      m_info.pNext = NULL;
      m_info.flags = 0;
    }

    DeviceQueueCreateInfo& flags(VkDeviceQueueCreateFlags f) {
      m_info.flags = f;
      return *this;
    }

    DeviceQueueCreateInfo& familyIndex(const uint32_t index) {
      m_info.queueFamilyIndex = index;
      return *this;
    }

  };
}