#include <fstream>
#include <filesystem>
#include <algorithm>

#include <render/Utilities.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

#include <dds/dds.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

void createVkImage(Context * context,
                   VkImageType imageType,
                   VkFormat format,
                   VkExtent2D extent,
                   uint32_t miplevels,
                   uint32_t arrayLayers,
                   VkSampleCountFlagBits samples,
                   VkImageTiling tiling,
                   VkImageUsageFlags usage,
                   VkMemoryPropertyFlags properties,
                   VkImage * image,
                   VkDeviceMemory * imageMemory)
{
	VkExtent3D e = {};
	e.width = extent.width;
	e.height = extent.height;
	e.depth = 1;

	createVkImage(context, imageType, format, e, miplevels, arrayLayers, samples,
	              tiling, usage, properties, image, imageMemory);
}

void createVkImage(Context * context,
                   VkImageType imageType,
                   VkFormat format,
                   VkExtent3D extent,
                   uint32_t miplevels,
                   uint32_t arrayLayers,
                   VkSampleCountFlagBits samples,
                   VkImageTiling tiling,
                   VkImageUsageFlags usage,
                   VkMemoryPropertyFlags properties,
                   VkImage * image,
                   VkDeviceMemory * imageMemory)
{
	const void * pNext = nullptr;
	uint32_t flags = (arrayLayers == 6) ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
	VkSharingMode sharingMode = (context->queueIndices.size() > 1) ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
	uint32_t queueFamilyIndexCount = context->queueIndices.size();
	const uint32_t * pQueueFamilyIndices = context->queueIndices.data();
	VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	createVkImage(context->physicalDevice, context->device, pNext, flags, imageType, format, extent,
	              miplevels, arrayLayers, samples, tiling, usage, sharingMode, queueFamilyIndexCount,
	              pQueueFamilyIndices, initialLayout, properties, image, imageMemory);
}

void createVkImage(VkPhysicalDevice physicalDevice,
                   VkDevice device,
                   VkImageType imageType,
                   VkFormat format,
                   VkExtent2D extent,
                   uint32_t miplevels,
                   uint32_t arrayLayers,
                   VkSampleCountFlagBits samples,
                   VkImageTiling tiling,
                   VkImageUsageFlags usage,
                   VkSharingMode sharingMode,
                   uint32_t queueFamilyIndexCount,
                   const uint32_t * pQueueFamilyIndices,
                   VkMemoryPropertyFlags properties,
                   VkImage * image,
                   VkDeviceMemory * imageMemory)
{
	VkExtent3D e = {};
	e.width = extent.width;
	e.height = extent.height;
	e.depth = 1;

	createVkImage(physicalDevice, device, imageType, format, e, miplevels,
	              arrayLayers, samples, tiling, usage, sharingMode, queueFamilyIndexCount,
	              pQueueFamilyIndices, properties, image, imageMemory);
}

void createVkImage(VkPhysicalDevice physicalDevice,
                   VkDevice device,
                   VkImageType imageType,
                   VkFormat format,
                   VkExtent3D extent,
                   uint32_t miplevels,
                   uint32_t arrayLayers,
                   VkSampleCountFlagBits samples,
                   VkImageTiling tiling,
                   VkImageUsageFlags usage,
                   VkSharingMode sharingMode,
                   uint32_t queueFamilyIndexCount,
                   const uint32_t * pQueueFamilyIndices,
                   VkMemoryPropertyFlags properties,
                   VkImage * image,
                   VkDeviceMemory * imageMemory)
{
	const void * pNext = nullptr;
	uint32_t flags = (arrayLayers == 6) ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
	VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	createVkImage(physicalDevice, device, pNext, flags, imageType, format, extent, miplevels,
	              arrayLayers, samples, tiling, usage, sharingMode, queueFamilyIndexCount,
	              pQueueFamilyIndices, initialLayout, properties, image, imageMemory);
}

