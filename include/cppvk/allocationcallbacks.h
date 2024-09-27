#pragma once


#include "vk.h"
#include "pointer.h"
#include <memory>

namespace cppvk {


  template<typename T>
  void*  allocationFunction(void* pUserData, size_t  size,  size_t  alignment, VkSystemAllocationScope allocationScope){
    std::cout << "allocationFunction(" << (T)pUserData << "), size(" << size << "), alignment(" << alignment << "), allocationScope(" << allocationScope <<")"<<  std::endl;
    // the allocation itself - ignore alignment, for while
    void* ptr = malloc(size);//_aligned_malloc(size, alignment);
    // memset(ptr, 0, size);
    return ptr;
  }

  template<typename T>
  void freeFunction(void*   pUserData, void*   pMemory){
      std::cout << "freeFunction:(" << (T)pUserData << "), pMemory(" << pMemory <<")"<< std::endl;
      free(pMemory);
  }

  template<typename T>
  void* reallocationFunction(void*   pUserData,   void*   pOriginal,  size_t  size, size_t  alignment,  VkSystemAllocationScope allocationScope){
    std::cout << "reallocationFunction:" << (T)pUserData << ", pOriginal" << pOriginal << ", size" << size << ", alignment" << alignment << ", allocationScope" << allocationScope << std::endl;
      return realloc(pOriginal, size);
  }

  template<typename T>
  void internalAllocationNotification(void*   pUserData,  size_t  size,   VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope){
    std::cout<< "internalAllocationNotification:" <<  (T)pUserData <<", size" << size  <<", allocationType" << allocationType <<  ", allocationScope"  << allocationScope << std::endl;
  }

  template<typename T>
  void internalFreeNotification(void*   pUserData, size_t  size,  VkInternalAllocationType  allocationType, VkSystemAllocationScope allocationScope){
      std::cout << "internalFreeNotification:" << (T)pUserData << ", size" << size << ", allocationType" << allocationType << ", allocationScope" << allocationScope << std::endl;
  }

  /**
* Create Pallocator
* @param info - String for tracking Allocator usage
*/
  template<typename T>
  static cppvk::AllocationCallbacksPtr createPAllocator(const char* info) {
    auto m_allocator = std::make_shared<VkAllocationCallbacks>();
    m_allocator->pUserData = (void*)info;
    m_allocator->pfnAllocation = (PFN_vkAllocationFunction)(&allocationFunction<T>);
    m_allocator->pfnReallocation = (PFN_vkReallocationFunction)(&reallocationFunction<T>);
    m_allocator->pfnFree = (PFN_vkFreeFunction)&freeFunction<T>;
    m_allocator->pfnInternalAllocation = (PFN_vkInternalAllocationNotification)&internalAllocationNotification<T>;
    m_allocator->pfnInternalFree = (PFN_vkInternalFreeNotification)&internalFreeNotification<T>;

    return m_allocator;
  }

}








































