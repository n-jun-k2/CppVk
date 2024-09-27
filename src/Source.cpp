#pragma warning( disable : 4505 4189 26812)
#include "Window/App.h"
#include "Window/AppWindow.h"

#include "cppvk/vk.h"
#include "cppvk/allocationcallbacks.h"
#include "cppvk/pointer.h"
#include "cppvk/memory.h"
#include "cppvk/builders/instancebuilder.h"
#include "cppvk/builders/debugutilsmessengerbuilder.h"
#include "cppvk/builders/surfacebuilder.h"
#include "cppvk/builders/logicaldevicebuilder.h"
#include "cppvk/builders/commandpoolbuilder.h"
#include "cppvk/builders/commandbufferbuilder.h"
#include "cppvk/builders/swapchainbuilder.h"
#include "cppvk/builders/imagebuilder.h"
#include "cppvk/builders/imageviewbuilder.h"
#include "cppvk/builders/devicememorybuilder.h"
#include "cppvk/builders/bufferbuilder.h"
#include "cppvk/builders/descriptorsetlayoutbuilder.h"
#include "cppvk/builders/descriptorpoolbuilder.h"
#include "cppvk/builders/descriptorsetbuilder.h"
#include "cppvk/builders/pipelinelayoutbuilder.h"

#include "cppvk/models/descriptorsetupdater.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <algorithm>
#include <set>
#include <vector>



#if _DEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
  VkDebugUtilsMessageSeverityFlagBitsEXT ,
  VkDebugUtilsMessageTypeFlagsEXT ,
  const VkDebugUtilsMessengerCallbackDataEXT* pcallback,
  void* )
{
  std::cerr << "validation layer : " << pcallback->pMessage << std::endl;
  return VK_FALSE;
}
#endif


class MyContext {
  static constexpr size_t CMD_BUFFER_SIZE = 1;
  static constexpr size_t DESCRIPTOR_POOL_SIZE = 1;
#if _DEBUG
  cppvk::DebugUtilsMessengerPtr m_debugUtilsMessenger;
  cppvk::AllocationCallbacksPtr m_debugUtilsMessanger_callbacks;
#endif
  cppvk::InstancePtr m_instance;
  cppvk::SurfacePtr m_surface;
  cppvk::DevicePtr m_logicalDevice;
  cppvk::CommandPoolPtr m_cmdPool;
  cppvk::CommandBufferPtr m_cmdBuffer;
  cppvk::SwapchainPtr m_swapchain;
  cppvk::PipelineLayoutPtr m_pipelinelayout;
  cppvk::DescriptorPoolPtr m_descriptorpool;
  cppvk::DescriptorSetPtr m_descriptorset;

  std::vector<VkImage> m_swapchain_images; // no destroy
  std::vector<cppvk::ImageViewPtr> m_swapchain_iamgeViews;

  cppvk::DeviceMemoryPtr m_depthMemory;
  cppvk::ImagePtr m_depthImage;
  cppvk::ImageViewPtr m_depthImageView;

  cppvk::BufferPtr m_uniformBuffer;
  cppvk::DeviceMemoryPtr m_uniformMemory;
  cppvk::DescriptorSetLayoutPtr m_uniformDescriptorSetLayout;

  glm::mat4 projection;
  glm::mat4 view;
  glm::mat4 model;
  glm::mat4 clip;

public:
  MyContext() = default;
  ~MyContext() = default;

