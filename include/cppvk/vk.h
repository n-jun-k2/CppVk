#pragma once

#pragma warning(push)
#pragma warning(disable : 26812)
#pragma warning(disable : 4505)

#if defined(ANDROID) || defined (__ANDROID__)
  #define VK_USE_PLATFORM_ANDROID_KHR
#elif defined(_WIN32)
  #define VK_USE_PLATFORM_WIN32_KHR
#else
  #define VK_USE_PLATFORM_XCB_KHR
#endif

#include "type.h"

#include <vulkan/vulkan.h>

#define STR(x) #x
#undef min
#undef max

#define MessageServerity(str)		VK_DEBUG_UTILS_MESSAGE_SEVERITY_##str##_BIT_EXT
#define MessageType(str) VK_DEBUG_UTILS_MESSAGE_TYPE_##str##_BIT_EXT

#include <tuple>
#include <vector>
#include <cassert>
#include <iostream>
#include <fstream>
#include <functional>
#include <type_traits>
#include <optional>

namespace cppvk {

  using Code = std::vector<char>;
  using Indexs = std::vector<uint32_t>;
  using Names = std::vector<const char*>;
  using PresentModes = std::vector<VkPresentModeKHR>;
  using SurfaceFormats = std::vector<VkSurfaceFormatKHR>;
  using LayerPropertiesList = std::vector<VkLayerProperties>;
  using ExtensionPropertiesList = std::vector<VkExtensionProperties>;



  void checkVk(const VkResult& result, const std::string& message = "") {
    if (result == VK_SUCCESS)return;
    std::cerr << "VkResult : " << result << std::endl;
    throw std::runtime_error(message);
  }

  template< template<typename E, typename Allocator=std::allocator<E>>typename Container>
  static void getEnumeratePhysicalDevices(VkInstance instance, Container<VkPhysicalDevice>& list) {
    uint32_t count;
    vkEnumeratePhysicalDevices(instance, &count, nullptr);
    list.resize(count);
    vkEnumeratePhysicalDevices(instance, &count, list.data());
  }

  template<template<typename E, typename Allocator=std::allocator<E>>typename Container>
  static void getEnumeratePhysicalDeviceGroupsKHR(VkInstance instance, Container<VkPhysicalDeviceGroupPropertiesKHR>& list) {
    uint32_t count;
    vkEnumeratePhysicalDeviceGroupsKHR(instance, &count, nullptr);
    list.resize(count);
    vkEnumeratePhysicalDeviceGroupsKHR(instance, &count, list.data());
  }

  template<template<typename E, typename Allocator=std::allocator<E>>typename Container>
  static void getEnumeratePhysicalDeviceGroups(VkInstance instance, Container<VkPhysicalDeviceGroupProperties>& list) {
    uint32_t count;
    vkEnumeratePhysicalDeviceGroups(instance, &count, nullptr);
    list.resize(count);
    vkEnumeratePhysicalDeviceGroups(instance, &count, list.data());
  }

  template<template<typename E, typename Allocator=std::allocator<E>>typename Container>
  static void getEnumerateInstanceExtension(Container<VkExtensionProperties>& list) {
    uint32_t size = 0;
    checkVk(vkEnumerateInstanceExtensionProperties(nullptr, &size, nullptr));
    list.resize(size);
    checkVk(vkEnumerateInstanceExtensionProperties(nullptr, &size, list.data()));
  }

  template<template<typename E, typename Allocator=std::allocator<E>>typename Container>
  static void getEnumerateInstanceLayer(Container<VkLayerProperties>& list) {
    uint32_t size = 0;
    checkVk(vkEnumerateInstanceLayerProperties(&size, nullptr));
    list.resize(size);
    checkVk(vkEnumerateInstanceLayerProperties(&size, list.data()));
  }

  static uint32_t getEnumerateInstanceVersion() {
    uint32_t version;
    checkVk(vkEnumerateInstanceVersion(&version));
    return version;
  }

  template<class _T, template<class... Args>class Container>
  static bool _existSupport(const Names& target, const Container<_T>& source, std::function<const char* (const _T&)> toString) noexcept {

    if (target.size() == 0) return true;

    bool isfound = false;
    for (const auto& t : target)
    {
      for (const auto& s : source)
      {
        isfound = strcmp(t, toString(s)) == 0;
        if (isfound)break;
      }
      if (!isfound)return false;
    }
    return true;
  }

  static bool existSupport(const Names& target, const ExtensionPropertiesList& source) {
    return _existSupport<VkExtensionProperties, std::vector>(target, source, [](VkExtensionProperties prop) {return prop.extensionName; });
  }
  static bool existSupport(const Names& target, const LayerPropertiesList& source) {
    return _existSupport<VkLayerProperties, std::vector>(target, source, [](VkLayerProperties prop) {return prop.layerName; });
  }

