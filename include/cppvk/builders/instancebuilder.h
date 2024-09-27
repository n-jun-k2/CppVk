#pragma once

#include "../vk.h"
#include "../type.h"
#include "../pointer.h"
#include "../common.h"
#include "../deleter/deleter.h"

#include <functional>


namespace cppvk {

  /// <summary>
  /// Vulkan instance builder pattern
  /// </summary>
  class InstanceBuilder :
    Noncopyable, Nondynamicallocation {

  private:
    VkInstanceCreateInfo info;
    VkApplicationInfo appInfo;

    std::string m_applicationName;
    std::string m_engineName;
    std::vector<std::string> m_enabledLayerNames;
    std::vector<std::string> m_enabledExtensionNames;

    std::vector<const char*> m_rawEnabledLayersNames;
    std::vector<const char*> m_rawEnabledExtensionsNames;

  public:

    /// <summary>
    /// Create smart pointer
    /// </summary>
    /// <param name="callbacks"></param>
    /// <returns></returns>
    InstancePtr create(AllocationCallbacksPtr callbacks = nullptr) {
      containerToCPtr(info.enabledLayerCount, &info.ppEnabledLayerNames, m_rawEnabledLayersNames);
      containerToCPtr(info.enabledExtensionCount, &info.ppEnabledExtensionNames, m_rawEnabledExtensionsNames);

      VkInstance vkinstance;
      checkVk(vkCreateInstance(&info, callbacks ? callbacks.get() : VK_NULL_HANDLE, &vkinstance));
      return InstancePtr(vkinstance, InstanceDeleter(callbacks));
    }

    explicit InstanceBuilder() {
      appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
      appInfo.pNext = VK_NULL_HANDLE;
      appInfo.pApplicationName = NULL;
      appInfo.pEngineName = NULL;

      info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
      info.pNext = VK_NULL_HANDLE;
      info.flags = 0;
      info.pApplicationInfo = &appInfo;
      info.enabledLayerCount = 0;
      info.ppEnabledLayerNames = NULL;
      info.enabledExtensionCount = 0;
      info.ppEnabledExtensionNames = NULL;

    }
    ~InstanceBuilder() = default;

    /// <summary>
    ///  pApplicationName is NULL or is a pointer to a null-terminated UTF-8 string containing the name of the application.
    /// </summary>
    /// <param name="name"></param>
    /// <returns>InstanceBuilder</returns>
    InstanceBuilder& applicationName(const std::string& name) {
      m_applicationName = name;
      appInfo.pApplicationName = m_applicationName.c_str();
      return *this;
    }
    /// <summary>
    /// pEngineName is NULL or is a pointer to a null-terminated UTF-8 string containing the name of the engine (if any) used to create the application.
    /// </summary>
    /// <param name="name"></param>
    /// <returns></returns>
    InstanceBuilder& engineName(const std::string& name) {
      m_engineName = name;
      appInfo.pEngineName = m_engineName.c_str();
      return *this;
    }
    /// <summary>
    /// applicationVersion is an unsigned integer variable containing the developer-supplied version number of the application.
    /// </summary>
    /// <param name="version"></param>
    /// <returns></returns>
    InstanceBuilder& applicationVersion(const uint32_t version) {
      appInfo.applicationVersion = version;
      return *this;
    }
    /// <summary>
    /// engineVersion is an unsigned integer variable containing the developer-supplied version number of the engine used to create the application.
    /// </summary>
    /// <param name="version"></param>
    /// <returns></returns>
    InstanceBuilder& engineVersion(const uint32_t version) {
      appInfo.engineVersion = version;
      return *this;
    }
    /// <summary>
    ///  apiVersion is the version of the Vulkan API against which the application expects to run, encoded as described in the API Version Numbers and Semantics section. If apiVersion is 0 the implementation must ignore it, otherwise if the implementation does not support the requested apiVersion, or an effective substitute for apiVersion, it must return VK_ERROR_INCOMPATIBLE_DRIVER. The patch version number specified in apiVersion is ignored when creating an instance object. Only the major and minor versions of the instance must match those requested in apiVersion.
    /// </summary>
    /// <param name="version"></param>
    /// <returns></returns>
    InstanceBuilder& apiVersion(const uint32_t version) {
      appInfo.apiVersion = version;
      return *this;
    }
    /// <summary>
    /// ppEnabledLayerNames is a pointer to an array of enabledLayerCount null-terminated UTF-8 strings containing the names of layers to enable for the created instance. See the Layers section for further details.
    /// </summary>
    /// <param name="create"></param>
    /// <returns></returns>
    InstanceBuilder& enabledLayerNames(std::function<void(std::vector<std::string>&)> create) {
      create(m_enabledLayerNames);
      stringListToCPtrList(m_enabledLayerNames, m_rawEnabledLayersNames);
      return *this;
    }

    /// <summary>
    /// ppEnabledLayerNames is a pointer to an array of enabledLayerCount null-terminated UTF-8 strings containing the names of layers to enable for the created instance. See the Layers section for further details.
    /// </summary>
    /// <param name="create"></param>
    /// <returns></returns>
    InstanceBuilder& enabledLayerNames(std::function<void(std::vector<const char*>&)> create) {
      create(m_rawEnabledLayersNames);
      return *this;
    }
    /// <summary>
    /// ppEnabledExtensionNames is a pointer to an array of enabledExtensionCount null-terminated UTF-8 strings containing the names of extensions to enable.
    /// </summary>
    /// <param name="create"></param>
    /// <returns></returns>
    InstanceBuilder& enabledExtensionNames(std::function<void(std::vector<std::string>&)> create) {
      create(m_enabledExtensionNames);
      stringListToCPtrList(m_enabledExtensionNames, m_rawEnabledExtensionsNames);
      return *this;
    }
    /// <summary>
    /// ppEnabledExtensionNames is a pointer to an array of enabledExtensionCount null-terminated UTF-8 strings containing the names of extensions to enable.
    /// </summary>
    /// <param name="create"></param>
    /// <returns></returns>
    InstanceBuilder& enabledExtensionNames(std::function<void(std::vector<const char*>&)> create) {
      create(m_rawEnabledExtensionsNames);
      return *this;
    }

  };//InstanceBuilder

}