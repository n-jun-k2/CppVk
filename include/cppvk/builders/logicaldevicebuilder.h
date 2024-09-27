#pragma once

#include "../vk.h"
#include "../type.h"
#include "../pointer.h"
#include "../common.h"
#include "../deleter/deleter.h"
#include "../infos/devicequeueinfo.h"

#include <functional>

namespace cppvk {

  /// <summary>
  /// Logical device builder pattern
  /// </summary>
  class LogicalDeviceBuilder :
  Noncopyable, Nondynamicallocation {

  private:

    VkDeviceCreateInfo m_info;
    VkPhysicalDevice m_physicalDevice;

    std::vector<std::string> m_EnabledLayerNames;
    std::vector<std::string> m_EnabledExtensionNames;

    std::vector<const char*> m_rawEnabledLayerNames;
    std::vector<const char*> m_rawEnabledExtensionNames;

    std::vector<VkDeviceQueueCreateInfo> m_pQueueCreateInfo;
    std::vector<std::vector<float>> m_pQueueCreateInfoPriorites;
    VkPhysicalDeviceFeatures m_Features;

  public:

    LogicalDeviceBuilder() = delete;

    /// <summary>
    /// LogicalDeviceBuilder constructor
    /// </summary>
    /// <param name="physicaldevice"></param>
    explicit LogicalDeviceBuilder(const VkPhysicalDevice physicaldevice) :
      m_physicalDevice(physicaldevice){
      m_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
      m_info.pNext = NULL;
      m_info.flags = 0;
      m_info.queueCreateInfoCount = 0;
      m_info.enabledLayerCount = 0;
      m_info.enabledExtensionCount = 0;
    }

    /// <summary>
    /// Create smart pointer
    /// </summary>
    /// <param name="callbacks"></param>
    /// <returns></returns>
    cppvk::DevicePtr create(AllocationCallbacksPtr callbacks = nullptr) {

      m_info.pEnabledFeatures = &m_Features;
      containerToCPtr(m_info.enabledExtensionCount, &m_info.ppEnabledExtensionNames, m_rawEnabledExtensionNames);
      containerToCPtr(m_info.enabledLayerCount, &m_info.ppEnabledLayerNames, m_rawEnabledLayerNames);

      int idx = 0;
      for (auto& info : m_pQueueCreateInfo) {
        containerToCPtr(info.queueCount, &info.pQueuePriorities, m_pQueueCreateInfoPriorites[idx]);
        ++idx;
      }

      containerToCPtr(m_info.queueCreateInfoCount, &m_info.pQueueCreateInfos, m_pQueueCreateInfo);


      VkDevice vkDevice;
      checkVk(vkCreateDevice(m_physicalDevice, &m_info, callbacks ? callbacks.get() : VK_NULL_HANDLE, &vkDevice));
      return DevicePtr(vkDevice, LogicalDeviceDeleter(callbacks));
    }

		/// <summary>
		/// Create a LayerName managed by string
		/// </summary>
		/// <param name="create"></param>
		/// <returns></returns>
		LogicalDeviceBuilder& layerNames(std::function<void(std::vector<std::string>&)> create) {
      create(m_EnabledLayerNames);
      stringListToCPtrList(m_EnabledLayerNames, m_rawEnabledLayerNames);
			return *this;
		}

		/// <summary>
		/// Create a LayerName managed by string
		/// </summary>
		/// <param name="create"></param>
		/// <returns></returns>
		LogicalDeviceBuilder& layerNames(std::function<void(std::vector<const char*>&)> create) {
      create(m_rawEnabledLayerNames);
			return *this;
		}

		/// <summary>
		/// Create Extension name managed by string
		/// </summary>
		/// <param name="create"></param>
		/// <returns></returns>
    LogicalDeviceBuilder& extensions(std::function<void(std::vector<std::string>&)> create) {
      create(m_EnabledExtensionNames);
      stringListToCPtrList(m_EnabledExtensionNames, m_rawEnabledExtensionNames);
      return *this;
    }

		/// <summary>
		/// Create Extension name managed by string
		/// </summary>
		/// <param name="create"></param>
		/// <returns></returns>
    LogicalDeviceBuilder& extensions(std::function<void(std::vector<const char*>&)> create) {
      create(m_rawEnabledExtensionNames);
      return *this;
    }

		/// <summary>
		///
		/// </summary>
		/// <param name="arg"></param>
		/// <returns></returns>
    LogicalDeviceBuilder& features(std::function<void(VkPhysicalDeviceFeatures&)> update) {
      update(m_Features);
      return *this;
    }

		/// <summary>
		///
		/// </summary>
		/// <param name="size"></param>
		/// <returns></returns>
    LogicalDeviceBuilder& queueCreateInfoInit(const int size) {
      m_pQueueCreateInfo.resize(size);
      m_pQueueCreateInfoPriorites.resize(size);
      return *this;
    }

		/// <summary>
		///
		/// </summary>
		/// <param name="arg"></param>
		/// <returns></returns>
    LogicalDeviceBuilder& queueCreateInfoUpdate(std::function<void(DeviceQueueCreateInfo&&, std::vector<float>&)> create, const uint32_t offset = 0, const int count = 0) {
      int i = offset;
      const size_t size = count <= 0 ? m_pQueueCreateInfo.size() : count;
      for (; i < size; ++i) create(DeviceQueueCreateInfo(m_pQueueCreateInfo[i]), m_pQueueCreateInfoPriorites[i]);
      return *this;
    }

  };

}