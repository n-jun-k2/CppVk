#pragma once

#include "../vk.h"
#include "../type.h"
#include "../pointer.h"

namespace cppvk {

  class SurfaceBuilder :
    Noncopyable, Nondynamicallocation {
  private:
    InstanceRef m_refInstance;
    VkWin32SurfaceCreateInfoKHR  m_info;

  public:

    SurfaceBuilder() = delete;
    ~SurfaceBuilder() = default;

    explicit SurfaceBuilder(cppvk::InstanceRef refInstance) : m_refInstance(refInstance) {
      m_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
      m_info.hinstance = GetModuleHandle(nullptr);
      m_info.pNext = NULL;
      m_info.flags = 0;
    }

    /// <summary>
    ///
    /// </summary>
    /// <param name="item"></param>
    /// <returns></returns>
    SurfaceBuilder& hwnd(HWND item) {
      m_info.hwnd = item;
      return *this;
    }

    /// <summary>
    ///
    /// </summary>
    /// <param name="item"></param>
    /// <returns></returns>
    SurfaceBuilder& hinstance(HINSTANCE item) {
      m_info.hinstance = item;
      return *this;
    }

    /// <summary>
    ///
    /// </summary>
    /// <param name="callbacks"></param>
    /// <returns></returns>
    cppvk::SurfacePtr create(AllocationCallbacksPtr callbacks = nullptr){
      if (auto pInstance = m_refInstance.lock())
      {
        VkSurfaceKHR vkSurface;
        checkVk(vkCreateWin32SurfaceKHR(pInstance.get(), &m_info, callbacks ? callbacks.get() : VK_NULL_HANDLE, &vkSurface));
        return SurfacePtr(vkSurface, SurfacerDeleter(pInstance, callbacks));
      } else {
        throw std::runtime_error("Failed to create Surface");
      }
    }
  };
}