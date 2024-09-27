#pragma once

#include "../vk.h"
#include "../pointer.h"
#include "../common.h"
#include "../deleter/deleter.h"

namespace cppvk {
  class DescriptorSetBuilder : Noncopyable, Nondynamicallocation {
    private:
      VkDescriptorSetAllocateInfo m_info;
      DeviceRef m_refDevice;
      DescriptorPoolRef m_refDescriptorPool;
      DescriptorSetLayoutPoolRef m_refDescriptorSetLayoutPool;
    public:
      explicit DescriptorSetBuilder(DeviceRef device) : m_refDevice(device) {
        m_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        m_info.pNext = VK_NULL_HANDLE;
      }
      DescriptorSetBuilder() = delete;
      ~DescriptorSetBuilder() = default;

      DescriptorSetBuilder& descriptorPool(DescriptorPoolPtr pool) {
        m_refDescriptorPool = pool;
        return *this;
      }

      DescriptorSetBuilder& layoutpool(DescriptorSetLayoutPoolRef pool) {
        m_refDescriptorSetLayoutPool = pool;
        return *this;
      }

      DescriptorSetPtr create() {
        auto pDevice = this->m_refDevice.lock();
        auto pPool = this->m_refDescriptorPool.lock();
        auto pLayoutPool = this->m_refDescriptorSetLayoutPool.lock();
        if(pDevice && pPool && pLayoutPool){
          m_info.descriptorPool = pPool.get();
          m_info.descriptorSetCount = static_cast<uint32_t>(pLayoutPool->size());
          m_info.pSetLayouts = nullptr;
          if(m_info.descriptorSetCount > 0) {
            m_info.pSetLayouts = pLayoutPool->data();
          }
          auto buffer = new std::vector<VkDescriptorSet>(m_info.descriptorSetCount);
          checkVk(vkAllocateDescriptorSets(pDevice.get(), &m_info, buffer->data()));
          return DescriptorSetPtr(buffer, cppvk::DescriptorSetDeleter(std::make_shared<device_and_descriptorpool>(pDevice, pPool), nullptr));
        }
        throw std::runtime_error("Failed to create descriptorset");
      }

  };
}