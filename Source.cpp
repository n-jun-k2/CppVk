#include "Window/App.h"
#include "Window/AppWindow.h"
#include "cppvk.h"

#include <set>

#define MessageServerity(str)		VK_DEBUG_UTILS_MESSAGE_SEVERITY_##str##_BIT_EXT
#define MessageType(str) VK_DEBUG_UTILS_MESSAGE_TYPE_##str##_BIT_EXT


#pragma warning(disable  : 26812 100)

//�Ƃ肠�����A�C�x���g�S�ďo�́B
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
	VkDebugUtilsMessageTypeFlagsEXT type, 
	const VkDebugUtilsMessengerCallbackDataEXT* pcallback, 
	void* userData)
{
	std::cerr << "validation layer : " << pcallback->pMessage << std::endl;
	return VK_FALSE;
}

const std::string RESOURCE_Dir = "resource";
const std::string VERTEX_SPV = "vert.spv";
const std::string FRAGMENT_SPV ="frag.spv";

/*
 find_if�̃C���f�b�N�X��Ԃ���
*/
template <class InputIterator, class Predicate>
uint32_t find_if_index(InputIterator first, InputIterator last, Predicate pred) {
	auto itr = std::find_if(first, last, pred);
	if (itr == last)return UINT32_MAX;
	return static_cast<uint32_t>(std::distance(first, itr));
}

class VkContext {
	cppvk::InstancePtr m_instance;
	cppvk::DebugMessengerPtr m_messenger;
	cppvk::SurfacePtr m_surface;
	cppvk::DevicePtr m_device;
	cppvk::SwapchainPtr m_swapchain;
	cppvk::RenderpassPtr m_renderpass;
	cppvk::CommandPoolPtr m_commandpool;
	cppvk::PipelineLayoutPtr m_pipelineLayout;
	cppvk::PipelinePtr m_graphicPipline;

	VkQueue m_graphics_queue;
	cppvk::DeviceMemoryPtr m_depth_image_memory;
	cppvk::ImagePtr m_depth_image;
	cppvk::ImageViewPtr m_depth_image_view;
	std::vector<cppvk::ImagePtr> m_swapchain_image_list;
	std::vector<cppvk::ImageViewPtr> m_swapchain_image_view_list;
	std::vector<cppvk::FramebufferPtr> m_frameuffer_list;
public:
	VkContext() {}
	~VkContext() {}

	void  WinInit(HWND wPtr,const uint32_t& width, const uint32_t& height) {

		auto useDebug = true;
		auto ext = cppvk::GetEnumerateInstanceExtension();
		auto lay = cppvk::GetEnumerateInstanceLayer();


		//�ݒ�ϐ���`
		cppvk::Names extensions{};
		cppvk::Names validation_layers{ "VK_LAYER_LUNARG_standard_validation" };
		cppvk::Names dev_extension = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };//�K�{�̊g���@�\�������ŏ��ɓ���Ă����B

		//�@�\���݂̂𒊏o
		for (const auto& e : ext) {
			if (strcmp(e.extensionName, "VK_KHR_surface_protected_capabilities") != 0)
				extensions.push_back(e.extensionName);//�ǉ��̃��C��
		}
		//�f�o�b�O�p�@�\���`
		if (useDebug)extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		//�e���؃��C���[�m�F
		if (!cppvk::ExistSupport(validation_layers, lay))
			std::cerr << "Error Validation Layers " << std::endl;


		//�C���X�^���X�̍쐬
		m_instance = cppvk::InstanceBuilder::get()
			.applicationName("Hello Vulkan")
			.engineName("Vulkan Engine")
			.apiVersion(VK_API_VERSION_1_1)
			.engineVersion(VK_MAKE_VERSION(1, 0, 0))
			.enabledExtensionNames(extensions)
			.enabledLayerNames(validation_layers)
			.build();

#if _DEBUG
		//�f�o�b�O�p�̃C���X�^���X���쐬�B
		m_messenger = cppvk::DebugUtilsMessengerBuilder::get(m_instance)
			.severity(MessageServerity(VERBOSE) | MessageServerity(WARNING) | MessageServerity(ERROR))
			.type(MessageType(GENERAL) | MessageType(VALIDATION) | MessageType(PERFORMANCE))
			.callback(debugCallback)
			.build();
#endif
		//�T�[�t�F�C�X�����쐬.
		m_surface = cppvk::WinSurfaceBuilder::get(m_instance)
			.hwnd(wPtr)
			.build();

