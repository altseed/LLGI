
#pragma once

#include <Vulkan/LLGI.CommandListVulkan.h>
#include <Vulkan/LLGI.GraphicsVulkan.h>
#include <Vulkan/LLGI.PlatformVulkan.h>
#include <Vulkan/LLGI.RenderPassVulkan.h>
#include "../thirdparty/imgui/imgui_impl_vulkan.h"
#include "ImGuiPlatform.h"


class ImguiPlatformVulkan : ImguiPlatform
{
	LLGI::GraphicsVulkan* g_ = nullptr;
	LLGI::PlatformVulkan* p_ = nullptr;
	VkDescriptorPool descriptorPool_ = VK_NULL_HANDLE;

public:
	ImguiPlatformVulkan(LLGI::Graphics* g, LLGI::Platform* p)
		: g_(static_cast<LLGI::GraphicsVulkan*>(g)), p_(static_cast<LLGI::PlatformVulkan*>(p))
	{
		{
			VkDescriptorPoolSize pool_sizes[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
												 {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
												 {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
												 {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
												 {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
												 {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
												 {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
												 {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
												 {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
												 {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
												 {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};
			VkDescriptorPoolCreateInfo pool_info = {};
			pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
			pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
			pool_info.pPoolSizes = pool_sizes;
			vkCreateDescriptorPool(g_->GetDevice(), &pool_info, nullptr, &descriptorPool_);
		}

		ImGui_ImplVulkan_InitInfo info;
		info.Instance = p_->GetInstance();
		info.PhysicalDevice = p_->GetPhysicalDevice();
		info.Device = g_->GetDevice();
		info.QueueFamily = p_->GetQueueFamilyIndex();
		info.Queue = p_->GetQueue();
		info.PipelineCache = p_->GetPipelineCache();
		info.DescriptorPool = descriptorPool_;
		info.MinImageCount = p_->GetSwapBufferCountMin();
		info.ImageCount = p_->GetSwapBufferCount();
		info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		// ImGui_ImplVulkan_Init(&info, ); TODO
	}

	virtual ~ImguiPlatformVulkan()
	{
		vkDestroyDescriptorPool(g_->GetDevice(), descriptorPool_, nullptr);
		ImGui_ImplVulkan_Shutdown();
	}

	void NewFrame() override { ImGui_ImplVulkan_NewFrame(); }

	void RenderDrawData(ImDrawData* draw_data, LLGI::CommandList* commandList) override
	{
		auto cl = static_cast<LLGI::CommandListVulkan*>(commandList);
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cl->GetCommandBuffer());
	}
};