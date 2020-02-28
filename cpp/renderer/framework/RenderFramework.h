
#ifndef RENDER_FRAMEWORK_H
#define RENDER_FRAMEWORK_H

#ifndef ANDROID

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#else

#include <android_native_app_glue.h>

#include <VrApi.h>
#include <VrApi_Vulkan.h>

#define VK_USE_PLATFORM_ANDROID_KHR
#include <vulkan_wrapper.h>

#endif

#include <vector>
#include <unordered_set>
#include <string.h>

#include <tiny_obj_loader.h>
#include <stb_image.h>

#include <Log.h>

#define APPLICATION_NAME "Project Koi"
#define APPLICATION_VERSION 3
#define ENGINE_NAME "Koi Engine"
#define ENGINE_VERSION 2

#define SAMPLE_COUNT VK_SAMPLE_COUNT_4_BIT
#define MIP_LEVELS 1
#define MULTIVIEW 0

struct Vec4
{
	float x;
	float y;
	float z;
	float w;

	float& operator [] (const int i)
	{
		return *((float *) (this) + i);
	}

	bool operator == (const Vec4& other) const
	{
		return x == other.x && y == other.y && z == other.z && w == other.w;
	}
};

struct Vec3
{
	float x;
	float y;
	float z;

	bool operator == (const Vec3& other) const
	{
		return x == other.x && y == other.y && z == other.z;
	}
};

struct Vec2
{
	float x;
	float y;

	bool operator == (const Vec2& other) const
	{
		return x == other.x && y == other.y;
	}

	operator Vec3 () const
	{
		Vec3 result = {x, y, 0.0};
		return result;
	}
};

struct Mat4
{
	Vec4 a;
	Vec4 b;
	Vec4 c;
	Vec4 d;

	Vec4& operator [] (const int i)
	{
		return *((Vec4 *) (this) + i);
	}
};

struct Vertex
{
	Vec3 position;
	Vec3 color;
	Vec2 tex;
	Vec3 normal;
	uint32_t texID;

	static void getAttributeDescriptions(uint32_t binding, std::vector<VkVertexInputAttributeDescription> * attribDesc)
	{
		attribDesc->emplace_back();
		attribDesc->back().location = attribDesc->size() - 1;
		attribDesc->back().binding = binding;
		attribDesc->back().format = VK_FORMAT_R32G32B32_SFLOAT;
		attribDesc->back().offset = offsetof(Vertex, position);

		attribDesc->emplace_back();
		attribDesc->back().location = attribDesc->size() - 1;
		attribDesc->back().binding = binding;
		attribDesc->back().format = VK_FORMAT_R32G32B32_SFLOAT;
		attribDesc->back().offset = offsetof(Vertex, color);

		attribDesc->emplace_back();
		attribDesc->back().location = attribDesc->size() - 1;
		attribDesc->back().binding = binding;
		attribDesc->back().format = VK_FORMAT_R32G32_SFLOAT;
		attribDesc->back().offset = offsetof(Vertex, tex);

		attribDesc->emplace_back();
		attribDesc->back().location = attribDesc->size() - 1;
		attribDesc->back().binding = binding;
		attribDesc->back().format = VK_FORMAT_R32G32B32_SFLOAT;
		attribDesc->back().offset = offsetof(Vertex, normal);

		attribDesc->emplace_back();
		attribDesc->back().location = attribDesc->size() - 1;
		attribDesc->back().binding = binding;
		attribDesc->back().format = VK_FORMAT_R32_UINT;
		attribDesc->back().offset = offsetof(Vertex, texID);
	}

	bool operator == (const Vertex& other) const
	{
		return position == other.position && color == other.color && tex == other.tex && normal == other.normal;
	}
};

struct Material
{
	uint32_t textureIndex = 0;
	Vec3 ambient;
	Vec3 diffuse;
	Vec3 specular;
};

struct Instance
{
	float opacity = 1.0f;
	Material material;
	Mat4 transform = {{1.0f, 0.0f, 0.0f, 0.0f},
	                  {0.0f, 1.0f, 0.0f, 0.0f},
	                  {0.0f, 0.0f, 1.0f, 0.0f},
	                  {0.0f, 0.0f, 0.0f, 1.0f}};

