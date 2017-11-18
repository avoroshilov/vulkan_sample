#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "shader_bindings.h"

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec4 in_color;
layout(location = 3) in vec2 in_texCoords;

layout(set = 0, binding = SH_BIND_GLOBAL_CONSTANTS) uniform UniformBufferObject
{
	float time;
} ubo;

layout(set = 0, binding = SH_BIND_TRANSFORM) uniform TransformUBO
{
	mat4 view;
	mat4 proj;
} transformUBO;

layout(push_constant) uniform MeshPushConst
{
	mat4 model;
} meshPushConst;

out gl_PerVertex
{
	vec4 gl_Position;
};

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec2 out_texCoords;
layout(location = 2) out float out_time;

void main()
{
	gl_Position = vec4(in_position, 1.0);
	out_texCoords = in_texCoords;
	out_color = in_color;
	out_time = ubo.time;
}