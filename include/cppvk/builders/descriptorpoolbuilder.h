#pragma once

#include "../vk.h"
#include "../pointer.h"
#include "../common.h"
#include "../deleter/deleter.h"

namespace cppvk {
  class DescriptorPoolBuilder :
    Noncopyable, Nondynamicallocation  {
    private:
      DeviceRef m_refDevice;
      VkDescriptorPoolCreateInfo m_info;
      std::vector<VkDescriptorPoolSize> m_sizeList;

      void poolSelectTypeSize(VkDescriptorType type, std::function<void(uint32_t&)> update, const int offset = 0, const unsigned int count = 0) {
        foreach<VkDescriptorPoolSize>(m_sizeList, [=](VkDescriptorPoolSize& info){
          info.type = type;
          update(info.descriptorCount);
        }, offset, count);
      }

    public:
      explicit DescriptorPoolBuilder(DeviceRef device) : m_refDevice(device) {
        m_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        m_info.flags = 0;
        m_info.pNext = nullptr;
      }
      DescriptorPoolBuilder() = delete;
      ~DescriptorPoolBuilder() = default;

      cppvk::DescriptorPoolPtr create(AllocationCallbacksPtr callbacks = nullptr) {
        containerToCPtr(m_info.poolSizeCount, &m_info.pPoolSizes, m_sizeList);
        if(auto pDevice = m_refDevice.lock()) {
          VkDescriptorPool pool;
          checkVk(vkCreateDescriptorPool(pDevice.get(), &m_info, callbacks ? callbacks.get() : VK_NULL_HANDLE, &pool));
          return DescriptorPoolPtr(pool, DescriptorPoolDeleter(pDevice, callbacks));
        }
        throw std::runtime_error("Failed to DescriptorPool.");
      }

      DescriptorPoolBuilder& poolSizeCount(const uint32_t count) {
        m_sizeList.resize(count);
        return *this;
      }

      DescriptorPoolBuilder&  poolSize(std::function<void(VkDescriptorPoolSize&)> create, const int offset = 0, const unsigned int count = 0) {
        foreach(m_sizeList, create, offset, count);
        return *this;
      }

      DescriptorPoolBuilder& poolSizeUniformBuffer(std::function<void(uint32_t&)> update, const int offset = 0, const unsigned int count = 0) {
        this->poolSelectTypeSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, update, offset, count);
        return *this;
      }


      DescriptorPoolBuilder& maxSets(const uint32_t sets) {
        m_info.maxSets = sets;
        return *this;
      }

      DescriptorPoolBuilder& flags(const VkDescriptorPoolCreateFlags flags) {
        m_info.flags = flags;
        return *this;
      }

  };
}