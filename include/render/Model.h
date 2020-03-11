#ifndef MODEL_H
#define MODEL_H

#include <vector>

#include <render/KoiVector.h>
#include <render/Context.h>
#include <render/Renderer.h>
#include <render/Scene.h>

class Model;

struct VertexData
{
    alignas(16) Vec3 position;
    alignas(16) Vec3 color;
    alignas(16) Vec3 normal;
    alignas(16) Vec2 tex;
};

struct Vertex
{
    VertexData data;

    static void getAttributeDescriptions(uint32_t binding, std::vector<VkVertexInputAttributeDescription> & attribDesc);
	bool operator == (const Vertex& other) const { return data.position == other.data.position && data.normal == other.data.normal && data.color == other.data.color && data.tex == other.data.tex; }

};

struct InstanceData
{
	Mat4 transform = Mat4(1.0f);
};

struct Instance
{
    InstanceData data;

	static void getAttributeDescriptions(uint32_t binding, std::vector<VkVertexInputAttributeDescription> & attribDesc);
};

class Shape
{
    public:
	uint32_t materialID;
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

    VkDescriptorSetLayout descriptorSetLayout;
    std::vector<VkDescriptorSet> descriptorSets;

	VkDeviceSize vertexBufferSize;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexMemory;

	VkDeviceSize indexBufferSize;
	VkBuffer indexBuffer;
	VkDeviceMemory indexMemory;
};

struct MaterialData
{
	alignas(16) Vec3 ambient;
	alignas(16) Vec3 diffuse;
	alignas(16) Vec3 specular;
	alignas(16) Vec3 emission;
	alignas(4) int32_t shininess;
	alignas(4) float opacity;
};

struct Material
{
	std::vector<UniformBuffer> buffers;

    static VkDescriptorSetLayoutBinding getVkDescriptorSetLayoutBinding(uint32_t binding);

    MaterialData data;
};

struct Texture
{
	Context * context;

	VkFormat format;
	VkImage image = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkImageView imageView = VK_NULL_HANDLE;

    static uint32_t count;
    static VkSampler sampler;

	Texture(std::string filename, Context * context, Renderer * renderer);
	~Texture();

    static VkDescriptorSetLayoutBinding getVkDescriptorSetLayoutBinding(uint32_t binding);
};

class ModelBase
{
    public:
	Context * context;
	Renderer * renderer;
    Scene * scene;

	std::string name;

    std::vector<Shape> shapes;
    std::vector<Material> materials;
    std::vector<Instance> instances;

    VkDeviceSize instanceBufferSize;
	VkBuffer instanceBuffer = VK_NULL_HANDLE;
	VkDeviceMemory instanceMemory = VK_NULL_HANDLE;

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

	ModelBase(Context * context, Renderer * renderer, Scene * scene);
	virtual ~ModelBase() = 0;

	virtual void draw(VkCommandBuffer commandbuffer) = 0;
};

class Model : public ModelBase
{
	public:
    static uint32_t count;
    static VkPipelineLayout pipelineLayout;
    static VkPipeline pipeline;

    virtual void createVkPipeline();

	Model(std::string filename, std::string location, Context * context, Renderer * renderer, Scene * scene);
	virtual ~Model();

    virtual void draw(VkCommandBuffer commandbuffer);
};

class TexturedModel : public ModelBase
{
    public:
    std::vector<Texture *> textures;

    static uint32_t count;
    static VkPipelineLayout pipelineLayout;
    static VkPipeline pipeline;

    virtual void createVkPipeline();

    TexturedModel(std::string filename, std::string location, Context * context, Renderer * renderer, Scene * scene);
    virtual ~TexturedModel();

    virtual void draw(VkCommandBuffer commandbuffer);
};

class RiggedModel : public ModelBase
{
    // TODO: Learn Skeletal Animation
};

namespace std
{
	template<> struct hash<Vertex>
	{
	    size_t operator()(Vertex const& vertex) const
		{
			size_t seed = 0;
			hash<Vec3> hasher;
			hashy(seed, hasher(vertex.data.position));
			hashy(seed, hasher(vertex.data.color));
			hashy(seed, hasher(vertex.data.normal));
            hashy(seed, hasher(Vec3(vertex.data.tex, 1.0f)));
			return seed;
	    }
	};
}

#endif