#pragma once

#include "../vk.h"
#include "../type.h"

namespace cppvk {
  class WriteInfo: public InfoWrap<VkWriteDescriptorSet> {
    public:
      explicit WriteInfo(VkWriteDescriptorSet& info)
      :InfoWrap<VkWriteDescriptorSet>::InfoWrap(info){
        m_info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        m_info.pNext = NULL;
        m_info.pImageInfo = VK_NULL_HANDLE;
        m_info.pBufferInfo = VK_NULL_HANDLE;
        m_info.pTexelBufferView = VK_NULL_HANDLE;
      }
      WriteInfo() = delete;
  };
}