void createVkImage(VkPhysicalDevice physicalDevice,
                   VkDevice device,
                   const void * pNext,
                   VkImageCreateFlags flags,
                   VkImageType imageType,
                   VkFormat format,
                   VkExtent3D extent,
                   uint32_t miplevels,
                   uint32_t arrayLayers,
                   VkSampleCountFlagBits samples,
                   VkImageTiling tiling,
                   VkImageUsageFlags usage,
                   VkSharingMode sharingMode,
                   uint32_t queueFamilyIndexCount,
                   const uint32_t * pQueueFamilyIndices,
                   VkImageLayout initialLayout,
                   VkMemoryPropertyFlags properties,
                   VkImage * image,
                   VkDeviceMemory * imageMemory)
{
	VkImageCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createInfo.pNext = pNext;
	createInfo.flags = flags;
	createInfo.imageType = imageType;
	createInfo.format = format;
	createInfo.extent = extent;
	createInfo.mipLevels = miplevels;
	createInfo.arrayLayers = arrayLayers;
	createInfo.samples = samples;
	createInfo.tiling = tiling;
	createInfo.usage = usage;
	createInfo.sharingMode = sharingMode;
	createInfo.queueFamilyIndexCount = queueFamilyIndexCount;
	createInfo.pQueueFamilyIndices = pQueueFamilyIndices;
	createInfo.initialLayout = initialLayout;

	int result = vkCreateImage(device, &createInfo, nullptr, image);
	if (result != VK_SUCCESS)
	{
		PANIC("RENDER_FRAMEWORK - Failed to create VkImage %d", result);
		return;
	}

	VkMemoryRequirements memReqs;
	vkGetImageMemoryRequirements(device, *image, &memReqs);

	VkPhysicalDeviceMemoryProperties memProps;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);

	uint32_t typeIndex = -1;
	for (uint32_t i = 0; i < memProps.memoryTypeCount; i++)
	{
		if (memReqs.memoryTypeBits & (1 << i) && (memProps.memoryTypes[i].propertyFlags & properties) == properties)
		{
			typeIndex = i;
		}
	}

	if (typeIndex == -1)
	{
		PANIC("RENDER_FRAMEWORK - Failed to allocate VkImage memory");
		vkDestroyImage(device, *image, nullptr);
		return;
	}

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.allocationSize = memReqs.size;
	allocInfo.memoryTypeIndex = typeIndex;

	result = vkAllocateMemory(device, &allocInfo, nullptr, imageMemory);
	if (result != VK_SUCCESS)
	{
		PANIC("RENDER_FRAMEWORK - Failed to allocate image memory! %d", result);
	}

	vkBindImageMemory(device, *image, *imageMemory, 0);
}

void createVkImageView(Context * context,
                       VkImage image,
                       VkFormat format,
                       uint32_t miplevels,
                       uint32_t arrayLayers,
                       VkImageAspectFlagBits aspectFlags,
                       VkImageView * imageView)
{
	createVkImageView(context->physicalDevice, context->device, image,
	                  (arrayLayers == 6) ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D,
	                  format, miplevels, arrayLayers, aspectFlags, imageView);
}

void createVkImageView(VkPhysicalDevice physicalDevice,
                       VkDevice device,
                       VkImage image,
                       VkImageViewType viewType,
                       VkFormat format,
                       uint32_t miplevels,
                       uint32_t arrayLayers,
                       VkImageAspectFlagBits aspectFlags,
                       VkImageView * imageView)
{
	const void * pNext = nullptr;
	VkImageViewCreateFlags flags = 0;
	VkComponentMapping components = {};
	components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = aspectFlags;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = miplevels;
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.layerCount = arrayLayers;

	createVkImageView(physicalDevice, device, pNext, flags, image, viewType, format, components,
	                  subresourceRange, imageView);
}

void createVkImageView(VkPhysicalDevice physicalDevice,
                       VkDevice device,
                       const void * pNext,
                       VkImageViewCreateFlags flags,
                       VkImage image,
                       VkImageViewType viewType,
                       VkFormat format,
                       VkComponentMapping components,
                       VkImageSubresourceRange subresourceRange,
                       VkImageView * imageView)
{
	VkImageViewCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.pNext = pNext;
	createInfo.flags = flags;
	createInfo.image = image;
	createInfo.viewType = viewType;
	createInfo.format = format;
	createInfo.components = components;
	createInfo.subresourceRange = subresourceRange;

	int result = vkCreateImageView(device, &createInfo, nullptr, imageView);
	if (result != VK_SUCCESS)
	{
		PANIC("RENDER_FRAMEWORK - Failed to create VkImageView! %d", result);
	}
}

