#pragma once

#include "../vk.h"
#include "../pointer.h"
#include <array>
#include <memory>

namespace cppvk {

  template<typename T>
  class _deleter {
    public:
      using type = std::remove_pointer_t<T>;
      using pointer = type*;
      _deleter() = delete;
      explicit _deleter(AllocationCallbacksPtr callbacks) : m_callbacks(callbacks) {}
      ~_deleter() = default;
      virtual void operator()(pointer ptr)  = 0;
    protected:
      AllocationCallbacksPtr m_callbacks;
  };

  class InstanceDeleter:public _deleter<VkInstance>{
    public:
      using _deleter::_deleter;
      virtual void operator()(pointer ptr)  override{
        vkDestroyInstance(ptr, m_callbacks ? m_callbacks.get() : VK_NULL_HANDLE);
      }
  };

  class LogicalDeviceDeleter:public _deleter<VkDevice>{
    public:
      using _deleter::_deleter;
      virtual void operator()(pointer ptr)  override{
        vkDestroyDevice(ptr, m_callbacks ? m_callbacks.get() : VK_NULL_HANDLE);
      }
  };

  template<typename T, typename T_BASE>
  class _sub_deleter : public _deleter<T> {
    public:
      using base_type = std::remove_pointer_t<T_BASE>;
      using base_type_pointer = std::shared_ptr<base_type>;
      _sub_deleter() = delete;
      explicit _sub_deleter(base_type_pointer pParent, AllocationCallbacksPtr callbacks)
        : _deleter<T>::_deleter(callbacks), m_pparent(pParent){}
      ~_sub_deleter() = default;
    protected:
      base_type_pointer m_pparent;
  };

  class DebugUtilsMessengerDeleter
    : public _sub_deleter <VkDebugUtilsMessengerEXT, VkInstance> {
    public:
      using _sub_deleter::_sub_deleter;
      virtual void operator()(pointer ptr)  override {
        auto instance = m_pparent.get();
        DestroyDebugUtilsMessengerEXT(instance, ptr, m_callbacks ? m_callbacks.get() : VK_NULL_HANDLE);
      }
  };

  class SurfacerDeleter
    : public _sub_deleter <VkSurfaceKHR, VkInstance> {
  public:
    using _sub_deleter::_sub_deleter;
    virtual void operator()(pointer ptr)  override {
      auto instance = m_pparent.get();
      vkDestroySurfaceKHR(instance, ptr, m_callbacks ? m_callbacks.get() : VK_NULL_HANDLE);
    }
  };

  class CommandPoolDeleter
    : public _sub_deleter <VkCommandPool, VkDevice> {
  public:
    using _sub_deleter::_sub_deleter;
    virtual void operator()(pointer ptr) override {
      auto device = m_pparent.get();
      vkDestroyCommandPool(device, ptr, m_callbacks ? m_callbacks.get() : VK_NULL_HANDLE);
    }
  };

  class SwapchainDeleter
    : public _sub_deleter <VkSwapchainKHR, VkDevice> {
  public:
    using _sub_deleter::_sub_deleter;
    virtual void operator()(pointer ptr) override {
      auto device = m_pparent.get();
      vkDestroySwapchainKHR(device, ptr, m_callbacks ? m_callbacks.get() : VK_NULL_HANDLE);
    }
  };

  class DeviceMemoryDeleter :
  public _sub_deleter<VkDeviceMemory, VkDevice> {
    public:
      using _sub_deleter::_sub_deleter;
      virtual void operator()(pointer ptr) override {
        auto device = m_pparent.get();
        vkFreeMemory(device, ptr, m_callbacks ? m_callbacks.get() : VK_NULL_HANDLE);
      }
  };

  class ImageDeleter : public _sub_deleter<VkImage, VkDevice> {
    public:
    using _sub_deleter::_sub_deleter;
    virtual void operator()(pointer ptr) override {
      auto device = m_pparent.get();
      vkDestroyImage(device, ptr, m_callbacks ? m_callbacks.get() : VK_NULL_HANDLE);
    }
  };