	static void getAttributeDescriptions(uint32_t binding, std::vector<VkVertexInputAttributeDescription> * attribDesc)
	{
		attribDesc->emplace_back();
		attribDesc->back().location = attribDesc->size() - 1;
		attribDesc->back().binding = binding;
		attribDesc->back().format = VK_FORMAT_R32_SFLOAT;
		attribDesc->back().offset = offsetof(Instance, opacity);

		/*attribDesc->emplace_back();
		attribDesc->back().location = attribDesc->size() - 1;
		attribDesc->back().binding = binding;
		attribDesc->back().format = VK_FORMAT_R32_UINT;
		attribDesc->back().offset = offsetof(Instance, material.textureIndex);*/

		attribDesc->emplace_back();
		attribDesc->back().location = attribDesc->size() - 1;
		attribDesc->back().binding = binding;
		attribDesc->back().format = VK_FORMAT_R32G32B32_SFLOAT;
		attribDesc->back().offset = offsetof(Instance, material.ambient);

		attribDesc->emplace_back();
		attribDesc->back().location = attribDesc->size() - 1;
		attribDesc->back().binding = binding;
		attribDesc->back().format = VK_FORMAT_R32G32B32_SFLOAT;
		attribDesc->back().offset = offsetof(Instance, material.diffuse);

		attribDesc->emplace_back();
		attribDesc->back().location = attribDesc->size() - 1;
		attribDesc->back().binding = binding;
		attribDesc->back().format = VK_FORMAT_R32G32B32_SFLOAT;
		attribDesc->back().offset = offsetof(Instance, material.specular);

		attribDesc->emplace_back();
		attribDesc->back().location = attribDesc->size() - 1;
		attribDesc->back().binding = binding;
		attribDesc->back().format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attribDesc->back().offset = offsetof(Instance, transform.a);

		attribDesc->emplace_back();
		attribDesc->back().location = attribDesc->size() - 1;
		attribDesc->back().binding = binding;
		attribDesc->back().format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attribDesc->back().offset = offsetof(Instance, transform.b);

		attribDesc->emplace_back();
		attribDesc->back().location = attribDesc->size() - 1;
		attribDesc->back().binding = binding;
		attribDesc->back().format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attribDesc->back().offset = offsetof(Instance, transform.c);

		attribDesc->emplace_back();
		attribDesc->back().location = attribDesc->size() - 1;
		attribDesc->back().binding = binding;
		attribDesc->back().format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attribDesc->back().offset = offsetof(Instance, transform.d);
	}
};

struct Mesh
{
	std::string name;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	std::vector<Instance> instances;

	VkDeviceSize vertexBufferSize;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexMemory;

	VkDeviceSize indexBufferSize;
	VkBuffer indexBuffer;
	VkDeviceMemory indexMemory;

	VkDeviceSize instanceBufferSize;
	VkBuffer instanceBuffer;
	VkDeviceMemory instanceMemory;	
};

struct UBO
{
	Mat4 view;
	Mat4 proj;
};

namespace std
{
	inline void hashy(size_t& seed, size_t hash)
	{
		hash += 0x9e3779b9 + (seed << 6) + (seed >> 2);
		seed ^= hash;
	}

	template<> struct hash<Vec2>
	{
	    size_t operator()(Vec2 const& vec2) const
		{
			size_t seed = 0;
			hash<float> hasher;
			hashy(seed, hasher(vec2.x));
			hashy(seed, hasher(vec2.y));
			return seed;
	    }
	};

	template<> struct hash<Vec3>
	{
	    size_t operator()(Vec3 const& vec3) const
		{
			size_t seed = 0;
			hash<float> hasher;
			hashy(seed, hasher(vec3.x));
			hashy(seed, hasher(vec3.y));
			hashy(seed, hasher(vec3.z));
			return seed;
	    }
	};

	template<> struct hash<Vec4>
	{
	    size_t operator()(Vec4 const& vec4) const
		{
			size_t seed = 0;
			hash<float> hasher;
			hashy(seed, hasher(vec4.x));
			hashy(seed, hasher(vec4.y));
			hashy(seed, hasher(vec4.z));
			hashy(seed, hasher(vec4.w));
			return seed;
	    }
	};

	template<> struct hash<Vertex>
	{
	    size_t operator()(Vertex const& vertex) const
		{
			size_t seed = 0;
			hash<Vec3> hasher;
			hashy(seed, hasher(vertex.position));
			hashy(seed, hasher(vertex.color));
			hashy(seed, hasher((Vec3) vertex.tex));
			hashy(seed, hasher((Vec3) vertex.normal));
			return seed;
	    }
	};
}

