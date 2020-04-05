
#include "Window/App.h"
#include "Window/AppWindow.h"
#include "cppvk.h"

#define MessageServerity(str)		VK_DEBUG_UTILS_MESSAGE_SEVERITY_##str##_BIT_EXT
#define MessageType(str) VK_DEBUG_UTILS_MESSAGE_TYPE_##str##_BIT_EXT


#pragma warning(push)
#pragma warning(disable:26812)
//�Ƃ肠�����A�C�x���g�S�ďo�́B
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
	VkDebugUtilsMessageTypeFlagsEXT type, 
	const VkDebugUtilsMessengerCallbackDataEXT* pcallback, 
	void* userData)
{
	std::cerr << "validation layer : " << pcallback->pMessage << std::endl;
	return VK_FALSE;
}
#pragma warning(pop)

class VkContext {
	cppvk::InstancePtr instance;
	cppvk::DebugMessengerPtr dMessenger;
	cppvk::SurfacePtr surface;
	cppvk::DevicePtr device;
	cppvk::SwapchainPtr swapchain;

public:
	VkContext() {}
	~VkContext() {}
	void  WinInit(HWND wPtr) {

		auto useDebug = true;
		auto ext = cppvk::helper::GetEnumerateInstanceExtension();
		auto lay = cppvk::helper::GetEnumerateInstanceLayer();

		//�ݒ�ϐ���`
		cppvk::Names extensions{};
		cppvk::Names validationLayers{ "VK_LAYER_LUNARG_standard_validation" };

		//�@�\���݂̂𒊏o
		for (const auto& e : ext) {
			if (strcmp(e.extensionName, "VK_KHR_surface_protected_capabilities") != 0)
				extensions.push_back(e.extensionName);//�ǉ��̃��C��
		}
		//�f�o�b�O�p�@�\���`
		if (useDebug)extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		//�e���؃��C���[�m�F
		if (!cppvk::helper::ExistSupport(validationLayers, lay))
			std::cerr << "Error Validation Layers " << std::endl;


		//�C���X�^���X�̍쐬
		instance = cppvk::InstanceBuilder::get()
			.enabledExtensionNames(extensions)
			.enabledLayerNames(validationLayers)
			.build();
#if _DEBUG
		//�f�o�b�O�p�̃C���X�^���X���쐬�B
		dMessenger = cppvk::DebugUtilsMessengerBuilder::get(instance)
			.severity(MessageServerity(VERBOSE) | MessageServerity(WARNING) | MessageServerity(ERROR))
			.type(MessageType(GENERAL) | MessageType(VALIDATION) | MessageType(PERFORMANCE))
			.callback(debugCallback)
			.build();
#endif
		//�T�[�t�F�C�X�����쐬.
		surface = cppvk::WinSurfaceBuilder::get(instance)
			.hwnd(wPtr)
			.build();

		//�����f�o�C�X
		auto gpu = cppvk::helper::chooseGpu(instance, [](VkPhysicalDeviceProperties props, VkPhysicalDeviceFeatures features) {
			return props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
			});

		auto qFimily = cppvk::helper::FindQueueFamilyIndices(gpu, surface->get());
		auto qPriorities = cppvk::Priorities{ 1.0f };
		//�_���f�o�C�X�̍쐬�B
		device = cppvk::DeviceBuilder::get(gpu)
			.extensions({ VK_KHR_SWAPCHAIN_EXTENSION_NAME })
			.features({})
			.addQueueInfo([&](cppvk::DeviceBuilder::DeviceQueueBuilder& qbuilder) {
			auto findices = cppvk::QueueFamilyIndices::GraphicsFamily(qFimily);
			qbuilder
				.priorities(qPriorities)
				.familyIndices(findices);
				})
			.addQueueInfo([&](cppvk::DeviceBuilder::DeviceQueueBuilder& qbuilder) {
					auto findices = cppvk::QueueFamilyIndices::PresentFamily(qFimily);
					qbuilder
						.priorities(qPriorities)
						.familyIndices(findices);
				})
			.layerNames(validationLayers)
			.build();
				
		auto swapSupport = cppvk::helper::GetSwapchainSupport(gpu, **surface);
		auto imagebuffercount = cppvk::SwapchainSupportDetails::getSurfaceCapabilities(swapSupport).minImageCount + 1;
		auto indices = cppvk::helper::FindQueueFamilyIndices(gpu, **surface);
		auto format = cppvk::helper::chooseSwapSurfaceFormat(swapSupport);
		auto presentMode = cppvk::helper::chooseSwapPresentMode(swapSupport);
		auto extent = cppvk::helper::chooseSwapExtent(swapSupport);
		auto queueIndices = cppvk::Indexs{ cppvk::QueueFamilyIndices::GraphicsFamily(indices),cppvk::QueueFamilyIndices::PresentFamily(indices) };
		
		swapchain = cppvk::SwapchainBuilder::get(device)
				.surface(surface)
				.minImageCount(imagebuffercount)
				.imageFormat(format.format)
				.imageExtent(extent)
				.imageColorSpace(format.colorSpace)
				.imageArrayLayers(1)
				.imageUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
				.preTransform(cppvk::SwapchainSupportDetails::getSurfaceCapabilities(swapSupport).currentTransform)
				.compositeAlpha(VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
				.presentMode(presentMode)
				.clippedOn()
				.oldSwapchainNullSet()
				//.imageSharingMode(VK_SHARING_MODE_EXCLUSIVE)
				.imageSharingMode(VK_SHARING_MODE_CONCURRENT)
				.queueFamily(queueIndices)
				.build();
		
	}
};

class VkApi :public  App {
	VkContext context; 
public:
	virtual void Initialize() override {
		context.WinInit(winptr->getWin32());
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
}