		//�����f�o�C�X�̑I��
		auto gpu = m_instance->ChooseGpu([&dev_extension](cppvk::PhyscialDeivceSet& dev_set) {
			std::set<std::string> requiredExtensions(dev_extension.begin(), dev_extension.end());
			for (const auto& ext : dev_set.extensions) {
				requiredExtensions.erase(ext.extensionName);
			}
			return dev_set.props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && requiredExtensions.empty();
		});

		/*�f�o�C�X�L���[�C���f�b�N�X�̎擾	*/
		auto graphics_queue_index = find_if_index(gpu.qprops.begin(), gpu.qprops.end(), [](VkQueueFamilyProperties queue) {return queue.queueFlags & VK_QUEUE_GRAPHICS_BIT; });
		if (graphics_queue_index == UINT32_MAX)
			std::cerr << "not  find VK_QUEUE_GRAPHICS_BIT." << std::endl;

		/*�f�o�C�X�̊g���@�\���擾*/
		dev_extension.clear();
		for (auto&& dev_ext : gpu.extensions) {
			dev_extension.push_back(dev_ext.extensionName);
		}

		cppvk::Priorities default_queue_priority{ 1.0f };

		/*�_���f�o�C�X�̍쐬*/
		m_device = cppvk::DeviceBuilder::get(gpu.device)
			.addQueueInfo(default_queue_priority, graphics_queue_index)
			.extensions(dev_extension)
			.features({})
			.build();

		/*�R�}���h�v�[���̍쐬*/
		m_commandpool = cppvk::CommandPoolBuilder::get(m_device)
			.flags(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)//�R�}���h�o�b�t�@�𖈉�ݒ肷��
			.queueFamilyIndices(graphics_queue_index)
			.build();

		/*�f�o�C�X�L���[�̎擾*/
		m_graphics_queue =m_device->getQueue(graphics_queue_index, 0);


		/*�T�[�t�F�C�X�̊e�\�͒l�̎擾���s���B*/
		auto capabilites = m_surface->GetSurfaceCapabilities(gpu.device);
		auto formats = m_surface->GetEnumerateSurfaceFormats(gpu.device);
		auto presents = m_surface->GetEnumerateSurfacePresentmodes(gpu.device);

		/*�œK�ȃt�H�[�}�b�g��I��*/
		auto surface_format = std::find_if(formats.begin(), formats.end(), [](VkSurfaceFormatKHR format){
			return format.format == VkFormat::VK_FORMAT_B8G8R8A8_UNORM;
		});
		if (surface_format == formats.end())
			std::cerr << "No suitable format found." << std::endl;

		/*�œK�ȃv���[���g���[�h��I��*/
		auto surface_present = std::find_if(presents.begin(), presents.end(), [](VkPresentModeKHR mode) {
			return mode == VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR;
		});
		if (surface_present == presents.end())
			std::cerr << "No suitable present mode found." << std::endl;