namespace RenderFramework
{
const std::vector<VkQueueFlagBits> requiredQueues = {VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_TRANSFER_BIT};
#ifndef SUPPRESS_DEBUG
const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation",
	"VK_LAYER_LUNARG_standard_validation"
};
#else
const std::vector<const char*> validationLayers = {

};
#endif
const std::vector<const char *> instanceExtensions = {};
const std::vector<const char *> deviceExtensions = {"VK_KHR_swapchain"};

typedef struct
{
	VkQueueFlags flags;
	uint32_t index;
	VkQueue queue;
} Queue;

struct Camera
{
	Vec3 position = {0.0f, 0.0f, 0.0f};
	Vec3 direction = {0.0f, 0.0f, 1.0f};
	Vec3 up = {0.0f, 1.0f, 0.0f};
};

class Context
{
	public:
		Context();
		~Context();

		void chooseQueues();

		GLFWwindow * window;
		VkSurfaceKHR surface;

		VkInstance instance;
		VkPhysicalDevice physicalDevice;
		VkDevice device;

		uint32_t queueCount;
		Queue graphicsQueue;
		Queue transferQueue;
		Queue presentQueue;

		VkCommandPool graphicsCommandPool;
		VkCommandPool transferCommandPool;
};

class Renderer
{
	public:
		Renderer(Context * context);
		~Renderer();

		void createSwapchain();
		void destroySwapchain();

		uint32_t getNextImage();
		void render(uint32_t imageIndex, std::vector<VkCommandBuffer> commandBuffers);
		void present(uint32_t imageIndex);

		uint32_t frameIndex = 0;

		VkRenderPass renderPass;
		VkSwapchainKHR swapchain;

		uint32_t length;
		VkExtent2D extent;
		uint32_t miplevels;
		VkSampleCountFlags sampleCount;
		VkPresentModeKHR presentMode;

		std::vector<VkImage> images;
		std::vector<VkImageView> imageViews;
		std::vector<VkFramebuffer> framebuffers;
		std::vector<VkFence> fences;

		VkSemaphore imageAvailable;
		VkSemaphore renderFinished;
		VkSemaphore guiFinished;

		VkFormat colorFormat;
		VkColorSpaceKHR colorSpace;
		VkImage colorImage;
		VkDeviceMemory colorImageMemory;
		VkImageView colorImageView;

		VkFormat depthFormat;
		VkImage depthImage;
		VkDeviceMemory depthImageMemory;
		VkImageView depthImageView;

		private:
			Context * context;
};

class Scene3D
{
	public:
		Scene3D(Context * context, Renderer * renderer, std::string filename);
		~Scene3D();

		void getVertexInputDescriptions(std::vector<VkVertexInputBindingDescription> * bindingDesc,
                                        std::vector<VkVertexInputAttributeDescription> * attribDesc);
		void setupDescriptors();
		void createPipeline();
		void recordCommands();
		void updateUBO();
		VkCommandBuffer getFrame(uint32_t imageIndex);
		void draw();

		UBO ubo;
		Camera camera;

		VkSampler textureSampler;
		std::vector<VkImage> textureImages;
		std::vector<VkDeviceMemory> textureImageMemorys;
		std::vector<VkImageView> textureImageViews;

		std::vector<Mesh> meshes;

		std::vector<VkCommandBuffer> commandBuffers;

		std::vector<VkBuffer> uniformBuffers;
		std::vector<VkDeviceMemory> uniformBuffersMemory;

		VkDescriptorPool descriptorPool;
		VkDescriptorSetLayout descriptorSetLayout;
		std::vector<VkDescriptorSet> descriptorSets;

		VkShaderModule vertexShader;
		VkShaderModule fragmentShader;

		VkPipelineLayout pipelineLayout;
		VkPipeline graphicsPipeline;

		private:
			Context * context;
			Renderer * renderer;
};

class GUI
{
	public:
		GUI(Context * context, Renderer * renderer);
		~GUI();

		void update(long elapsedTime);
		VkCommandBuffer getFrame(uint32_t imageIndex);
		void draw();

		Context * context;
		Renderer * renderer;

		VkDescriptorPool descriptorPool;
		VkRenderPass renderPass;

		std::vector<VkFramebuffer> framebuffers;

		std::vector<VkCommandPool> commandPools;
		std::vector<VkCommandBuffer> commandBuffers;
};

