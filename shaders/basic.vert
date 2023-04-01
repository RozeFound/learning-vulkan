#version 450

// vulkan NDC:	x: -1(left), 1(right)
//				y: -1(top), 1(bottom)

layout(push_constant) uniform constants {
	mat4x4 model;
	mat4x4 view;
	mat4x4 projection;
	mat4x4 pvm;
} mvp;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {

	gl_Position = mvp.pvm * vec4(inPosition, 1.0);

	fragColor = inColor;
	fragTexCoord = inTexCoord;
	
}