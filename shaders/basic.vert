#version 450

// vulkan NDC:	x: -1(left), 1(right)
//				y: -1(top), 1(bottom)

layout(binding = 0) uniform mvp {
	mat4x4 model;
	mat4x4 view;
	mat4x4 projection;
} ubo;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(push_constant) uniform constants {
	mat4x4 model;
} data;

layout(location = 0) out vec3 fragColor;

void main() {
	gl_Position = ubo.projection * ubo.view * data.model * vec4(inPosition, 0.0, 1.0);
	fragColor = inColor;
}