void createVkFramebuffer(VkDevice device, const void * pNext, VkFramebufferCreateFlags flags,
                         VkRenderPass renderPass, VkImageView colorImageView,
                         VkImageView depthImageView, VkImageView swapchainImageView, uint32_t width,
                         uint32_t height, uint32_t layers, VkFramebuffer * framebuffer)
{
	std::vector<VkImageView> attachments;

	if (colorImageView != VK_NULL_HANDLE)
		attachments.push_back(colorImageView);

	if (depthImageView != VK_NULL_HANDLE)
		attachments.push_back(depthImageView);

	if (swapchainImageView != VK_NULL_HANDLE)
		attachments.push_back(swapchainImageView);

	VkFramebufferCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.pNext = pNext;
	createInfo.flags = flags;
	createInfo.renderPass = renderPass;
	createInfo.attachmentCount = attachments.size();
	createInfo.pAttachments = attachments.data();
	createInfo.width = width;
	createInfo.height = height;
	createInfo.layers = layers;

	int result = vkCreateFramebuffer(device, &createInfo, nullptr, framebuffer);
	if (result != VK_SUCCESS)
	{
		PANIC("RENDER_FRAMEWORK _ Failed to create framebuffer %d", result);
	}
}

VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool)
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void endSingleTimeCommands(VkDevice device, VkQueue queue, VkCommandPool commandPool, VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);

	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void transitionImageLayout(VkDevice device, VkQueue queue, VkCommandPool commandPool, VkImage image, VkFormat format,
                           VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t layers)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT)
		{
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = layers;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		                        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		                        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		                        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		                        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		                        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
		                        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		                        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	else
	{
		PANIC("RENDER_FRAMEWORK - Unsupported layout transition");
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	endSingleTimeCommands(device, queue, commandPool, commandBuffer); 
}

void createBuffer(Context * context, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                  VkBuffer * buffer, VkDeviceMemory * bufferMemory)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;

	bufferInfo.queueFamilyIndexCount = context->queueIndices.size();
	bufferInfo.pQueueFamilyIndices = context->queueIndices.data();
	bufferInfo.sharingMode = (context->queueIndices.size() > 1) ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;

	int result = vkCreateBuffer(context->device, &bufferInfo, nullptr, buffer);
	VALIDATE(result == VK_SUCCESS, "Failed to create buffer! %d", result);

	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(context->device, *buffer, &memReqs);

	VkPhysicalDeviceMemoryProperties memProps;
	vkGetPhysicalDeviceMemoryProperties(context->physicalDevice, &memProps);

	uint32_t typeIndex = -1;
	for (uint32_t i = 0; i < memProps.memoryTypeCount; i++)
	{
		if (memReqs.memoryTypeBits & (1 << i) && (memProps.memoryTypes[i].propertyFlags & properties) == properties)
		{
			typeIndex = i;
		}
	}

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memReqs.size;
	allocInfo.memoryTypeIndex = typeIndex;

	result = vkAllocateMemory(context->device, &allocInfo, nullptr, bufferMemory);
	VALIDATE(result == VK_SUCCESS, "Failed to allocate memory for buffer! %d", result)

	vkBindBufferMemory(context->device, *buffer, *bufferMemory, 0);
}

void copyBuffer(Context * context, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(context->device, context->primaryTransferQueue->commandPool);

	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	endSingleTimeCommands(context->device, context->primaryTransferQueue->queue, context->primaryTransferQueue->commandPool, commandBuffer);
}

void copyBufferToImage(Context * context, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(context->device, context->primaryTransferQueue->commandPool);

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = layerCount;

	region.imageOffset = {0, 0, 0};
	region.imageExtent = {width, height, 1};

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	endSingleTimeCommands(context->device, context->primaryTransferQueue->queue, context->primaryTransferQueue->commandPool, commandBuffer);
}

