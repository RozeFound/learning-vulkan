#version 450

// vulkan NDC:	x: -1(left), 1(right)
//				y: -1(top), 1(bottom)

layout(binding = 0) uniform mvp {
	mat4x4 model;
	mat4x4 view;
	mat4x4 projection;
} ubo;

layout(std140, binding = 1) readonly buffer storage {
	mat4x4 model[];
} data;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() {
	gl_Position = ubo.projection * ubo.view * data.model[gl_InstanceIndex] * vec4(inPosition, 0.0, 1.0);
	fragColor = inColor;
}