#pragma once

#include "vk.h"
#include "type.h"
#include <array>
#include <memory>

namespace cppvk {

  template<class T>
  using pointer = std::shared_ptr<std::remove_pointer_t<T>>;

  template<class T>
  using reference = std::weak_ptr<std::remove_pointer_t<T>>;

  using InstancePtr = pointer<VkInstance>;
  using InstanceRef = reference<VkInstance>;

  using DebugUtilsMessengerPtr = pointer<VkDebugUtilsMessengerEXT>;
  using DebugUtilsMessengerRef = reference<VkDebugUtilsMessengerEXT>;

  using DevicePtr = pointer<VkDevice>;
  using DeviceRef = reference<VkDevice>;

  using DeviceMemoryPtr = pointer<VkDeviceMemory>;
  using DeviceMemoryRef = reference<VkDeviceMemory>;

  using SurfacePtr = pointer<VkSurfaceKHR>;
  using SurfaceRef = reference<VkSurfaceKHR>;

  using CommandPoolPtr = pointer<VkCommandPool>;
  using CommandPoolRef = reference<VkCommandPool>;

  using CommandBufferPtr = pointer<std::vector<VkCommandBuffer>>;
  using CommandBufferRef = reference<std::vector<VkCommandBuffer>>;

  using SwapchainPtr = pointer<VkSwapchainKHR>;
  using SwapchainRef = reference<VkSwapchainKHR>;

  using ImagePtr = pointer<VkImage>;
  using ImageRef = reference<VkImage>;

  using ImageViewPtr = pointer<VkImageView>;
  using ImageViewRef = reference<VkImageView>;

  using ShaderModulePtr = pointer<VkShaderModule>;
  using ShaderModuleRef = reference<VkShaderModule>;

  using AllocationCallbacksPtr = pointer<VkAllocationCallbacks>;
  using AllocationCallbacksRef = reference<VkAllocationCallbacks>;

  using BufferPtr = pointer<VkBuffer>;
  using BufferRef = reference<VkBuffer>;

  using DescriptorSetLayoutPtr = pointer<VkDescriptorSetLayout>;
  using DescriptorSetLayoutRef = reference<VkDescriptorSetLayout>;

  using PipelineLayoutPtr = pointer<VkPipelineLayout>;
  using PipelineLayoutRef = reference<VkPipelineLayout>;

  using DescriptorPoolPtr = pointer<VkDescriptorPool>;
  using DescriptorPoolRef = reference<VkDescriptorPool>;

  using SemaphorePtr = pointer<VkSemaphore>;
  using SemaphoreRef = reference<VkSemaphore>;

  using RenderPassPtr = pointer<VkRenderPass>;
  using RenderPassRef = reference<VkRenderPass>;

  using DescriptorSetPtr = pointer<std::vector<VkDescriptorSet>>;
  using DescriptorSetRef = reference<std::vector<VkDescriptorSet>>;

  using DescriptorSetLayoutPool = Pool<VkDescriptorSetLayout, std::vector>;
  using DescriptorSetLayoutPoolPtr = pointer<DescriptorSetLayoutPool>;
  using DescriptorSetLayoutPoolRef = reference<DescriptorSetLayoutPool>;

  using BufferViewPool = Pool<VkBufferView, std::vector>;
  using BufferViewPoolPtr = pointer<BufferViewPool>;
  using BufferViewPoolRef = reference<BufferViewPool>;

}