  class ImageViewDeleter : public _sub_deleter<VkImageView, VkDevice> {
    public:
    using _sub_deleter::_sub_deleter;
    virtual void operator()(pointer ptr) override {
      auto device = m_pparent.get();
      vkDestroyImageView(device, ptr, m_callbacks ? m_callbacks.get() : VK_NULL_HANDLE);
    }
  };

  class ShaderModuleDeleter : public _sub_deleter<VkShaderModule, VkDevice> {
    public:
      using _sub_deleter::_sub_deleter;
      virtual void operator()(pointer ptr) override {
        auto device = m_pparent.get();
        vkDestroyShaderModule(device, ptr, m_callbacks ? m_callbacks.get() : VK_NULL_HANDLE);
      }
  };

  class BufferDeleter : public _sub_deleter<VkBuffer, VkDevice> {
    public:
      using _sub_deleter::_sub_deleter;
      virtual void operator()(pointer ptr) override {
        auto device = m_pparent.get();
        vkDestroyBuffer(device, ptr, m_callbacks ? m_callbacks.get() : VK_NULL_HANDLE);
      }
  };

  class DescriptorSetLayoutDeleter : public _sub_deleter<VkDescriptorSetLayout, VkDevice> {
    public:
      using _sub_deleter::_sub_deleter;
      virtual void operator()(pointer ptr) override {
        auto device = m_pparent.get();
        vkDestroyDescriptorSetLayout(device, ptr, m_callbacks ? m_callbacks.get() : VK_NULL_HANDLE);
      }
  };

  class PipelineLayoutDeleter : public _sub_deleter<VkPipelineLayout, VkDevice> {
    public:
      using _sub_deleter::_sub_deleter;
      virtual void operator()(pointer ptr) override {
        auto device = m_pparent.get();
        vkDestroyPipelineLayout(device, ptr, m_callbacks ? m_callbacks.get() : VK_NULL_HANDLE);
      }
  };

  class DescriptorPoolDeleter : public _sub_deleter<VkDescriptorPool, VkDevice> {
    public:
      using _sub_deleter::_sub_deleter;
      virtual void operator()(pointer ptr) override {
        auto device = m_pparent.get();
        vkDestroyDescriptorPool(device, ptr, m_callbacks ? m_callbacks.get() : VK_NULL_HANDLE);
      }
  };

  class SemaphoreDeleter : public _sub_deleter<VkSemaphore, VkDevice> {
    public:
      using _sub_deleter::_sub_deleter;
      virtual void operator()(pointer ptr) override {
        auto device = m_pparent.get();
        vkDestroySemaphore(device, ptr, m_callbacks ? m_callbacks.get() : VK_NULL_HANDLE);
      }
  };

  template<typename PoolType>
  using device_and_pool = std::pair< pointer<VkDevice_T>, pointer<PoolType>>;

  template<typename PoolType, typename ArrayType>
  using _array_sub_deleter = _sub_deleter <std::vector<ArrayType>, device_and_pool<PoolType>>;


  using deivce_and_commandpool = device_and_pool<VkCommandPool>;
  using _cmd_sub_deleter = _array_sub_deleter<VkCommandPool,VkCommandBuffer>;

  class CommandBufferDeleter : public   _cmd_sub_deleter {
  public:
    using _cmd_sub_deleter::_cmd_sub_deleter;
    virtual void operator()(typename  _cmd_sub_deleter::pointer ptr) override {
      auto device = this->m_pparent->first.get();
      auto pool = this->m_pparent->second.get();
      vkFreeCommandBuffers(device, pool, static_cast<uint32_t>(ptr->size()), ptr->data());
    }
  };

  using device_and_descriptorpool = device_and_pool<VkDescriptorPool>;
  using _descriptorset_sub_deleter = _array_sub_deleter<VkDescriptorPool, VkDescriptorSet>;

  class DescriptorSetDeleter : public _descriptorset_sub_deleter {
  public:
    using _descriptorset_sub_deleter::_descriptorset_sub_deleter;
    virtual void operator()(typename  _descriptorset_sub_deleter::pointer ptr) override {
      auto device = this->m_pparent->first.get();
      auto pool = this->m_pparent->second.get();
      vkFreeDescriptorSets(device, pool, static_cast<uint32_t>(ptr->size()), ptr->data());
    }
  };

}