// GLFWwindow
void createBorderlessWindow(GLFWwindow ** window);
void destroyWindow(GLFWwindow ** window);
void createVkSurfaceKHR(VkInstance instance, GLFWwindow * window, VkSurfaceKHR * surface);
void destroyVkSurfaceKHR(VkInstance instance, VkSurfaceKHR * surface);

// VkInstance
void createInstance(VkInstance * instance);
void createInstanceGLFW(std::vector<const char *> layers, std::vector<const char *> extensions, VkInstance * instance);
void createInstance(std::vector<const char *> layers, std::vector<const char *> extensions, VkInstance * instance);
void createInstance(const char ** layers, uint32_t layerCount, const char ** extensions, uint32_t extensionCount, VkInstance * instance);
void destroyInstance(VkInstance * instance);
std::vector<VkPhysicalDevice> enumeratePhysicalDevices(VkInstance instance);

// VkPhysicalDevice
void chooseVkPhysicalDevice(VkInstance instance, VkPhysicalDevice * physicalDevice);
void clearVkPhysicalDevice(VkPhysicalDevice * physicalDevice);
void getAvailableQueues(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkQueueFlags type, bool presentationSupport, std::vector<Queue> * queues);
std::vector<VkExtensionProperties> getDeviceExtensions(VkPhysicalDevice device);
std::vector<VkQueueFamilyProperties> getDeviceQueueProperties(VkPhysicalDevice device);

// VkDevice

void createVkDevice(VkPhysicalDevice physicalDevice, uint32_t queueCount, uint32_t * queueIndices, VkDevice * device);
void destroyVkDevice(VkDevice * device);
void getVkQueue(VkDevice device, uint32_t queueIndex, VkQueue * queue);
void createVkCommandPool(VkDevice device, const void * pNext, VkCommandPoolCreateFlags flags, uint32_t queueFamilyIndex, VkCommandPool * commandPool);

// VkSwapchainKHR 
void getSwapchainFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkFormat * colorFormat, VkColorSpaceKHR * colorSpace, VkFormat * depthFormat);
void getSwapchainPresentMode(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkPresentModeKHR * presentMode);
void getSwapchainExtent(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkExtent2D * extent);
void getSwapchainLength(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t * length);
void createVkSwapchainKHR(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, uint32_t minImageCount, VkFormat imageFormat, VkColorSpaceKHR imageColorSpace, VkExtent2D imageExtent, uint32_t * pQueueFamilyIndices, uint32_t queueCount, VkPresentModeKHR presentMode, VkSwapchainKHR * swapchain);
void createVkSwapchainKHR(VkDevice device, const void * pNext, VkSwapchainCreateFlagsKHR flags, VkSurfaceKHR surface, uint32_t minImageCount, VkFormat imageFormat, VkColorSpaceKHR imageColorSpace, VkExtent2D imageExtent, uint32_t imageArrayLayers, VkImageUsageFlags imageUsage, VkSharingMode imageSharingMode, uint32_t queueFamilyIndexCount, const uint32_t * pQueueFamilyIndices, VkSurfaceTransformFlagBitsKHR preTransform, VkCompositeAlphaFlagBitsKHR compositeAlpha, VkPresentModeKHR presentMode, VkBool32 clipped, VkSwapchainKHR oldSwapchain, VkSwapchainKHR * swapchain);
void destroyVkSwapchainKHR(VkDevice logicalDevice, VkSwapchainKHR * swapchain);
void allocateVkCommandBuffers(VkDevice device, const void * pNext, VkCommandPool commandPool, VkCommandBufferLevel level, uint32_t commandBufferCount, VkCommandBuffer * commandBuffers);
void createVkFence(VkDevice device, const void * pNext, VkFenceCreateFlags flags, VkFence * fence);

// VkRenderPass
void createVkRenderPassOverlay(VkDevice device, VkFormat colorFormat, VkRenderPass * renderPass);
void createVkRenderPass(VkDevice device, VkFormat colorFormat, VkFormat depthFormat, VkSampleCountFlagBits sampleCount, VkRenderPass * renderPass);
void destroyVkRenderPass(VkDevice device, VkRenderPass * renderPass);

// VkShaderModule
void createShaderModule(Context * context, std::string filename, VkShaderModule * shaderModule);

// Utility
void parseVrApiExtensionsList(char * extensionNames, const char * extensions[], uint32_t * extensionsCount);

