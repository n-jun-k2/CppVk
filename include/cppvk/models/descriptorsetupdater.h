#pragma once

#include "../vk.h"
#include "../pointer.h"
#include "../common.h"

#include "../infos/writeimageinfo.h"
#include "../infos/writebufferinfo.h"
#include "../infos/writebufferviewinfo.h"

#include <variant>

namespace cppvk {

  class DescriptorSetUpdater {
    private:
      DevicePtr m_pDevice;
      DescriptorSetPtr m_pSrcDescriptor;
      DescriptorSetPtr m_pDstDescriptor;

      using vectorBuffer = std::vector<VkDescriptorBufferInfo>;
      using vectorBufferView = std::vector<VkBufferView>;
      using vectorImage = std::vector<VkDescriptorImageInfo>;
      std::vector<VkWriteDescriptorSet> m_writeList;
      std::vector<std::variant<vectorBuffer, vectorBufferView, vectorImage>> m_writeInfoList;

      std::vector<VkCopyDescriptorSet> m_copyList;

    public:
      explicit DescriptorSetUpdater(DevicePtr device): m_pDevice(device){}
      DescriptorSetUpdater() = delete;
      ~DescriptorSetUpdater() = default;

      void srcDescriptor(DescriptorSetPtr pSrc) {
        m_pSrcDescriptor = pSrc;
      }

      void dstDescriptor(DescriptorSetPtr pDst) {
        m_pDstDescriptor = pDst;
      }

      void writeSize(const uint32_t size) {
        m_writeList.resize(size);
        m_writeInfoList.resize(size);
      }

      void writeBufferData(std::function<void(WriteBufferInfo&)> update, const int offset = 0, const unsigned int count = 0U) {
        foreach<VkWriteDescriptorSet>(m_writeList, [offset, &writeInfoList = this->m_writeInfoList, update](VkWriteDescriptorSet& item)
        {
          static uint32_t idx = offset;
          writeInfoList[idx] = vectorBuffer();
          vectorBuffer& list = std::get<vectorBuffer>(writeInfoList[idx]);
          WriteBufferInfo tInfo(item, list);
          update(tInfo);
          item.pBufferInfo = list.data();
        }, offset, count);
      }

      void writeBufferViewData(std::function<void(WriteBufferViewInfo&)> update, const int offset = 0, const unsigned int count = 0U) {
        foreach<VkWriteDescriptorSet>(m_writeList, [offset, &writeInfoList = this->m_writeInfoList, update](VkWriteDescriptorSet& item)
        {
          static uint32_t idx = offset;
          writeInfoList[idx] = vectorBufferView();
          vectorBufferView& list = std::get<vectorBufferView>(writeInfoList[idx]);
          WriteBufferViewInfo tInfo(item, list);
          update(tInfo);
          item.pTexelBufferView = list.data();
        }, offset, count);
      }

      void writeImageData(std::function<void(WriteImageInfo&)> update, const int offset = 0, const unsigned int count = 0U) {
        foreach<VkWriteDescriptorSet>(m_writeList, [offset, &writeInfoList = this->m_writeInfoList, update](VkWriteDescriptorSet& item)
        {
          static uint32_t idx = offset;
          writeInfoList[idx] = vectorImage();
          vectorImage& list = std::get<vectorImage>(writeInfoList[idx]);
          WriteImageInfo tInfo(item, list);
          update(tInfo);
          item.pImageInfo = list.data();
        }, offset, count);
      }

      void copySize(const uint32_t size) {
        m_copyList.resize(size);
      }

      void copyData(std::function<void(VkCopyDescriptorSet&)> update, const int offset = 0, const unsigned int count = 0U) {
        foreach(m_copyList, update, offset, count);
      }

      void update() {

        auto writeContainerSize = 0;
        VkWriteDescriptorSet* writeData = nullptr;
        auto copyContainerSize = 0;
        VkCopyDescriptorSet* copyData = nullptr;

        if (!m_writeList.empty() && !m_pDstDescriptor->empty()) {
          foreachZip<VkWriteDescriptorSet, VkDescriptorSet>
          (m_writeList, *m_pDstDescriptor, [](VkWriteDescriptorSet& wInfo, VkDescriptorSet& dst) {
            wInfo.dstSet = dst;
          });
          containerToCPtr(writeContainerSize, &writeData, m_writeList);
        }

        if (!m_copyList.empty() && m_pDstDescriptor->empty()) {
          foreachZip<VkCopyDescriptorSet, VkDescriptorSet>
          (m_copyList, *m_pDstDescriptor, [](VkCopyDescriptorSet& cInfo, VkDescriptorSet& dst) {
            cInfo.dstSet = dst;
          });

          foreachZip<VkCopyDescriptorSet, VkDescriptorSet>
          (m_copyList, *m_pSrcDescriptor, [](VkCopyDescriptorSet& cInfo, VkDescriptorSet& dst) {
            cInfo.srcSet = dst;
          });
          containerToCPtr(copyContainerSize, &copyData, m_copyList);
        }

        vkUpdateDescriptorSets(m_pDevice.get(), writeContainerSize, writeData, copyContainerSize, copyData);
      }

  };
}