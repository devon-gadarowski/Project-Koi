#ifndef UTILITIES_H
#define UTILITIES_H

#include <render/KoiVulkan.h>

#include <system/Log.h>
#include <render/Context.h>
#include <render/Model.h>

void createVkImage(Context * context, VkImageType imageType, VkFormat format, VkExtent3D extent, uint32_t miplevels, uint32_t arrayLayers, VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage * image, VkDeviceMemory * imageMemory);
void createVkImage(Context * context, VkImageType imageType, VkFormat format, VkExtent2D extent, uint32_t miplevels, uint32_t arrayLayers, VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage * image, VkDeviceMemory * imageMemory);
void createVkImage(VkPhysicalDevice physicalDevice, VkDevice device, VkImageType imageType, VkFormat format, VkExtent2D extent, uint32_t miplevels, uint32_t arrayLayers, VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags usage, VkSharingMode sharingMode, uint32_t queueFamilyIndexCount, const uint32_t * pQueueFamilyIndices, VkMemoryPropertyFlags properties, VkImage * image, VkDeviceMemory * imageMemory);
void createVkImage(VkPhysicalDevice physicalDevice, VkDevice device, VkImageType imageType, VkFormat format, VkExtent3D extent, uint32_t miplevels, uint32_t arrayLayers, VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags usage, VkSharingMode sharingMode, uint32_t queueFamilyIndexCount, const uint32_t * pQueueFamilyIndices, VkMemoryPropertyFlags properties, VkImage * image, VkDeviceMemory * imageMemory);
void createVkImage(VkPhysicalDevice physicalDevice, VkDevice device, const void * pNext, VkImageCreateFlags flags, VkImageType imageType, VkFormat format, VkExtent3D extent, uint32_t miplevels, uint32_t arrayLayers, VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags usage, VkSharingMode sharingMode, uint32_t queueFamilyIndexCount, const uint32_t * pQueueFamilyIndices, VkImageLayout initialLayout, VkMemoryPropertyFlags properties, VkImage * image, VkDeviceMemory * imageMemory);
void transitionImageLayout(VkDevice device, VkQueue queue, VkCommandPool commandPool, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t layers);
VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);
void endSingleTimeCommands(VkDevice device, VkQueue queue, VkCommandPool commandPool, VkCommandBuffer commandBuffer);
void createVkImageView(Context * context, VkImage image, VkFormat format, uint32_t miplevels, uint32_t arrayLayers, VkImageAspectFlagBits aspectFlags, VkImageView * imageView);
void createVkImageView(VkPhysicalDevice physicalDevice, VkDevice device, VkImage image, VkImageViewType viewType, VkFormat format, uint32_t miplevels, uint32_t arrayLayers, VkImageAspectFlagBits aspectFlags, VkImageView * imageView);
void createVkImageView(VkPhysicalDevice physicalDevice, VkDevice device, const void * pNext, VkImageViewCreateFlags flags, VkImage image, VkImageViewType viewType, VkFormat format, VkComponentMapping components, VkImageSubresourceRange subresourceRange, VkImageView * imageView);
void createVkFramebuffer(VkDevice device, const void * pNext, VkFramebufferCreateFlags flags, VkRenderPass renderPass, VkImageView colorImageView, VkImageView depthImageView, VkImageView swapchainImageView, uint32_t width, uint32_t height, uint32_t layers, VkFramebuffer * framebuffer);
void createBuffer(Context * context, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer * buffer, VkDeviceMemory * bufferMemory);
VkShaderModule loadShader(Context * context, std::string filename);
VkRenderPass createVkRenderPass(VkDevice device, VkFormat colorFormat, VkFormat depthFormat, VkSampleCountFlagBits sampleCount);
VkFramebuffer createVkFramebuffer(VkDevice device, const void * pNext, VkFramebufferCreateFlags flags, VkRenderPass renderPass, VkImageView colorImageView, VkImageView depthImageView, VkImageView swapchainImageView, uint32_t width, uint32_t height, uint32_t layers);
void copyBuffer(Context * context, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
void copyBufferToImage(Context * context, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);
void loadOBJ(std::string filename, std::string location, Context * context, Renderer * renderer, ModelBase * m, std::vector<std::string> * textures);
void createMeshTextureSampler(VkDevice device, VkSampler * textureSampler);
bool loadMeshTexture(std::string name, Context * context, Renderer * renderer, Texture * texture);
void createVertexBuffer(Context * context, std::vector<Vertex>& vertices, VkBuffer * vertexBuffer, VkDeviceMemory * vertexMemory, VkDeviceSize * vertexBufferSize);
void createIndexBuffer(Context * context, std::vector<uint32_t>& indices, VkBuffer * indexBuffer, VkDeviceMemory * indexMemory, VkDeviceSize * indexBufferSize);
void createInstanceBuffer(Context * context, std::vector<Instance>& instances, VkBuffer * instanceBuffer, VkDeviceMemory * instanceMemory, VkDeviceSize * instanceBufferSize);
std::string findFile(std::string filename, std::string root);

#endif