void loadOBJ(std::string filename, std::string location, Context * context, Renderer * renderer, ModelBase * m, std::vector<std::string> * textures)
{
	std::string err, warn;

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, (location + filename).c_str(), location.c_str(), true, true);

	for (auto& shape : shapes)
	{
		Shape mesh = {};
		size_t index_offset = 0;
		std::unordered_map<Vertex, uint32_t> uniqueVertices;
		for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++)
		{
			int fv = shape.mesh.num_face_vertices[f];
			for (size_t v = 0; v < fv; v++)
			{
				tinyobj::index_t index = shape.mesh.indices[index_offset + v];

				Vertex vertex = {};

				vertex.data.position = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				vertex.data.color = {
					attrib.colors[3 * index.normal_index + 0],
					attrib.colors[3 * index.normal_index + 1],
					attrib.colors[3 * index.normal_index + 2]
				};

				vertex.data.normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};

				vertex.data.tex = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};

				if (uniqueVertices.count(vertex) == 0)
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(mesh.vertices.size());
					mesh.vertices.push_back(vertex);
				}

				mesh.indices.push_back(uniqueVertices[vertex]);
			}
			mesh.materialID = shape.mesh.material_ids[f];
			index_offset += fv;
		}
		m->shapes.push_back(mesh);
	}

	for (auto& material : materials)
	{
		Material mat;

		if (textures != nullptr && material.diffuse_texname != "")
			textures->push_back(material.diffuse_texname);

		mat.data.ambient = {material.ambient[0], material.ambient[1], material.ambient[2]};
		mat.data.diffuse = {material.diffuse[0], material.diffuse[1], material.diffuse[2]};
		mat.data.specular = {material.specular[0], material.specular[1], material.specular[2]};
		mat.data.emission = {material.emission[0], material.emission[1], material.emission[2]};
		mat.data.shininess = material.shininess;
		mat.data.opacity = material.dissolve;

		m->materials.push_back(mat);
	}

	std::sort(m->shapes.begin(), m->shapes.end(), [m](Shape shape1, Shape shape2)->bool { return m->materials[shape1.materialID].data.opacity > m->materials[shape2.materialID].data.opacity; });

	VALIDATE(err.length() == 0, "%s", err.c_str());

	if (warn.length() > 1)
	{
		WARN("%s", warn.c_str());
	}
}

void createMeshTextureSampler(VkDevice device, VkSampler * textureSampler)
{
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.pNext = nullptr;
	samplerInfo.flags = 0;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;

	int result = vkCreateSampler(device, &samplerInfo, nullptr, textureSampler);
	if (result != VK_SUCCESS)
	{
		PANIC("RENDER_FRAMEWORK - Failed to create texture sampler %d", result);
	}
}

int getFileExtension(const char * filename)
{
	int i, j;

	char extension[10]; 

	enum extensions {
		dds = 0,
		png,
		jpg,
		bmp,
		other
	};

	for (i = 0; filename[i] != '\0'; i++);

	for ( ; filename[i-1] != '.'; i--);

	for (j = 0; filename[i] != '\0'; i++, j++)
		extension[j] = filename[i];

	extension[j] = '\0';

	if (strcmp(extension, "dds") == 0)
		return dds;

	if (strcmp(extension, "png") == 0)
		return png;

	if (strcmp(extension, "jpg") == 0)
		return jpg;

	if (strcmp(extension, "bmp") == 0)
		return bmp;

	return other;
}

