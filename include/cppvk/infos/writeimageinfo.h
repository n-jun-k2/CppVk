#pragma once

#include "writeinfo.h"
#include "../common.h"

namespace cppvk {
  class WriteImageInfo : public WriteInfo {
    private:
      using InfoList = std::vector<VkDescriptorImageInfo>;
      InfoList& m_infoList;
    public:
      explicit WriteImageInfo(VkWriteDescriptorSet& info, InfoList& images)
      : WriteInfo::WriteInfo(info), m_infoList(images) {}

      WriteImageInfo& type(VkDescriptorType type) {
        m_info.descriptorType = type;
        return *this;
      }

      WriteImageInfo& count(const uint32_t count) {
        m_info.descriptorCount = count;
        m_infoList.resize(count);
        return *this;
      }

      WriteImageInfo& binding(const uint32_t binding) {
        m_info.dstBinding = binding;
        return *this;
      }

      WriteImageInfo& arrayElement(const uint32_t element) {
        m_info.dstArrayElement = element;
        return *this;
      }

      WriteImageInfo& imageInfo(std::function<void(VkDescriptorImageInfo&)> update, const uint32_t offset = 0, const int count = 0) {
        foreach<VkDescriptorImageInfo>(m_infoList, update, offset, count);
        return *this;
      }
  };
}