#pragma once

#include "../vk.h"
#include "../type.h"
#include "../pointer.h"

#include <functional>

namespace cppvk {
  class AllocaionCallbacksBuilder :
    Noncopyable, Nondynamicallocation {

    public:

      using AllocationFunction = std::function<void*(void*, size_t, size_t, VkSystemAllocationScope)>;
      using FreeFunction = std::function<void(void*, void*)>;
      using ReallocationFunction = std::function<void*(void*, void*, size_t, size_t, VkSystemAllocationScope)>;
      using InternalAllocationNotificationFunction = std::function<void(void*, size_t, VkInternalAllocationType, VkSystemAllocationScope)>;
      using InternalFreeNotificationFunction = std::function<void(void*, size_t, VkInternalAllocationType, VkSystemAllocationScope)>;

      AllocaionCallbacksBuilder() = default;
      ~AllocaionCallbacksBuilder() = default;

      AllocationCallbacksPtr create() {
        auto allocator = std::make_shared<VkAllocationCallbacks>();
      }

  };
}