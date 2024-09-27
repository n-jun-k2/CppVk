#pragma once

#include "../vk.h"
#include "../type.h"
#include "../pointer.h"
#include "../deleter/deleter.h"


namespace cppvk {
  class ImageViewBuilder : Noncopyable, Nondynamicallocation {
    private:
      VkImageViewCreateInfo m_info;
      DeviceRef m_refDevice;
      ImageRef m_refImage;
    public:
      explicit ImageViewBuilder(DeviceRef pLogicalDevice)
      : m_refDevice(pLogicalDevice) {
        m_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        m_info.pNext = VK_NULL_HANDLE;
        m_info.flags = 0;
        m_info.subresourceRange = {};
      }
      ~ImageViewBuilder() = default;

      cppvk::ImageViewPtr create(AllocationCallbacksPtr callbacks = nullptr) {
        auto pImage = m_refImage.lock();
        auto pLogicalDevice = m_refDevice.lock();
        if (pLogicalDevice || pImage) {

          if(pImage)
            m_info.image = pImage.get();

          VkImageView imageView;
          checkVk(vkCreateImageView(pLogicalDevice.get(), &m_info, callbacks ? callbacks.get() : VK_NULL_HANDLE, &imageView));
          return ImageViewPtr(imageView, ImageViewDeleter(pLogicalDevice, callbacks));
        }
        throw std::runtime_error("Failed to create ImageView");
      }

      ImageViewBuilder& flag(const VkImageViewCreateFlags& flags) {
        m_info.flags = flags;
        return *this;
      }

      ImageViewBuilder& format(VkFormat format) {
        m_info.format = format;
        return *this;
      }

      ImageViewBuilder& components(std::function<void(VkComponentMapping&)> create) {
        create(m_info.components);
        return *this;
      }

      ImageViewBuilder& viewType(VkImageViewType type) {
        m_info.viewType = type;
        return *this;
      }

      ImageViewBuilder& aspectMask(const VkImageAspectFlags mask) {
        m_info.subresourceRange.aspectMask = mask;
        return *this;
      }

      ImageViewBuilder& baseMipLevel(const uint32_t level) {
        m_info.subresourceRange.baseMipLevel = level;
        return *this;
      }

      ImageViewBuilder& levelCount(const uint32_t count) {
        m_info.subresourceRange.levelCount = count;
        return *this;
      }

      ImageViewBuilder& baseArrayLayer(const uint32_t layer) {
        m_info.subresourceRange.baseArrayLayer = layer;
        return *this;
      }

      ImageViewBuilder& layerCount(const uint32_t count) {
        m_info.subresourceRange.layerCount = count;
        return *this;
      }

      ImageViewBuilder& image(ImageRef refImage) {
        m_refImage = refImage;
        return *this;
      }

      ImageViewBuilder& image(VkImage image) {
        m_info.image = image;
        return *this;
      }

      ImageViewBuilder& subresourceRange(std::function<void(VkImageSubresourceRange&)> create) {
        create(m_info.subresourceRange);
        return *this;
      }
  };
}