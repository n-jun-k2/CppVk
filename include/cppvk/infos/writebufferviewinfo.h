#pragma once

#include "writeinfo.h"
#include "../common.h"

namespace cppvk {
  class WriteBufferViewInfo : public WriteInfo {
    private:
      using InfoList = std::vector<VkBufferView>;
      InfoList& m_infoList;
    public:
      explicit WriteBufferViewInfo(VkWriteDescriptorSet& info, InfoList& images)
      : WriteInfo::WriteInfo(info), m_infoList(images) {}

      WriteBufferViewInfo& type(VkDescriptorType type) {
        m_info.descriptorType = type;
        return *this;
      }

      WriteBufferViewInfo& count(const uint32_t count) {
        m_info.descriptorCount = count;
        m_infoList.resize(count);
        return *this;
      }

      WriteBufferViewInfo& binding(const uint32_t binding) {
        m_info.dstBinding = binding;
        return *this;
      }

      WriteBufferViewInfo& arrayElement(const uint32_t element) {
        m_info.dstArrayElement = element;
        return *this;
      }

      WriteBufferViewInfo& bufferView(std::function<void(VkBufferView&)> update, const uint32_t offset = 0, const int count = 0) {
        foreach<VkBufferView>(m_infoList, update, offset, count);
        return *this;
      }
  };
}