bool loadMeshTexture(std::string name, Context * context, Renderer * renderer, Texture * texture)
{
	VALIDATE(texture != nullptr && name != "", "Failed to load texture \"%s\"", name.c_str());

	unsigned char * pixels = nullptr;
	dds_info imageInfo;
	int width, height, channels;
	VkDeviceSize imageSize;
	VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

	if (getFileExtension(name.c_str()) == 0)
	{
		static const dds_u32 supfmt[] = { DDS_FMT_R8G8B8A8, DDS_FMT_B8G8R8A8, DDS_FMT_B8G8R8X8, DDS_FMT_DXT1, DDS_FMT_DXT3, DDS_FMT_DXT5, 0 };
		int result = dds_load_from_file(name.c_str(), &imageInfo, supfmt);

		VALIDATE(result == DDS_SUCCESS, "RENDER_FRAMEWORK - Failed to load texture %s %d", name.c_str(), result);

		width = imageInfo.image.width;
		height = imageInfo.image.height;
		imageSize = imageInfo.image.size;

		pixels = (unsigned char *) malloc(imageSize);

		if (imageInfo.image.format == DDS_FMT_DXT5)
			format = VK_FORMAT_BC3_UNORM_BLOCK;
		if (imageInfo.image.format == DDS_FMT_B8G8R8A8)
			format = VK_FORMAT_B8G8R8A8_UNORM;

		dds_read(&imageInfo, pixels);

		dds_close(&imageInfo);
	}
	else
	{
		pixels = stbi_load(name.c_str(), &width, &height, &channels, STBI_rgb_alpha);
		imageSize = width * height * 4;
	}

	VALIDATE(pixels, "RENDER_FRAMEWORK - Failed to load texture %s", name.c_str());

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(context, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	             &stagingBuffer, &stagingBufferMemory);

	void * data;
	vkMapMemory(context->device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(((unsigned char *) data), pixels, imageSize);
	vkUnmapMemory(context->device, stagingBufferMemory);

	free(pixels);

	VkExtent2D e = {};
	e.width = width;
	e.height = height;

	createVkImage(context, VK_IMAGE_TYPE_2D, format, e, 1, 1, VK_SAMPLE_COUNT_1_BIT,
	            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
	            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texture->image, &texture->memory);

	transitionImageLayout(context->device, context->primaryGraphicsQueue->queue, context->primaryGraphicsQueue->commandPool, texture->image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1);

	copyBufferToImage(context, stagingBuffer, texture->image, static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1);

	transitionImageLayout(context->device, context->primaryGraphicsQueue->queue, context->primaryGraphicsQueue->commandPool, texture->image, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);

	createVkImageView(context, texture->image, format, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT, &texture->imageView);

	vkDestroyBuffer(context->device, stagingBuffer, nullptr);
	vkFreeMemory(context->device, stagingBufferMemory, nullptr);

	texture->format = format;

	return true;
}

void createVertexBuffer(Context * context, std::vector<Vertex>& vertices, VkBuffer * vertexBuffer,
                        VkDeviceMemory * vertexMemory, VkDeviceSize * vertexBufferSize)
{
	*vertexBufferSize = sizeof(Vertex) * vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	createBuffer(context, *vertexBufferSize, usage, properties, &stagingBuffer, &stagingBufferMemory);

	void * data;
	vkMapMemory(context->device, stagingBufferMemory, 0, *vertexBufferSize, 0, &data);
	memcpy(data, vertices.data(), (size_t) *vertexBufferSize);
	vkUnmapMemory(context->device, stagingBufferMemory);

	usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	createBuffer(context, *vertexBufferSize, usage, properties, vertexBuffer, vertexMemory);

	copyBuffer(context, stagingBuffer, *vertexBuffer, *vertexBufferSize);

	vkDestroyBuffer(context->device, stagingBuffer, nullptr);
	vkFreeMemory(context->device, stagingBufferMemory, nullptr);
}

void createIndexBuffer(Context * context, std::vector<uint32_t>& indices, VkBuffer * indexBuffer,
                       VkDeviceMemory * indexMemory, VkDeviceSize * indexBufferSize)
{
	*indexBufferSize = sizeof(uint32_t) * indices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	createBuffer(context, *indexBufferSize, usage, properties, &stagingBuffer, &stagingBufferMemory);

	void * data;
	vkMapMemory(context->device, stagingBufferMemory, 0, *indexBufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t) *indexBufferSize);
	vkUnmapMemory(context->device, stagingBufferMemory);

	usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	createBuffer(context, *indexBufferSize, usage, properties, indexBuffer, indexMemory);

	copyBuffer(context, stagingBuffer, *indexBuffer, *indexBufferSize);

	vkDestroyBuffer(context->device, stagingBuffer, nullptr);
	vkFreeMemory(context->device, stagingBufferMemory, nullptr);
}

void createInstanceBuffer(Context * context, std::vector<Instance>& instances, VkBuffer * instanceBuffer,
                          VkDeviceMemory * instanceMemory, VkDeviceSize * instanceBufferSize)
{
	*instanceBufferSize = sizeof(Instance) * instances.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	createBuffer(context, *instanceBufferSize, usage, properties, &stagingBuffer, &stagingBufferMemory);

	void * data;
	vkMapMemory(context->device, stagingBufferMemory, 0, *instanceBufferSize, 0, &data);
	memcpy(data, instances.data(), (size_t) *instanceBufferSize);
	vkUnmapMemory(context->device, stagingBufferMemory);

	usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	createBuffer(context, *instanceBufferSize, usage, properties, instanceBuffer, instanceMemory);

	copyBuffer(context, stagingBuffer, *instanceBuffer, *instanceBufferSize);

	vkDestroyBuffer(context->device, stagingBuffer, nullptr);
	vkFreeMemory(context->device, stagingBufferMemory, nullptr);
}

VkShaderModule loadShader(Context * context, std::string filename)
{
	VkShaderModule shaderModule;

	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<char> buffer(size);
	VALIDATE(file.read(buffer.data(), size), "RENDER_FRAMEWORK - Failed to read shader file %s", filename.c_str());

	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = size;
	createInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());
	int result = vkCreateShaderModule(context->device, &createInfo, nullptr, &shaderModule); 
	VALIDATE(result == VK_SUCCESS, "RENDER_FRAMEWORK - Failed to create shader module %d", result);

	DEBUG("RENDER_FRAMEWORK - VkShaderModule Created %s", filename.c_str());

	return shaderModule;
}

