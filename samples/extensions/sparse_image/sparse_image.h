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

size_t const ON_SCREEN_HORIZONTAL_BLOCKS = 50;
size_t const ON_SCREEN_VERTICAL_BLOCKS   = 30;
double const FOV_DEGREES                 = 60.0;
double const MIP_LEVEL_MARGIN            = 0.2;

class SparseImage : public ApiVulkanSample
{
  public:

	enum Stages
	{
		SPARSE_IMAGE_IDLE_STAGE,
		SPARSE_IMAGE_CALCULATE_MESH_STAGE,
		SPARSE_IMAGE_REQUIRED_MEMORY_LAYOUT_STAGE,
		SPARSE_IMAGE_BIND_PAGES_PRE_MIP_GEN_STAGE,
		SPARSE_IMAGE_UPDATE_LEVEL0_STAGE,
		SPARSE_IMAGE_GENERATE_MIPS_STAGE,
		SPARSE_IMAGE_BIND_PAGES_POST_MIP_GEN_STAGE,
		SPARSE_IMAGE_FREE_MEMORY_STAGE,
		SPARSE_IMAGE_NUM_STAGES,
	};

	struct MVP
	{
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};

	struct SimpleBuffer
	{
		VkBuffer       buffer;
		VkDeviceMemory memory;
		void          *mapped_memory;
		size_t         num_elements;
		size_t         size;
	};

	struct SimpleVertex
	{
		glm::vec2 norm;
		glm::vec2 uv;
	};

	struct MipProperties
	{
		size_t num_rows;
		size_t num_columns;
		size_t mip_num_pages;
		size_t mip_base_page_index;        
		size_t width;
		size_t height;
	};

	struct TextureBlock
	{
		uint32_t row;
		uint32_t column;
		uint8_t  old_mip_level;
		uint8_t  new_mip_level;
		bool     on_screen;
	};

	struct MemPageDescription
	{
		size_t  x;
		size_t  y;
		uint8_t mip_level;
	};

	struct PageTable
	{
		bool                                 bound;
		bool                                 valid;
		bool                                 gen_mip_required;   
		bool                                 fixed;
		size_t                               memory_index;
		std::list<std::pair<size_t, size_t>> render_required_list;
	};

	struct Point
	{
		double x;
		double y;
		bool   on_screen;
	};

	struct MipBlock
	{
		uint8_t mip_level;
		bool    on_screen;
	};

	struct VirtualTexture
	{
		VkImage        texture_image;
		VkImageView    texture_image_view;
		VkDeviceMemory texture_memory;        
		
		std::vector<VkSparseImageMemoryBind> sparse_image_memory_bind;

		std::unique_ptr<vkb::sg::Image> row_data_image;

		std::unique_ptr<vkb::core::Buffer> single_page_buffer;

		std::list<size_t> available_memory_index_list;

		// dimensions
		size_t width;
		size_t height;

		// number of virtual pages (what if the total image was allocated)
		size_t num_pages;
		size_t page_size;

		// table that includes data on which page is allocated to what memory block from the textureMemory vector.
		std::vector<struct PageTable> page_table;

		// list containing information which pages from the virtual should be bound
		std::list<size_t> bind_list;

		// list containing information which pages from the virtual should be updated
		std::list<size_t> update_list;

		// list containing information which pages should be freed (present but not required or required for mip generation)
		std::list<size_t> free_list;

		bool update_required = false;

		// sparse image - related format and memory properties
		VkSparseImageFormatProperties   format_properties;
		VkSparseImageMemoryRequirements memory_sparse_requirements;
		VkMemoryRequirements            mem_requirements;

		std::vector<std::vector<MipBlock>> current_mip_table;
		std::vector<std::vector<MipBlock>> new_mip_table;
		std::list<struct TextureBlock>     texture_block_update_list;

		uint8_t base_mip_level;
		uint8_t mip_levels;