  static bool isSurfaeSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const uint32_t index) {
    VkBool32 isSupporte = VK_FALSE;
    checkVk(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, index, surface, &isSupporte));
    return isSupporte == VK_TRUE;
  }

  template<template<typename E, typename Allocator=std::allocator<E>>typename Container>
  struct PhysicalDeviceDetails {
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memoryProperties;
    Container<VkExtensionProperties> extensions;
    Container<VkLayerProperties> validations;
    Container<VkQueueFamilyProperties> queueProperties;


    PhysicalDeviceDetails() = delete;
    explicit PhysicalDeviceDetails(VkPhysicalDevice pDevice) {
      vkGetPhysicalDeviceProperties(pDevice, &properties);
      vkGetPhysicalDeviceFeatures(pDevice, &features);
      vkGetPhysicalDeviceMemoryProperties(pDevice, &memoryProperties);

      uint32_t  count = 0;
      vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &count, nullptr);
      queueProperties.resize(count);
      vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &count, queueProperties.data());

      checkVk(vkEnumerateDeviceExtensionProperties(pDevice, nullptr, &count, nullptr));
      extensions.resize(count);
      checkVk(vkEnumerateDeviceExtensionProperties(pDevice, nullptr, &count, extensions.data()));

      checkVk(vkEnumerateDeviceLayerProperties(pDevice, &count, nullptr));
      validations.resize(count);
      checkVk(vkEnumerateDeviceLayerProperties(pDevice, &count, validations.data()));
    }

    uint32_t findMemoryType(uint32_t filter, VkMemoryPropertyFlags property) {
      for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        auto type = filter & (1 << i);
        auto flag = memoryProperties.memoryTypes[i].propertyFlags & property;
        if( type && flag == property) return i;
      }
      throw std::runtime_error("Failed to find memory type");
    }
  };

  template<template<typename E, typename Allocator=std::allocator<E>>typename Container>
  struct PhysicalDeviceSurfaceDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    Container<VkSurfaceFormatKHR> formatList;
    Container<VkPresentModeKHR> presentModeList;

    explicit PhysicalDeviceSurfaceDetails(VkPhysicalDevice pDevice, VkSurfaceKHR pSurface) {
      checkVk(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pDevice, pSurface, &capabilities));

      uint32_t size;
      checkVk(vkGetPhysicalDeviceSurfaceFormatsKHR(pDevice, pSurface, &size, nullptr));
      formatList.resize(size);
			checkVk(vkGetPhysicalDeviceSurfaceFormatsKHR(pDevice, pSurface, &size, formatList.data()));

      checkVk(vkGetPhysicalDeviceSurfacePresentModesKHR(pDevice, pSurface, &size, nullptr));
      presentModeList.resize(size);
      checkVk(vkGetPhysicalDeviceSurfacePresentModesKHR(pDevice, pSurface, &size, presentModeList.data()));
    }
  };

  template<template<typename T, typename Allocate = std::allocator<T>>class Container>
  void getSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, Container<VkImage> &list) {
    uint32_t count;
    checkVk(vkGetSwapchainImagesKHR(device, swapchain, &count, nullptr));
    list.resize(count);
    checkVk(vkGetSwapchainImagesKHR(device, swapchain, &count, list.data()));
  }

  /// <summary>
  ///
  /// </summary>
  /// <param name="instance"></param>
  /// <param name="pInfo"></param>
  /// <param name="pAllocator"></param>
  /// <param name="pDebugMessenger"></param>
  /// <returns></returns>
  static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pInfo,
    const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
  {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)return func(instance, pInfo, pAllocator, pDebugMessenger);
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }

  /// <summary>
  /// Destroy function
  /// </summary>
  /// <param name="instance"></param>
  /// <param name="debugMessenger"></param>
  /// <param name="pAllocator"></param>
  static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)func(instance, debugMessenger, pAllocator);
  }

  static void extent2Dto3D(const VkExtent2D& src, VkExtent3D& dist) {
    dist.depth = 1;
    dist.width = src.width;
    dist.height = src.height;
  }

  /// <summary>
  /// Get the tiling corresponding to the "feature" flag.
  /// </summary>
  /// <param name="gpu">Opaque handle to a physical device object</param>
  /// <param name="format">Available image formats</param>
  /// <param name="feature">Flags expecting support</param>
  /// <returns> VK_IMAGE_TILING_OPTIMAL or VK_IMAGE_TILING_LINEAR or std::nullopt</returns>
  static std::optional<VkImageTiling> isLinearOrOptimal(VkPhysicalDevice gpu, const VkFormat& format, const VkFormatFeatureFlags feature) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(gpu, format, &props);

    if((props.optimalTilingFeatures & feature) == feature)
      return VK_IMAGE_TILING_OPTIMAL;
    if((props.linearTilingFeatures & feature) == feature)
      return VK_IMAGE_TILING_LINEAR;
    return std::nullopt;
  }

}
#pragma warning(pop)