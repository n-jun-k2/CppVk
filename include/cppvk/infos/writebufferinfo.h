#pragma once

#include "writeinfo.h"
#include "../common.h"

namespace cppvk {
  class WriteBufferInfo : public WriteInfo{
  private:
    using InfoList = std::vector<VkDescriptorBufferInfo>;
    InfoList& m_infoList;
  public:
    explicit WriteBufferInfo(VkWriteDescriptorSet& info, InfoList& bufferInfo)
    : WriteInfo::WriteInfo(info), m_infoList(bufferInfo) {}

    WriteBufferInfo& type(VkDescriptorType type) {
      m_info.descriptorType = type;
      return *this;
    }

    WriteBufferInfo& count(const uint32_t count) {
      m_info.descriptorCount = count;
      m_infoList.resize(count);
      return *this;
    }

    WriteBufferInfo& binding(const uint32_t binding) {
      m_info.dstBinding = binding;
      return *this;
    }

    WriteBufferInfo& arrayElement(const uint32_t element) {
      m_info.dstArrayElement = element;
      return *this;
    }

    WriteBufferInfo& bufferInfo(std::function<void(VkDescriptorBufferInfo&)> update, const uint32_t offset = 0, const int count = 0) {
      foreach<VkDescriptorBufferInfo>(m_infoList, update, offset, count);
      return *this;
    }

  };
}