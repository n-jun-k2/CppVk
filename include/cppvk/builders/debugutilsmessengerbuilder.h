#pragma once

#include "../vk.h"
#include "../type.h"
#include "../pointer.h"
#include "../deleter/deleter.h"

namespace cppvk {
  /// <summary>
  ///
  /// </summary>
  class DebugUtilsMessengerBuilder :
    Noncopyable, Nondynamicallocation {

  private:

    InstanceRef m_refInstance;
    VkDebugUtilsMessengerCreateInfoEXT m_info;

  public:
    ~DebugUtilsMessengerBuilder() = default;

    /// <summary>
    ///
    /// </summary>
    /// <param name="refInstance"></param>
    explicit DebugUtilsMessengerBuilder(InstanceRef refInstance) : m_refInstance(refInstance) {
      m_info = {};
      m_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
      m_info.pNext = NULL;
      m_info.flags = 0;
      m_info.pUserData = nullptr;
    }


    /// <summary>
    ///
    /// </summary>
    /// <param name="value"></param>
    /// <returns></returns>
    DebugUtilsMessengerBuilder& severity(VkDebugUtilsMessageSeverityFlagsEXT value) {
      m_info.messageSeverity = value;
      return *this;
    }

    /// <summary>
    ///
    /// </summary>
    /// <param name="value"></param>
    /// <returns></returns>
    DebugUtilsMessengerBuilder& type(VkDebugUtilsMessageTypeFlagsEXT value) {
      m_info.messageType = value;
      return *this;
    }

    /// <summary>
    ///
    /// </summary>
    /// <param name="value"></param>
    /// <returns></returns>
    DebugUtilsMessengerBuilder& callback(PFN_vkDebugUtilsMessengerCallbackEXT value) {
      m_info.pfnUserCallback = value;
      return *this;
    }

    /// <summary>
    ///
    /// </summary>
    /// <param name="callbacks"></param>
    /// <returns></returns>
    DebugUtilsMessengerPtr create(AllocationCallbacksPtr callbacks = nullptr) {
      if (DebugUtilsMessengerDeleter::base_type_pointer pInstance = m_refInstance.lock()) {
        VkDebugUtilsMessengerEXT vkMessenger;
        checkVk(CreateDebugUtilsMessengerEXT(pInstance.get(), &m_info, callbacks ? callbacks.get() : VK_NULL_HANDLE, &vkMessenger));
        return DebugUtilsMessengerPtr(vkMessenger, DebugUtilsMessengerDeleter(pInstance, callbacks));
      }
      else {
        throw std::runtime_error("Failed to create DebugUtilsMessenger");
      }
    }

  };//DebugUtilsMessengerBuilder
}