/*
 * Copyright (c) 2021, NVIDIA CORPORATION.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
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
 *
 * SPDX-FileCopyrightText: Copyright (c) 2014-2021 NVIDIA CORPORATION
 * SPDX-License-Identifier: Apache-2.0
 */


/*
 * More complex example for hardware accelerated ray tracing using VK_KHR_ray_tracing_pipeline and VK_KHR_acceleration_structure
 */

#pragma once

#include "api_vulkan_sample.h"
#include "glsl_compiler.h"

struct ObjMaterial
{
	glm::vec3 diffuse{0.7f, 0.7f, 0.7f};
	glm::vec3 specular{0.7f, 0.7f, 0.7f};
	float     shininess{0.f};
};

struct ObjVertex
{
	glm::vec3 pos;
	glm::vec3 nrm;
};

struct ObjModelCpu
{
	std::vector<ObjVertex> vertices;
	std::vector<uint32_t>  indices;
	std::vector<int32_t>   mat_index;
};

struct ObjModelGpu
{
	uint32_t                           nb_indices{0};
	uint32_t                           nb_vertices{0};
	std::unique_ptr<vkb::core::Buffer> vertex_buffer;           // Device buffer of all 'Vertex'
	std::unique_ptr<vkb::core::Buffer> index_buffer;            // Device buffer of the indices forming triangles
	std::unique_ptr<vkb::core::Buffer> mat_color_buffer;        // Device buffer of array of 'Wavefront material'
	std::unique_ptr<vkb::core::Buffer> mat_index_buffer;        // Device buffer of array of 'Wavefront material'
};

class RaytracingReflection : public ApiVulkanSample
{
	struct AccelerationStructure
	{
		VkAccelerationStructureKHR         handle;
		std::unique_ptr<vkb::core::Buffer> buffer;
	};

  public:
	VkPhysicalDeviceRayTracingPipelinePropertiesKHR  ray_tracing_pipeline_properties{};
	VkPhysicalDeviceAccelerationStructureFeaturesKHR acceleration_structure_features{};

	std::vector<AccelerationStructure> bottom_level_acceleration_structure;
	AccelerationStructure              top_level_acceleration_structure;

	// Array of objects and instances in the scene
	std::vector<ObjModelGpu> obj_models;

	//std::vector<std::unique_ptr<const vkb::core::Buffer>> mat_buffers;
	std::unique_ptr<vkb::core::Buffer>                vertex_buffer;
	std::unique_ptr<vkb::core::Buffer>                index_buffer;
	uint32_t                                          index_count;
	std::vector<VkRayTracingShaderGroupCreateInfoKHR> shader_groups{};

	std::unique_ptr<vkb::core::Buffer> raygen_shader_binding_table;
	std::unique_ptr<vkb::core::Buffer> miss_shader_binding_table;
	std::unique_ptr<vkb::core::Buffer> hit_shader_binding_table;

	struct StorageImage
	{
		VkDeviceMemory memory;
		VkImage        image = VK_NULL_HANDLE;
		VkImageView    view;
		VkFormat       format;
		uint32_t       width;
		uint32_t       height;
	} storage_image;

	struct UniformData
	{
		glm::mat4 view_inverse;
		glm::mat4 proj_inverse;
	} uniform_data;
	std::unique_ptr<vkb::core::Buffer> ubo;

	VkPipeline            pipeline;
	VkPipelineLayout      pipeline_layout;
	VkDescriptorSet       descriptor_set;
	VkDescriptorSetLayout descriptor_set_layout;

	RaytracingReflection();
	~RaytracingReflection();

	void                               create_storage_image();
	void                               create_bottom_level_acceleration_structure(ObjModelGpu &obj_model);
	void                               create_top_level_acceleration_structure(std::vector<VkAccelerationStructureInstanceKHR> &blas_instances);
	void                               load_model(const std::string &file_name, std::shared_ptr<ObjMaterial> mat = {});
	void                               create_model(ObjModelCpu &obj, const std::vector<ObjMaterial> &materials);
	VkAccelerationStructureInstanceKHR create_blas_instance(uint32_t blas_id, glm::mat4 &mat);
	void                               delete_acceleration_structure(AccelerationStructure &acceleration_structure);
	void                               create_scene();
	void                               create_shader_binding_tables();
	void                               create_descriptor_sets();
	void                               create_ray_tracing_pipeline();
	void                               create_uniform_buffer();
	void                               update_uniform_buffers();
	void                               draw();

	void build_command_buffers() override;
	void request_gpu_features(vkb::PhysicalDevice &gpu) override;
	bool prepare(vkb::Platform &platform) override;
	void render(float delta_time) override;
};

std::unique_ptr<vkb::VulkanSample> create_ray_tracing_reflection();
