#pragma once

#include "vk.h"
#include "type.h"
#include "deleter/deleter.h"

#include <vector>

namespace cppvk {

  /// <summary>
  ///
  /// </summary>
  /// <param name="device"></param>
  /// <param name="memory"></param>
  /// <param name="flag"></param>
  template<typename T>
  static std::shared_ptr<std::vector<T>> mapMemory(cppvk::DevicePtr pDevice, const size_t offset, const size_t size, cppvk::DeviceMemoryPtr pMemory, VkMemoryMapFlags flag) {
    const auto Length = size - offset;
    auto ptr = new std::vector<T>(Length);

    void* tempPtr = reinterpret_cast<void*>(ptr->data());
    cppvk::checkVk(vkMapMemory(pDevice.get(), pMemory.get(), offset, size, flag, &tempPtr));

    struct _unmap {
      cppvk::DevicePtr pLogicalDeivce;
      cppvk::DeviceMemoryPtr pMemory;
      explicit _unmap(cppvk::DevicePtr device, cppvk::DeviceMemoryPtr memory)
        : pLogicalDeivce(device), pMemory(memory){}
      void operator()(std::vector<T>* ptr) {
        vkUnmapMemory(pLogicalDeivce.get(), pMemory.get());
        delete ptr;
      }
    };

    return std::shared_ptr<std::vector<T>>(ptr, _unmap(pDevice, pMemory));
  }

  /// <summary>
  ///
  /// </summary>
  /// <param name="pDevice"></param>
  /// <param name="pBuffer"></param>
  /// <param name="pMemory"></param>
  static void bindMemory(cppvk::DevicePtr pDevice, cppvk::BufferPtr pBuffer, cppvk::DeviceMemoryPtr pMemory, VkDeviceSize offset = 0) {
    cppvk::checkVk(vkBindBufferMemory(pDevice.get(), pBuffer.get(), pMemory.get(), offset));
  }

  /// <summary>
  ///
  /// </summary>
  /// <param name="pDevice"></param>
  /// <param name="pImage"></param>
  /// <param name="pMemory"></param>
  /// <param name="memoryOffset"></param>
  static void bindImageMemory(cppvk::DevicePtr pDevice, cppvk::ImagePtr pImage, cppvk::DeviceMemoryPtr pMemory, const VkDeviceSize memoryOffset) {
    cppvk::checkVk(vkBindImageMemory(pDevice.get(), pImage.get(), pMemory.get(), memoryOffset));
  }

}