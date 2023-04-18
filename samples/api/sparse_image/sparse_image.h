/* Copyright (c) 2023, Mobica Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 the "License";
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "api_vulkan_sample.h"

class SparseImage : public ApiVulkanSample
{
  public:
	struct UBOCOMM
	{
		glm::mat4 projection;
		glm::mat4 view;
	} ubo_vs;

	std::unique_ptr<vkb::core::Buffer> ubo;

	std::unique_ptr<vkb::sg::SubMesh> object;

	VkPipeline            pipeline{VK_NULL_HANDLE};
	VkDescriptorSet       descriptor_set{VK_NULL_HANDLE};
	VkPipelineLayout      pipeline_layout{VK_NULL_HANDLE};
	VkDescriptorSetLayout descriptor_set_layout{VK_NULL_HANDLE};

	struct SparseImageResource
	{
		VkImage        image{VK_NULL_HANDLE};
		VkDeviceMemory memory{VK_NULL_HANDLE};
		VkImageLayout  layout{};
	} sparse_image;

	struct Texture
	{
		std::unique_ptr<vkb::sg::Image> image;
		//	uint32_t width{};
		//	uint32_t height{};
		uint32_t                        mip_levels{};
		VkImageSubresourceRange         sub_range{};
		VkFormat                        format{VK_FORMAT_R8G8B8A8_SRGB};
		VkSampler                       sampler{VK_NULL_HANDLE};
		VkImageView                     view{VK_NULL_HANDLE};
		VkSparseImageMemoryRequirements sparse_image_memory_requirements;
		uint32_t                        memory_type_index{};

		bool single_mip_tail;
		bool alinged_mip_size;
	} texture;

	SparseImage();
	virtual ~SparseImage();

	// Override basic framework functionality
	void build_command_buffers() override;
	void render(float delta_time) override;
	bool prepare(vkb::Platform &platform) override;
	void request_gpu_features(vkb::PhysicalDevice &gpu) override;
	void on_update_ui_overlay(vkb::Drawer &drawer) override;

	void set_camera();
	void load_assets();
	void prepare_pipeline();
	void prepare_uniform_buffers();
	void update_uniform_buffers();
	void setup_descriptor_set_layout();
	void create_descriptor_set();
	void create_descriptor_pool();
	void draw();

	void                  create_sparse_image();
	void                  create_sparse_texture();
	VkDescriptorImageInfo create_image_descriptor();
};

std::unique_ptr<vkb::VulkanSample> create_sparse_image();
