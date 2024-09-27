#pragma once

#include "../vk.h"
#include "../type.h"
#include "../pointer.h"
#include "../deleter/deleter.h"

namespace cppvk {

  class CommandBufferBuilder :
  Noncopyable, Nondynamicallocation{
  private:
    VkCommandBufferAllocateInfo m_info;
    DeviceRef m_refDevice;
    CommandPoolRef m_refCommandPool;
  public:

    explicit CommandBufferBuilder(DeviceRef refDevice) : m_refDevice(refDevice){
      m_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      m_info.pNext = VK_NULL_HANDLE;
      m_info.commandBufferCount = 0;
    }
    ~CommandBufferBuilder() = default;

    CommandBufferBuilder& level(const VkCommandBufferLevel& level) {
      m_info.level = level;
      return *this;
    }

    CommandBufferBuilder& commandPool(CommandPoolRef refCommandPool) {
      m_refCommandPool = refCommandPool;
      return *this;
    }

    CommandBufferBuilder& commandBufferCount (const uint32_t count) {
      m_info.commandBufferCount = count;
      return *this;
    }

    CommandBufferPtr create() {
      auto pCmdPool = this->m_refCommandPool.lock();
      auto pDevice = this->m_refDevice.lock();
      if (pDevice && pCmdPool ) {
        this->m_info.commandPool = pCmdPool.get();
        auto buffer = new std::vector<VkCommandBuffer>(m_info.commandBufferCount);
        cppvk::checkVk(vkAllocateCommandBuffers(pDevice.get(), &m_info, buffer->data()));
        return CommandBufferPtr(buffer, cppvk::CommandBufferDeleter(std::make_shared<deivce_and_commandpool>(pDevice, pCmdPool), nullptr));
      }
      throw std::runtime_error("Failed to create CommandBuffers");
    }

  };

}