		/*�X���b�v�`�F�[�������̏����擾*/
		auto image_count = capabilites.minImageCount;
		auto surface_extent = capabilites.currentExtent;
		if (surface_extent.width == 0u) {
			//�����Ȓl��⊮
			surface_extent.width = width;
			surface_extent.height = height;
		}
		/*�L���[�t�@�~���[�̃C���f�b�N�X��p��*/
		cppvk::Indexs queue_family_indices = { graphics_queue_index };
		//�T�[�t�F�C�X�̃T�|�[�g�@�\���`�F�b�N
		for (auto&& indice : queue_family_indices) {
			if(!m_surface->GetPhysicalDevicceSurfaceSupportKHR(gpu.device, indice))
				std::cerr << "Unsupported index information." << std::endl;
		}
		/*�X���b�v�`�F�[���̍쐬*/
		m_swapchain = cppvk::SwapchainBuilder::get(m_device)
			.surface(m_surface)
			.minImageCount(image_count)
			.queueFamily(queue_family_indices)
			.imageSharingMode(VkSharingMode::VK_SHARING_MODE_EXCLUSIVE)
			.imageFormat(surface_format->format)
			.imageColorSpace(surface_format->colorSpace)
			.imageExtent(surface_extent)
			.imageUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
			.imageArrayLayers(1)
			.presentMode(*surface_present)
			.clippedOn()
			.compositeAlpha(VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
			.preTransform(VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
			.build();

		VkExtent3D depth_extent = {};
		depth_extent.depth = 1;
		depth_extent.width = surface_extent.width;
		depth_extent.height = surface_extent.height;

		/*�f�v�X�o�b�t�@��image�쐬*/
		m_depth_image = cppvk::ImageBuilder::get(m_device)//�f�v�X�o�b�t�@�p�̃C���[�W�쐬
			.imageType(VK_IMAGE_TYPE_2D)
			.format(VK_FORMAT_D32_SFLOAT)
			.extent(depth_extent)
			.mipLevels(1)
			.usage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
			.samples(VK_SAMPLE_COUNT_1_BIT)
			.arrayLayers(1)
			.build();

		auto depth_image_requirements = m_depth_image->GetMemoryRequirements();
		auto request_bits = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		auto memory_type_index = find_if_index(std::begin(gpu.memory.memoryTypes), std::end(gpu.memory.memoryTypes),	[&request_bits](VkMemoryType type) {	
			return static_cast<VkMemoryPropertyFlagBits> (type.propertyFlags & request_bits) == request_bits;
		});
		
		m_swapchain_image_list = m_swapchain->GetImages();

		//�쐬�����C���[�W���o�C���h����B
		m_depth_image_memory = std::make_shared<cppvk::DeviceMemory>(m_device, memory_type_index, depth_image_requirements.size);
		m_depth_image_memory->bind<0>(m_depth_image);

		/* swapchain �� image view�̍쐬
			swapchain����擾����image�͂��łɃo�C���h����Ă���	*/
		m_swapchain_image_view_list.resize(m_swapchain_image_list.size());
		for (auto i = 0; i < m_swapchain_image_list.size(); ++i) {
			m_swapchain_image_view_list[i] = cppvk::ImageViewBuilder::get(m_device)
				.viewType(VkImageViewType::VK_IMAGE_VIEW_TYPE_2D)
				.image(**m_swapchain_image_list[i])
				.format(surface_format->format)
				.components({
				VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B,
				VK_COMPONENT_SWIZZLE_A
				})
				.subresourceRange({ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 })
				.build();
		}
		/* for depth */
		m_depth_image_view = cppvk::ImageViewBuilder::get(m_device)
			.viewType(VkImageViewType::VK_IMAGE_VIEW_TYPE_2D)
			.image(**m_depth_image)
			.format(VK_FORMAT_D32_SFLOAT)
			.components({
				VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B,
				VK_COMPONENT_SWIZZLE_A
			})
			.subresourceRange({ VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 })
			.build();

		/*!@ �T�u�p�X�F�O�̃p�X�̃t���[���o�b�t�@�̓��e�Ɉˑ�����㑱�̃����_�����O����(�����_�[�p�X�̏����̂���)*/
		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		VkAttachmentReference depthAttachmentRef = {};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		auto color_attachment = cppvk::AttachmentDescriptionCreate(surface_format->format, VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT);
		auto depth_attachment = cppvk::AttachmentDescriptionCreate(VK_FORMAT_D32_SFLOAT, VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT);
		depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		/* Render Pass�̍쐬 �t���[���o�b�t�@�𐶐�����O�ɕK�v�B*/
		m_renderpass = cppvk::RenderpassBuilder::get(m_device)
			.addAttachments(color_attachment)
			.addAttachments(depth_attachment)
			.addSubpassDependency(cppvk::SubpassDependencyCreate())
			.addSubpassDescription(cppvk::SubpassDescriptionCreate(colorAttachmentRef, depthAttachmentRef))
			.build();

		/*�t���[���o�b�t�@�̍쐬(�`���Ǘ�)*/
		for (auto&& image_view : m_swapchain_image_view_list) {

			auto framebuffer = cppvk::FrameBufferBuilder::get(m_device)
				.renderPass(m_renderpass)
				.width(surface_extent.width)
				.height(surface_extent.height)
				.layers(1)
				.addAttachment(image_view)
				.addAttachment(m_depth_image_view)
				.build();

			m_frameuffer_list.push_back(framebuffer);
		}

	}
};

class VkApi :public  App {
	VkContext context; 
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
}