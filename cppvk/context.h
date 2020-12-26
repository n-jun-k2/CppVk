#pragma once

#include "vk.h"
#include <memory>
#include <functional>

namespace cppvk {

  /// <summary>
  /// Vulkan context object
  /// </summary>
  class [[maybe_unused]] Context {

  public:

    using DestoryFunc = std::function<void(Context&)>;
    using Ptr = std::shared_ptr<Context>;
    using Ref = std::weak_ptr<Context>;

  private:

    DestoryFunc destroy;

    explicit Context() : destroy([](Context&) {}),
      instance(VK_NULL_HANDLE),
      messanger(VK_NULL_HANDLE),
      device(VK_NULL_HANDLE),
      surface(VK_NULL_HANDLE) {
    }

  public:

    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT messanger = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    Context(const Context&)             = default;
    Context& operator=(const Context&)  = default;
    Context(Context&&)                  = default;
    Context& operator=(Context&&)       = default;

    ~Context() { destroy(*this); }

    /// <summary>
    /// Interface to add termination processing for Vulkan objects
    /// </summary>
    /// <param name="other">Delete process</param>
    void operator+=(DestoryFunc other) {
      const auto& pre = destroy;
      this->destroy = [=](Context& ctx){ other(ctx); pre(ctx); };
    }

    /// <summary>
    /// Get a context object
    /// </summary>
    /// <returns>Context::Ptr</returns>
    static Context::Ptr make() {
      struct _temp : cppvk::Context {
        _temp() : Context() {}
      };
      return std::move(std::make_shared<_temp>());
    }


  };

}