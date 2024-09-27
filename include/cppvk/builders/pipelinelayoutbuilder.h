#pragma once

#include "../vk.h"
#include "../pointer.h"
#include "../common.h"
#include "../deleter/deleter.h"

namespace cppvk {
  class PipelineLayoutBuilder :
  Noncopyable, Nondynamicallocation {
    private:
      VkPipelineLayoutCreateInfo m_info;
      DeviceRef m_refDevice;
      DescriptorSetLayoutPoolRef m_refPool;
      std::vector<VkPushConstantRange> m_pRanges;
    public:
      explicit PipelineLayoutBuilder(DeviceRef device) : m_refDevice(device) {
        m_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        m_info.pNext = VK_NULL_HANDLE;
        m_info.flags = 0;
      }
      PipelineLayoutBuilder() = delete;
      ~PipelineLayoutBuilder() = default;

      cppvk::PipelineLayoutPtr create(AllocationCallbacksPtr callbacks = nullptr) {
        auto pPool = m_refPool.lock();
        auto pDevice = m_refDevice.lock();

        containerToCPtr(m_info.pushConstantRangeCount, &m_info.pPushConstantRanges, m_pRanges);

        if (pDevice && pPool) {

          m_info.setLayoutCount = static_cast<uint32_t>(pPool->size());
          m_info.pSetLayouts = nullptr;
          if (m_info.setLayoutCount > 0) {
            m_info.pSetLayouts = pPool->data();
          }

          VkPipelineLayout layout;
          checkVk(vkCreatePipelineLayout(pDevice.get(), &m_info,  callbacks ? callbacks.get() : VK_NULL_HANDLE, &layout));
          return PipelineLayoutPtr(layout, PipelineLayoutDeleter(pDevice, callbacks));
        }
        throw std::runtime_error("Failed to create PipelineLayout");
      }

      PipelineLayoutBuilder& flags(VkPipelineLayoutCreateFlags flag) {
        m_info.flags = flag;
        return *this;
      }

      PipelineLayoutBuilder& layoutpool(DescriptorSetLayoutPoolPtr ref) {
        m_refPool = ref;
        return *this;
      }

      PipelineLayoutBuilder& pushConstantRangeCount(const uint32_t count) {
        m_pRanges.resize(count);
        return *this;
      }

      PipelineLayoutBuilder& pushConstantRange(std::function<void(VkPushConstantRange&)> create, const uint32_t offset = 0, const unsigned int count = 0) {
        foreach(m_pRanges, create, offset, count);
        return *this;
      }

  };
}