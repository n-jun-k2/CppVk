#pragma once

#include "../vk.h"
#include "../pointer.h"
#include "../common.h"
#include "../deleter/deleter.h"

namespace cppvk {
  class DescriptorSetLayoutBuilder:
    Noncopyable, Nondynamicallocation  {
      private:
        VkDescriptorSetLayoutCreateInfo m_info;
        DeviceRef m_refDevice;
        DescriptorSetLayoutPoolRef m_refPool;
        std::vector<VkDescriptorSetLayoutBinding> m_bindingList;

      public:
        explicit DescriptorSetLayoutBuilder(DeviceRef device) : m_refDevice(device) {
          m_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
          m_info.flags = 0;
          m_info.pNext = nullptr;
        }
        DescriptorSetLayoutBuilder() = delete;
        ~DescriptorSetLayoutBuilder() = default;

        cppvk::DescriptorSetLayoutPtr create(AllocationCallbacksPtr callbacks = nullptr) {
          auto pDevice = m_refDevice.lock();
          auto pPool = m_refPool.lock();

          containerToCPtr(m_info.bindingCount, &m_info.pBindings, m_bindingList);

          if (pDevice && pPool) {
            VkDescriptorSetLayout layout;
            checkVk(vkCreateDescriptorSetLayout(pDevice.get(), &m_info, callbacks ? callbacks.get() : VK_NULL_HANDLE, &layout));
            return pPool->getInstance(DescriptorSetLayoutDeleter(pDevice, callbacks), layout);
          }
          throw std::runtime_error("Fialed to create DescritptorSetLayout");
        }

        DescriptorSetLayoutBuilder& bindingCount(const uint32_t count) {
          m_bindingList.resize(count);
          return *this;
        }

        DescriptorSetLayoutBuilder& bindings(std::function<void(VkDescriptorSetLayoutBinding&)> create, const uint32_t offset = 0, const unsigned int count = 0) {
          foreach(m_bindingList, create, offset, count);
          return *this;
        }

        DescriptorSetLayoutBuilder& pool(DescriptorSetLayoutPoolPtr ref) {
          m_refPool = ref;
          return *this;
        }

        DescriptorSetLayoutBuilder& flags(VkDescriptorSetLayoutCreateFlags flag) {
          m_info.flags = flag;
          return *this;
        }

  };
}