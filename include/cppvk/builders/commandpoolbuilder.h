#pragma once

#include "../vk.h"
#include "../type.h"
#include "../pointer.h"
#include "../deleter/deleter.h"


namespace cppvk {

  class CommandPoolBuilder :
  Noncopyable, Nondynamicallocation {
  private:
    VkCommandPoolCreateInfo m_info;
    DeviceRef m_refDevice;

  public:

    explicit CommandPoolBuilder(DeviceRef refDevice) : m_refDevice(refDevice) {
      m_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
      m_info.pNext = VK_NULL_HANDLE;
      m_info.flags = 0;
    }
    CommandPoolBuilder() = delete;

    CommandPoolPtr create(AllocationCallbacksPtr callbacks = nullptr) {
      if (auto pLogicalDevice = this->m_refDevice.lock()) {
        VkCommandPool vkCommandPool;
        checkVk(vkCreateCommandPool(pLogicalDevice.get(), &m_info, callbacks ? callbacks.get() : VK_NULL_HANDLE, &vkCommandPool));
        return CommandPoolPtr(vkCommandPool, CommandPoolDeleter(pLogicalDevice, callbacks));
      } else {
        throw std::runtime_error("Failed to create CommandPool");
      }
    }

    /// <summary>
    /// flags is a bitmask of VkCommandPoolCreateFlagBits indicating usage behavior for the pool and command buffers allocated from it.
    /// </summary>
    /// <param name="flag"></param>
    /// <returns></returns>
    CommandPoolBuilder& flags(const VkCommandPoolCreateFlags flag) {
      m_info.flags = flag;
      return *this;
    }

    /// <summary>
    /// eueFamilyIndex designates a queue family as described in section Queue Family Properties.
    ///  All command buffers allocated from this command pool must be submitted on queues from the same queue family.
    /// </summary>
    /// <param name="value"></param>
    /// <returns></returns>
    CommandPoolBuilder& queueFamilyIndices(const uint32_t value){
      m_info.queueFamilyIndex = value;
      return *this;
    }
  };
}