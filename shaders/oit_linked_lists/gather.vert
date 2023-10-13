#version 450
/* Copyright (c) 2023, Maximilien Dagois
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

layout (location = 0) in vec3 inPos;

struct ObjectDesc
{
	mat4 model;
	vec4 color;
};

const uint kObjectCount = 64;

layout(set = 0, binding = 0) uniform SceneConstants
{
	mat4 projection;
	mat4 view;
	uvec4 parameters;
} sceneConstants;
layout (binding = 1) uniform InstanceData
{
	ObjectDesc desc[kObjectCount];
} instanceData;

layout (location = 0) out vec4 outColor;

void main()
{
	const ObjectDesc desc = instanceData.desc[gl_InstanceIndex];
	gl_Position = sceneConstants.projection * sceneConstants.view * desc.model * vec4(inPos, 1.0f);
	outColor = desc.color;
}
