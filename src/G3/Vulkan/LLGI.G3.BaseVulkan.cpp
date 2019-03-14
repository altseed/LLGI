#include "LLGI.G3.BaseVulkan.h"

namespace LLGI
{
namespace G3
{
void SetImageLayout(vk::CommandBuffer cmdbuffer,
					vk::Image image,
					vk::ImageLayout oldImageLayout,
					vk::ImageLayout newImageLayout,
					vk::ImageSubresourceRange subresourceRange)
{
	vk::ImageMemoryBarrier imageMemoryBarrier;
	imageMemoryBarrier.oldLayout = oldImageLayout;
	imageMemoryBarrier.newLayout = newImageLayout;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange = subresourceRange;

	// current layout

	if (oldImageLayout == vk::ImageLayout::ePreinitialized)
		imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eHostWrite;
	else if (oldImageLayout == vk::ImageLayout::eTransferDstOptimal)
		imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
	else if (oldImageLayout == vk::ImageLayout::eColorAttachmentOptimal)
		imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
	else if (oldImageLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
		imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
	else if (oldImageLayout == vk::ImageLayout::eTransferSrcOptimal)
		imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
	else if (oldImageLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
		imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;

	// next layout

	if (newImageLayout == vk::ImageLayout::eTransferDstOptimal)
		imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
	else if (newImageLayout == vk::ImageLayout::eTransferSrcOptimal)
		imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferRead; // | imageMemoryBarrier.srcAccessMask;
	else if (newImageLayout == vk::ImageLayout::eColorAttachmentOptimal)
		imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
	else if (newImageLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
		imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
	else if (newImageLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
		imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

	// Put barrier on top
	// Put barrier inside setup command buffer
	cmdbuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,
							  vk::PipelineStageFlagBits::eTopOfPipe,
							  vk::DependencyFlags(),
							  nullptr,
							  nullptr,
							  imageMemoryBarrier);
}

} // namespace G3
} // namespace LLGI