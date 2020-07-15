#version 320 es
/* Copyright (c) 2019-2020, Arm Limited and Contributors
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

precision highp float;

#ifdef HAS_BASE_COLOR_TEXTURE
layout(set = 0, binding = 0) uniform sampler2D base_color_texture;
#endif

layout(location = 0) in vec4 in_pos;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_normal;

layout(location = 0) out vec4 o_color;

layout(set = 0, binding = 1) uniform GlobalUniform
{
	mat4 model;
	mat4 view_proj;
	vec3 camera_position;
}
global_uniform;

struct Light
{
	vec4 position;         // position.w represents type of light
	vec4 color;            // color.w represents light intensity
	vec4 direction;        // direction.w represents range
	vec2 info;             // (only used for spot lights) info.x represents light inner cone angle, info.y represents light outer cone angle
};

layout(set = 0, binding = 4) uniform LightsInfo
{
	Light directional_lights[MAX_FORWARD_LIGHT_COUNT];
	Light point_lights[MAX_FORWARD_LIGHT_COUNT];
	Light spot_lights[MAX_FORWARD_LIGHT_COUNT];
}
lights_info;

// Push constants come with a limitation in the size of data.
// The standard requires at least 128 bytes
layout(push_constant, std430) uniform PBRMaterialUniform
{
	vec4  base_color_factor;
	float metallic_factor;
	float roughness_factor;
}
pbr_material_uniform;

layout (constant_id = 0) const uint DIRECTIONAL_LIGHT_COUNT = 0U;
layout (constant_id = 1) const uint POINT_LIGHT_COUNT = 0U;
layout (constant_id = 2) const uint SPOT_LIGHT_COUNT = 0U;

vec3 apply_directional_light(uint index, vec3 normal)
{
	Light light = lights_info.directional_lights[index];
	vec3 world_to_light = -light.direction.xyz;
	world_to_light = normalize(world_to_light);
	float ndotl = clamp(dot(normal, world_to_light), 0.0, 1.0);
	return ndotl * light.color.w * light.color.rgb;
}

vec3 apply_point_light(uint index, vec3 normal)
{
	Light light = lights_info.point_lights[index];
	vec3 world_to_light = light.position.xyz - in_pos.xyz;
	float dist = length(world_to_light);
	float atten = 1.0 / (dist * dist);
	world_to_light = normalize(world_to_light);
	float ndotl = clamp(dot(normal, world_to_light), 0.0, 1.0);
	return ndotl * light.color.w * atten * light.color.rgb;
}

vec3 apply_spot_light(uint index, vec3 normal)
{
	Light light = lights_info.spot_lights[index];
	vec3 light_to_pixel = normalize(in_pos.xyz - light.position.xyz);
	float theta = dot(light_to_pixel, normalize(light.direction.xyz));
	float inner_cone_angle = light.info.x;
	float outer_cone_angle = light.info.y;
	float intensity = (theta - outer_cone_angle) / (inner_cone_angle - outer_cone_angle);
	return smoothstep(0.0, 1.0, intensity) * light.color.w * light.color.rgb;
}

void main(void)
{
	vec3 normal = normalize(in_normal);

	vec3 light_contribution = vec3(0.0);

	for (uint i = 0U; i < DIRECTIONAL_LIGHT_COUNT; ++i)
	{
		light_contribution += apply_directional_light(i, normal);
	}

	for (uint i = 0U; i < POINT_LIGHT_COUNT; ++i)
	{
		light_contribution += apply_point_light(i, normal);
	}

	for (uint i = 0U; i < SPOT_LIGHT_COUNT; ++i)
	{
		light_contribution += apply_spot_light(i, normal);
	}

	vec4 base_color = vec4(1.0, 0.0, 0.0, 1.0);

#ifdef HAS_BASE_COLOR_TEXTURE
	base_color = texture(base_color_texture, in_uv);
#else
	base_color = pbr_material_uniform.base_color_factor;
#endif

	vec3 ambient_color = vec3(0.2) * base_color.xyz;

	o_color = vec4(ambient_color + light_contribution * base_color.xyz, base_color.w);
}