VkRenderPass createVkRenderPass(VkDevice device, VkFormat colorFormat, VkFormat depthFormat, VkSampleCountFlagBits sampleCount)
{
	VkRenderPass renderPass;

	std::vector<VkAttachmentDescription> attachments;

	VkAttachmentReference colorAttachmentRef = {};
	VkAttachmentReference depthAttachmentRef = {};
	VkAttachmentReference resolveAttachmentRef = {};

	VkAttachmentDescription colorAttachment = {};
	VkAttachmentDescription depthAttachment = {};
	VkAttachmentDescription resolveAttachment = {};

	if (colorFormat != VK_FORMAT_UNDEFINED)
	{
		colorAttachment.flags = 0;
		colorAttachment.format = colorFormat;
		colorAttachment.samples = sampleCount;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		colorAttachmentRef.attachment = attachments.size();
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		attachments.push_back(colorAttachment);
	}
	if (depthFormat != VK_FORMAT_UNDEFINED)
	{
		depthAttachment.flags = 0;
		depthAttachment.format = depthFormat;
		depthAttachment.samples = sampleCount;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		depthAttachmentRef.attachment = attachments.size();
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		attachments.push_back(depthAttachment);
	}
	if (sampleCount != VK_SAMPLE_COUNT_1_BIT)
	{
		resolveAttachment.flags = 0;
		resolveAttachment.format = colorFormat;
		resolveAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		resolveAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		resolveAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		resolveAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		resolveAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		resolveAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		resolveAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		resolveAttachmentRef.attachment = attachments.size();
		resolveAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		attachments.push_back(resolveAttachment);
	}

	VkSubpassDescription subpass = {};
	subpass.flags = 0;
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = nullptr;
	subpass.colorAttachmentCount = (colorFormat != VK_FORMAT_UNDEFINED) ? 1 : 0;
	subpass.pColorAttachments = (colorFormat != VK_FORMAT_UNDEFINED) ? &colorAttachmentRef : nullptr;
	subpass.pResolveAttachments = (sampleCount != VK_SAMPLE_COUNT_1_BIT) ? &resolveAttachmentRef : nullptr;
	subpass.pDepthStencilAttachment = (depthFormat != VK_FORMAT_UNDEFINED) ? &depthAttachmentRef : nullptr;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = nullptr;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pNext = nullptr;
	renderPassInfo.flags = 0;
	renderPassInfo.attachmentCount = attachments.size();
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 0;
	renderPassInfo.pDependencies = nullptr;

	int result = vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass);
	VALIDATE(result == VK_SUCCESS, "RENDER_FRAMEWORK - Failed to create RenderPass - error code: %d", result);

	DEBUG("RENDER_FRAMEWORK - VkRenderPass Created");

	return renderPass;
}

VkFramebuffer createVkFramebuffer(VkDevice device, const void * pNext, VkFramebufferCreateFlags flags,
                                  VkRenderPass renderPass, VkImageView colorImageView, VkImageView depthImageView,
						          VkImageView swapchainImageView, uint32_t width, uint32_t height, uint32_t layers)
{
	VkFramebuffer framebuffer;

	std::vector<VkImageView> attachments;

	if (colorImageView != VK_NULL_HANDLE)
		attachments.push_back(colorImageView);

	if (depthImageView != VK_NULL_HANDLE)
		attachments.push_back(depthImageView);

	if (swapchainImageView != VK_NULL_HANDLE)
		attachments.push_back(swapchainImageView);

	VkFramebufferCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.pNext = pNext;
	createInfo.flags = flags;
	createInfo.renderPass = renderPass;
	createInfo.attachmentCount = attachments.size();
	createInfo.pAttachments = attachments.data();
	createInfo.width = width;
	createInfo.height = height;
	createInfo.layers = layers;

	int result = vkCreateFramebuffer(device, &createInfo, nullptr, &framebuffer);
	VALIDATE(result == VK_SUCCESS, "RENDER_FRAMEWORK _ Failed to create framebuffer %d", result);

	return framebuffer;
}

std::string findFile(std::string filename, std::string root)
{
	std::filesystem::path path = std::filesystem::current_path().string() + "/" + root;

	for (auto iterator = std::filesystem::recursive_directory_iterator(path); iterator != std::filesystem::recursive_directory_iterator(); iterator++)
	{
		if (iterator->path().filename().string() == filename)
			return iterator->path().parent_path().string() + "/";
	}

	return "";
}