  void  WinInit(HWND hwnd, const uint32_t& , const uint32_t& ) {

    auto useDebug = true;
    auto devExtension = std::make_shared<cppvk::Names>(cppvk::Names{ VK_KHR_SWAPCHAIN_EXTENSION_NAME });

    m_instance = cppvk::InstanceBuilder()
      .applicationName("Hello Vulkan")
      .engineName("Vulkan Engine")
      .apiVersion(VK_API_VERSION_1_1)
      .engineVersion(VK_MAKE_VERSION(1, 0, 0))
      .enabledExtensionNames([&](std::vector<std::string>& extensionNames){

        std::vector<VkExtensionProperties> extesion;
        cppvk::getEnumerateInstanceExtension(extesion);

        const auto count = std::count_if(std::begin(extesion), std::end(extesion), [](auto e){ return strcmp(e.extensionName, "VK_KHR_surface_protected_capabilities") != 0;});
        extensionNames.reserve(count);

        for (const auto& e : extesion) {
          if (strcmp(e.extensionName, "VK_KHR_surface_protected_capabilities") != 0)
            extensionNames.push_back(e.extensionName);
        }

        if (useDebug)extensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
      })
      .enabledLayerNames([](std::vector<const char*>& layerNames){

        layerNames.reserve(1);
        layerNames.push_back("VK_LAYER_KHRONOS_validation");

        std::vector<VkLayerProperties> layer;
        cppvk::getEnumerateInstanceLayer(layer);
        if(!cppvk::existSupport(layerNames, layer)) {
          std::cout << "ERROR: VkLayerProperties not supported." << std::endl;
        }
      })
      .create();

#if _DEBUG
    m_debugUtilsMessenger = cppvk::DebugUtilsMessengerBuilder(m_instance)
      .severity(MessageServerity(VERBOSE) | MessageServerity(WARNING) | MessageServerity(ERROR))
      .type(MessageType(GENERAL) | MessageType(VALIDATION) | MessageType(PERFORMANCE))
      .callback(debugCallback)
      .create();
#endif

    m_surface = cppvk::SurfaceBuilder(m_instance)
      .hwnd(hwnd)
      .create();

    std::unique_ptr<cppvk::PhysicalDeviceDetails<std::vector>> pPhysicalDetails;
    std::unique_ptr<cppvk::PhysicalDeviceSurfaceDetails<std::vector>> pPhysicalSurfaceDetails;
    std::vector<VkPhysicalDevice> physicalDeviceList;
    cppvk::getEnumeratePhysicalDevices(m_instance.get(), physicalDeviceList);
    auto pPhysicalDevice = std::find_if(std::begin(physicalDeviceList), std::end(physicalDeviceList), [&](VkPhysicalDevice& physicalDevice) {
      pPhysicalDetails = std::make_unique< cppvk::PhysicalDeviceDetails<std::vector> >(physicalDevice);
      pPhysicalSurfaceDetails = std::make_unique< cppvk::PhysicalDeviceSurfaceDetails<std::vector> >(physicalDevice, m_surface.get());
      return true;
    });
    if (pPhysicalDevice == std::end(physicalDeviceList)) {
      throw std::runtime_error("Physical device not found");
    }

    auto graphics_queue_index = UINT32_MAX;
    auto present_queue_index = UINT32_MAX;
    {
      const auto queuePropertiesSize = static_cast<uint32_t>(pPhysicalDetails->queueProperties.size());

      std::vector<VkBool32> supportsPresent(queuePropertiesSize);
      for (uint32_t i = 0; i < queuePropertiesSize; ++i) supportsPresent[i] = cppvk::isSurfaeSupport(*pPhysicalDevice, m_surface.get(), i);

      for (uint32_t i = 0; i < queuePropertiesSize; ++i) {
        if((pPhysicalDetails->queueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)
          continue;

        if(graphics_queue_index == UINT32_MAX) graphics_queue_index = i;

        if (supportsPresent[i] == VK_TRUE) {
          graphics_queue_index = i;
          present_queue_index = i;
          break;
        }
      }

      if (present_queue_index == UINT32_MAX)
        present_queue_index = cppvk::find_if_index(std::begin(supportsPresent), std::end(supportsPresent), [](VkBool32 flag) {return flag == VK_TRUE;});

      if (graphics_queue_index == UINT32_MAX || present_queue_index == UINT32_MAX)
        throw std::runtime_error("Queue Family Index could not be found.");
    }

    m_logicalDevice = cppvk::LogicalDeviceBuilder(*pPhysicalDevice)
      .extensions([&pPhysicalDetails](std::vector<std::string>& names){
        const size_t count = std::count_if(std::begin(pPhysicalDetails->extensions), std::end(pPhysicalDetails->extensions), [](auto& e){
          return strcmp(e.extensionName, "VK_EXT_buffer_device_address") != 0;
        });
        names.reserve(count);
        for (auto&& ext : pPhysicalDetails->extensions) {
          if(strcmp(ext.extensionName, "VK_EXT_buffer_device_address") != 0)
            names.push_back(ext.extensionName);
        }
      })
      .layerNames([](std::vector<std::string>& names){
        names.reserve(1);
        names.push_back("VK_LAYER_KHRONOS_validation");
      })
      .features([&pPhysicalDetails](auto& feature) {
        feature = pPhysicalDetails->features;
      })
      .queueCreateInfoInit(1)
      .queueCreateInfoUpdate([=](cppvk::DeviceQueueCreateInfo&& info, std::vector<float>& priority){
        priority = {1.0f};
        info.familyIndex(graphics_queue_index);
      })
      .create();

    m_cmdPool = cppvk::CommandPoolBuilder(m_logicalDevice)
      .flags(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)
      .queueFamilyIndices(graphics_queue_index)
      .create();

    m_cmdBuffer = cppvk::CommandBufferBuilder(m_logicalDevice)
      .commandBufferCount(CMD_BUFFER_SIZE)
      .commandPool(m_cmdPool)
      .level(VK_COMMAND_BUFFER_LEVEL_PRIMARY)
      .create();

    auto suitableFormat = std::find_if(std::begin(pPhysicalSurfaceDetails->formatList), std::end(pPhysicalSurfaceDetails->formatList), [](VkSurfaceFormatKHR format) {
      return format.format == VkFormat::VK_FORMAT_B8G8R8A8_UNORM;
      });
    if (suitableFormat == std::end(pPhysicalSurfaceDetails->formatList)) {
      throw std::runtime_error("No suitable Surface format found." );
    }

    auto suitablePresent = std::find_if(std::begin(pPhysicalSurfaceDetails->presentModeList), std::end(pPhysicalSurfaceDetails->presentModeList), [](VkPresentModeKHR mode) {
      return mode == VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR;
      });
    if (suitablePresent == std::end(pPhysicalSurfaceDetails->presentModeList)) {
      throw std::runtime_error( "No suitable present mode found.");
    }

    VkSurfaceTransformFlagBitsKHR preTransform = pPhysicalSurfaceDetails->capabilities.currentTransform;
    if(preTransform & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

    const auto compositeAlphaFlags = std::vector<VkCompositeAlphaFlagBitsKHR> {
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    };
    auto compositeAlpha = std::find_if(std::begin(compositeAlphaFlags), std::end(compositeAlphaFlags), [&](VkCompositeAlphaFlagBitsKHR flags){
      return pPhysicalSurfaceDetails->capabilities.supportedCompositeAlpha & flags;
    });

    m_swapchain = cppvk::SwapchainBuilder(m_logicalDevice)
      .surface(m_surface)
      .minImageCount(pPhysicalSurfaceDetails->capabilities.minImageCount)
      .imageFormat(suitableFormat->format)
      .imageExtent(pPhysicalSurfaceDetails->capabilities.currentExtent)
      .preTransform(preTransform)
      .compositeAlpha(*compositeAlpha)
      .presentMode(*suitablePresent)
      .clippedOn()
      .imageColorSpace(suitableFormat->colorSpace)
      .imageUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
      .imageArrayLayers(1)
      .queueFamilyIndices([&](auto& indices){
        indices = { graphics_queue_index, present_queue_index};

        const auto queueFamilyIndicesSet = std::set<uint32_t>(std::begin(indices), std::end(indices));
        for (auto&& indice : queueFamilyIndicesSet) {
          if (!cppvk::isSurfaeSupport(*pPhysicalDevice, m_surface.get(), indice))
            throw std::runtime_error("Unsupported index information.");
        }

        if(queueFamilyIndicesSet.size() != indices.size())
          indices = cppvk::Indexs(std::begin(queueFamilyIndicesSet), std::end(queueFamilyIndicesSet));

      })
      .create();

    cppvk::getSwapchainImagesKHR(m_logicalDevice.get(), m_swapchain.get(), m_swapchain_images);

    m_swapchain_iamgeViews.reserve(m_swapchain_images.size());
    for(auto swapchainImage : m_swapchain_images) {
      m_swapchain_iamgeViews.push_back(
        cppvk::ImageViewBuilder(m_logicalDevice)
        .viewType(VK_IMAGE_VIEW_TYPE_2D)
        .image(swapchainImage)
        .format(suitableFormat->format)
        .components([](auto& info){
          info = {
            VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G,
            VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A,
          };
        })
        .aspectMask(VK_IMAGE_ASPECT_COLOR_BIT)
        .baseMipLevel(0)
        .levelCount(1)
        .baseArrayLayer(0)
        .layerCount(1)
        .create()
      );
    }

    VkExtent3D depthExtent = {};
    cppvk::extent2Dto3D(pPhysicalSurfaceDetails->capabilities.currentExtent, depthExtent);

    const VkFormat depthFormat = VK_FORMAT_D16_UNORM;
    auto depthImageTiling = cppvk::isLinearOrOptimal(*pPhysicalDevice, depthFormat, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    if (!depthImageTiling){
      throw std::runtime_error("Unsupported depth image format.");
    }

    m_depthImage = cppvk::ImageBuilder(m_logicalDevice)
      .tiling(depthImageTiling.value())
      .sharingMode(VK_SHARING_MODE_EXCLUSIVE)
      .imageType(VK_IMAGE_TYPE_2D)
      .format(depthFormat)
      .extent(depthExtent)
      .mipLevels(1)
      .usage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
      .samples(VK_SAMPLE_COUNT_1_BIT)
      .initialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
      .arrayLayers(1)
      .create();

    VkMemoryRequirements depthImageRequirements = {};
    vkGetImageMemoryRequirements(m_logicalDevice.get(), m_depthImage.get(), &depthImageRequirements);

    const auto depthImageTypeIndex = pPhysicalDetails->findMemoryType(depthImageRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    m_depthMemory = cppvk::DeviceMemoryBuilder(m_logicalDevice)
      .typeIndex(depthImageTypeIndex)
      .size(depthImageRequirements.size)
      .create();

    cppvk::bindImageMemory(m_logicalDevice, m_depthImage, m_depthMemory, 0);

    m_depthImageView = cppvk::ImageViewBuilder(m_logicalDevice)
      .viewType(VK_IMAGE_VIEW_TYPE_2D)
      .image(m_depthImage)
      .format(depthFormat)
      .components([](auto& info){
        info = {
          VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G,
          VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A,
        };
      })
      .subresourceRange([](auto& info){
        info = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1};
      })
      .create();

    m_uniformBuffer = cppvk::BufferBuilder(m_logicalDevice)
      .usage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
      .sharingMode(VK_SHARING_MODE_EXCLUSIVE)
      .size(sizeof(glm::mat4))
      .create();

    VkMemoryRequirements uniformBufferRequirements = {};
    vkGetBufferMemoryRequirements(m_logicalDevice.get(), m_uniformBuffer.get(), &uniformBufferRequirements);

    m_uniformMemory = cppvk::DeviceMemoryBuilder(m_logicalDevice)
      .typeIndex(pPhysicalDetails->findMemoryType(
        uniformBufferRequirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
      ))
      .size(uniformBufferRequirements.size)
      .create();

    {
      projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
      view = glm::lookAt(
          glm::vec3(-5, 3, -10),  // Camera is at (-5,3,-10), in World Space
          glm::vec3(0, 0, 0),     // and looks at the origin
          glm::vec3(0, -1, 0)     // Head is up (set to 0,-1,0 to look upside-down)
        );
      model = glm::mat4(1.0f);
      clip = glm::mat4(
                    1.0f, 0.0f, 0.0f, 0.0f,
                    0.0f,-1.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 0.5f, 0.0f,
                    0.0f, 0.0f, 0.5f, 1.0f
        );
      glm::mat4 mvp = clip * projection * view * model;
      auto mapData = cppvk::mapMemory<uint8_t>(m_logicalDevice, 0, uniformBufferRequirements.size, m_uniformMemory, 0);
      std::memcpy(mapData->data(), &mvp, mapData->size());
      cppvk::bindMemory(m_logicalDevice, m_uniformBuffer, m_uniformMemory);
    }

    auto descriptorSetLayoutPool = cppvk::DescriptorSetLayoutPool::createPool(1);

    m_uniformDescriptorSetLayout = cppvk::DescriptorSetLayoutBuilder(m_logicalDevice)
      .bindingCount(1)
      .bindings([](VkDescriptorSetLayoutBinding& layout) {
        layout.binding = 0;
        layout.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        layout.descriptorCount = 1;
        layout.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        layout.pImmutableSamplers = nullptr;
      })
      .pool(descriptorSetLayoutPool)
      .create();

    m_pipelinelayout = cppvk::PipelineLayoutBuilder(m_logicalDevice)
      .flags(0)
      .layoutpool(descriptorSetLayoutPool)
      .create();

    m_descriptorpool = cppvk::DescriptorPoolBuilder(m_logicalDevice)
      .maxSets(1)
      .poolSizeCount(1)
      .poolSizeUniformBuffer([](auto& size){ size = 1;})
      .create();

    m_descriptorset = cppvk::DescriptorSetBuilder(m_logicalDevice)
      .descriptorPool(m_descriptorpool)
      .layoutpool(descriptorSetLayoutPool)
      .create();

    cppvk::DescriptorSetUpdater descriptorUpdater(m_logicalDevice);
    descriptorUpdater.dstDescriptor(m_descriptorset);
    descriptorUpdater.writeSize(1);
    descriptorUpdater.writeBufferData([this](cppvk::WriteBufferInfo& info) {
      info
      .type(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
      .count(1)
      .binding(0)
      .arrayElement(0)
      .bufferInfo([this](VkDescriptorBufferInfo& bufInfo){
        bufInfo.buffer = this->m_uniformBuffer.get();
        bufInfo.offset = 0;
        bufInfo.range = sizeof(glm::mat4);
      });
    });
    descriptorUpdater.update();

  }
};

class VkApi :public  App {
  MyContext context;
public:
	virtual void Initialize() override {
		context.WinInit(winptr->getWin32(), 1280, 720);
	}

	virtual void Update() override {

	}

	virtual void Draw() const override {

	}

	virtual void Finalize() override {
	}

};

int main() {
	VkApi api;
	AppWindow ApplicationWindow(1280, 720, "Hello Vulkan Api", &api);
	ApplicationWindow.Run();
  return 1;
}