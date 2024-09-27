#pragma once

#include "../vk.h"
#include "../type.h"
#include "../pointer.h"
#include "../common.h"
#include "../deleter/deleter.h"

namespace cppvk {
  class ImageBuilder :Noncopyable, Nondynamicallocation{
    private:
      VkImageCreateInfo m_info;
      DeviceRef m_refLogicalDevice;
      std::vector<uint32_t> m_pQueueFamilyIndices;

    public:
      explicit ImageBuilder(DeviceRef pLogicalDevice)
      : m_refLogicalDevice(pLogicalDevice){
        m_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        m_info.pNext = VK_NULL_HANDLE;
        m_info.flags = 0;
        m_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        m_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        m_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        m_info.queueFamilyIndexCount = 0;
        m_info.pQueueFamilyIndices = nullptr;
      }

      cppvk::ImagePtr create(AllocationCallbacksPtr callbacks = nullptr) {
        containerToCPtr(m_info.queueFamilyIndexCount, &m_info.pQueueFamilyIndices, m_pQueueFamilyIndices);
        if(auto pLogicalDevice = m_refLogicalDevice.lock()){
          VkImage image;
          checkVk(vkCreateImage(pLogicalDevice.get(), &m_info, callbacks ? callbacks.get() : VK_NULL_HANDLE, &image));
          return ImagePtr(image, ImageDeleter(pLogicalDevice, callbacks));
        }
        throw std::runtime_error("Failed to create Image");
      }

      ImageBuilder& pNext() {
        m_info.pNext = VK_NULL_HANDLE;
        return *this;
      }

      ImageBuilder& flags(VkImageCreateFlags flag){
        m_info.flags = flag;
        return *this;
      }

      ImageBuilder& imageType(VkImageType type) {
        m_info.imageType = type;
        return *this;
      }

      ImageBuilder& format(VkFormat formats) {
        m_info.format = formats;
        return *this;
      }

      ImageBuilder& extent(VkExtent3D extents) {
        m_info.extent = extents;
        return *this;
      }

      ImageBuilder& mipLevels(const uint32_t level) {
        m_info.mipLevels = level;
        return *this;
      }

      ImageBuilder& arrayLayers(const uint32_t layers) {
        m_info.arrayLayers = layers;
        return *this;
      }

      ImageBuilder& samples(VkSampleCountFlagBits bits) {
        m_info.samples = bits;
        return *this;
      }

      ImageBuilder& tiling(VkImageTiling til) {
        m_info.tiling = til;
        return *this;
      }

      ImageBuilder& usage(VkImageUsageFlags usageFlags) {
        m_info.usage = usageFlags;
        return *this;
      }

      ImageBuilder& sharingMode(VkSharingMode mode) {
        m_info.sharingMode = mode;
        return *this;
      }

      ImageBuilder& queueFamilyIndices(std::function<void(std::vector<uint32_t>&)> create) {
      create(m_pQueueFamilyIndices);
      return *this;
    }

      ImageBuilder& initialLayout(VkImageLayout layout) {
        m_info.initialLayout = layout;
        return *this;
      }

  };
}