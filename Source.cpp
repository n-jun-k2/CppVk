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

const std::string RESOURCE_Dir = "resource";
const std::string VERTEX_SPV = "vert.spv";
const std::string FRAGMENT_SPV ="frag.spv";

class VkContext {
	cppvk::InstancePtr instance;
	cppvk::DebugMessengerPtr dMessenger;
	cppvk::SurfacePtr surface;
	cppvk::DevicePtr device;
	cppvk::SwapchainPtr swapchain;
	cppvk::RenderpassPtr renderpass;
	cppvk::CommandPoolPtr commandpool;

	cppvk::ShaderModulePtr vertexModule;
	cppvk::ShaderModulePtr fragmentModule;

	cppvk::ImageList images;
	std::vector<cppvk::ImageViewPtr> imageViews;

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
			.addQueueInfo([&](cppvk::DeviceBuilder::DeviceQueueSetter& qbuilder) {
			auto findices = cppvk::QueueFamilyIndices::GraphicsFamily(qFimily);
			qbuilder
				.priorities(qPriorities)
				.familyIndices(findices);
				})
			.addQueueInfo([&](cppvk::DeviceBuilder::DeviceQueueSetter& qbuilder) {
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

		images = cppvk::helper::GetSwapchainImage(**device, **swapchain);
		imageViews.resize(images.size());
		for(uint32_t i = 0; i < images.size(); ++i)
			imageViews[i] = cppvk::ImageViewBuilder::get(device)
					.image(images[i])
					.viewType(VK_IMAGE_VIEW_TYPE_2D)
					.format(format.format)
					.components(VkComponentMapping{
						VK_COMPONENT_SWIZZLE_IDENTITY,
						VK_COMPONENT_SWIZZLE_IDENTITY,
						VK_COMPONENT_SWIZZLE_IDENTITY,
						VK_COMPONENT_SWIZZLE_IDENTITY
						})
					.subresourceRange(VkImageSubresourceRange{
						VK_IMAGE_ASPECT_COLOR_BIT,
						0,
						1,
						0,
						1
					})
					.build();
		
		auto verxCode = cppvk::helper::readFile(RESOURCE_Dir + "\\" + VERTEX_SPV);
		auto fragCode = cppvk::helper::readFile(RESOURCE_Dir + "\\" + FRAGMENT_SPV);

		vertexModule = cppvk::ShaderModuleBuilder::get(device)
			.code(verxCode)
			.build();

		fragmentModule = cppvk::ShaderModuleBuilder::get(device)
			.code(fragCode)
			.build();

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		renderpass = cppvk::RenderpassBuilder::get(device)
			.addAttachments([format](VkAttachmentDescription& info){
				/* �J���[�o�b�t�@�̃A�^�b�`�����g����ݒ�*/
				info.format = format.format;
				info.samples = VK_SAMPLE_COUNT_1_BIT;
				/* �F�E�[���f�[�^�ݒ�*/
				info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				/* �X�e���V���o�b�t�@�̃f�[�^�ݒ�*/
				info.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				info.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				/*
				Vulkan API�̃e�N�X�`���ƃt���[���o�b�t�@�́A
				VkImage�Ƃ�������̃s�N�Z���t�H�[�}�b�g�����I�u�W�F�N�g�ɂ���ĕ\���܂��B��
				���������́o�s�N�Z���̃��C�A�E�g�p�͉摜�ɑ΂��ĉ����g�p�Ƃ��Ă��邩�ɂ���ĕς�邱�Ƃ�����܂��B
				*/
				info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				info.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			})
			.addSubpassDependency([](VkSubpassDependency& info){
				/*�ˑ��֌W�E�ˑ��T�u�p�X�̃C���f�b�N�X���w��*/
				info.srcSubpass = VK_SUBPASS_EXTERNAL;
				info.dstSubpass = 0;
				/*�ҋ@����A���씭���C�x���g���w�肷��B*/
				info.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				info.srcAccessMask = 0;
				/*�J���[�A�^�b�`�����g�o�͐ݒ�*/
				info.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				info.dstAccessMask =
					VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
					VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			})
			.addSubpassDescription([colorAttachmentRef](VkSubpassDescription& info){
				info.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
				info.colorAttachmentCount = 1;
				info.pColorAttachments = &colorAttachmentRef;
				/*subpass.pInputAttachments �V�F�[�_����ǂݍ��܂�郊�\�[�X*/
				/*subpass.pResolveAttachments �}���`�T���v�����O�J���[���\�[�X�Ɏg�p����郊�\�[�X */
				/*subpass.pDepthStencilAttachment ���s�ƃX�e���V���f�[�^�̃��\�[�X */
				/*subpass.pPreserveAttachments �f�[�^��ۑ����郊�\�[�X*/
			})
			.build();

		commandpool = cppvk::CommandPoolBuilder::get(device)
			.queueFamilyIndices(cppvk::QueueFamilyIndices::GraphicsFamily(indices))
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