		std::vector<struct MipProperties> mip_properties;
	};


    struct CalculateMipLevelData
	{
		std::vector<std::vector<Point>>          mesh;
		std::vector<std::vector<MipBlock>>       mip_table;

		uint32_t vertical_num_blocks;
		uint32_t horizontal_num_blocks;

		uint8_t mip_levels;

		std::vector<float>                       ax_vertical;
		std::vector<float>                       ax_horizontal;

		glm::mat4  mvp_transform;

		VkExtent2D texture_base_dim;
		VkExtent2D screen_base_dim;

		CalculateMipLevelData(glm::mat4 mvp_transform, VkExtent2D texture_base_dim, VkExtent2D screen_base_dim, uint32_t vertical_num_blocks, uint32_t horizontal_num_blocks, uint8_t mip_levels);
		void calculate_mesh_coordinates();
		void calculate_mip_levels();
	};

	enum Stages           next_stage;

	struct VirtualTexture virtual_texture;

	VkQueue sparse_queue;
	size_t  sparse_queue_family_index;

	std::unique_ptr<vkb::core::Buffer> vertex_buffer;

	std::unique_ptr<vkb::core::Buffer> index_buffer;
	size_t                             index_count;

	std::unique_ptr<vkb::core::Buffer> mvp_buffer;

	glm::mat4 current_mvp_transform;

	VkPipeline       sample_pipeline{};
	VkPipelineLayout sample_pipeline_layout{};
	
	VkDescriptorSetLayout descriptor_set_layout;
	VkDescriptorSet       descriptor_set;
	VkSampler             texture_sampler;

	size_t on_screen_num_vertical_blocks   = ON_SCREEN_VERTICAL_BLOCKS;
	size_t on_screen_num_horizontal_blocks = ON_SCREEN_HORIZONTAL_BLOCKS;
	double fov_degrees                     = FOV_DEGREES;
	double mip_level_margin                = MIP_LEVEL_MARGIN;


	SparseImage();
	virtual ~SparseImage();

	void setup_camera();
	void load_assets();

	void prepare_pipelines();

	void create_sparse_bind_queue();

	void create_vertex_buffer();
	void create_index_buffer();

	void create_uniform_buffer();
	void create_texture_sampler();

	void create_descriptor_set_layout();
	void create_descriptor_pool();
	void create_descriptor_sets();

	void create_sparse_texture_image();

	void                      update_mvp();
	struct MemPageDescription get_mem_page_description(size_t memory_index);
	void                      calculate_mips_table(glm::mat4 mvp_transform, uint32_t numVerticalBlocks, uint32_t numHorizontalBlocks, std::vector<std::vector<MipBlock>> &mipTable);
	void                      compare_mips_table();
	void                      calculate_required_memory_layout();
	void                      process_texture_block(const TextureBlock &on_screen_block);
	void                      get_memory_dependency_for_the_block(size_t column, size_t row, uint8_t mip_level, std::list<size_t> &index_list);
	void                      check_mip_page_requirements(std::list<MemPageDescription> &mipgen_required_list, struct MemPageDescription mip_dependency);
	void                      bind_sparse_image();
	void                      separate_single_row_data_block(uint8_t buffer[], const VkExtent2D blockDim, VkOffset2D offset, size_t stride);
	void                      transition_image_layout(VkImage image, VkImageLayout old_layout, VkImageLayout new_layout, uint8_t mip_level);
	uint8_t                   get_mip_level(size_t page_index);
	size_t                    get_memory_index(struct MemPageDescription mem_page_desc);

	// Override basic framework functionality
	void build_command_buffers() override;
	void render(float delta_time) override;
	void view_changed() override;
	bool prepare(const vkb::ApplicationOptions &options) override;
	void request_gpu_features(vkb::PhysicalDevice &gpu) override;
};

std::unique_ptr<vkb::VulkanSample> create_sparse_image();