void createVkImage(Context * context, VkImageType imageType, VkFormat format, VkExtent3D extent, uint32_t miplevels, uint32_t arrayLayers, VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage * image, VkDeviceMemory * imageMemory);
void createVkImage(Context * context, VkImageType imageType, VkFormat format, VkExtent2D extent, uint32_t miplevels, uint32_t arrayLayers, VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage * image, VkDeviceMemory * imageMemory);
void createVkImage(VkPhysicalDevice physicalDevice, VkDevice device, VkImageType imageType, VkFormat format, VkExtent2D extent, uint32_t miplevels, uint32_t arrayLayers, VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags usage, VkSharingMode sharingMode, uint32_t queueFamilyIndexCount, const uint32_t * pQueueFamilyIndices, VkMemoryPropertyFlags properties, VkImage * image, VkDeviceMemory * imageMemory);
void createVkImage(VkPhysicalDevice physicalDevice, VkDevice device, VkImageType imageType, VkFormat format, VkExtent3D extent, uint32_t miplevels, uint32_t arrayLayers, VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags usage, VkSharingMode sharingMode, uint32_t queueFamilyIndexCount, const uint32_t * pQueueFamilyIndices, VkMemoryPropertyFlags properties, VkImage * image, VkDeviceMemory * imageMemory);
void createVkImage(VkPhysicalDevice physicalDevice, VkDevice device, const void * pNext, VkImageCreateFlags flags, VkImageType imageType, VkFormat format, VkExtent3D extent, uint32_t miplevels, uint32_t arrayLayers, VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags usage, VkSharingMode sharingMode, uint32_t queueFamilyIndexCount, const uint32_t * pQueueFamilyIndices, VkImageLayout initialLayout, VkMemoryPropertyFlags properties, VkImage * image, VkDeviceMemory * imageMemory);

void transitionImageLayout(Context * context, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t layers);

VkCommandBuffer beginSingleTimeCommands(Context * context);
void endSingleTimeCommands(Context * context, VkCommandBuffer commandBuffer);

void createVkImageView(Context * context, VkImage image, VkFormat format, uint32_t miplevels, uint32_t arrayLayers, VkImageAspectFlagBits aspectFlags, VkImageView * imageView);
void createVkImageView(VkPhysicalDevice physicalDevice, VkDevice device, VkImage image, VkImageViewType viewType, VkFormat format, uint32_t miplevels, uint32_t arrayLayers, VkImageAspectFlagBits aspectFlags, VkImageView * imageView);
void createVkImageView(VkPhysicalDevice physicalDevice, VkDevice device, const void * pNext, VkImageViewCreateFlags flags, VkImage image, VkImageViewType viewType, VkFormat format, VkComponentMapping components, VkImageSubresourceRange subresourceRange, VkImageView * imageView);

void createVkFramebuffer(VkDevice device, const void * pNext, VkFramebufferCreateFlags flags, VkRenderPass renderPass, VkImageView colorImageView, VkImageView depthImageView, VkImageView swapchainImageView, uint32_t width, uint32_t height, uint32_t layers, VkFramebuffer * framebuffer);

void loadMeshTexture(Context * context, Renderer * renderer, const char * textureName, VkImage * image, VkDeviceMemory * imageMemory, VkImageView * imageView);
void createMeshTextureSampler(VkDevice device, VkSampler * textureSampler);

void loadSceneModels(std::string filename, std::vector<Mesh> * meshes, std::vector<std::string> * diffuseTextureNames);
void createMeshBuffers(Context * context, Mesh * mesh);

void createBuffer(Context * context, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer * buffer, VkDeviceMemory * bufferMemory);

VkDescriptorSetLayoutBinding getDescriptorSetLayoutBinding(uint32_t binding, VkDescriptorType type, uint32_t descriptorCount, VkShaderStageFlags stageFlags, const VkSampler * pImmutableSamplers);
void createVkDescriptorPool(VkDevice device, std::vector<VkDescriptorSetLayoutBinding>& bindings, uint32_t maxSets, VkDescriptorPool * descriptorPool);
void createVkDescriptorSetLayout(VkDevice device, std::vector<VkDescriptorSetLayoutBinding>& bindings, VkDescriptorSetLayout * layout);
void allocateVkDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, std::vector<VkDescriptorSetLayout>& layouts, std::vector<VkDescriptorSet> * descriptorSets);
}

#endif
