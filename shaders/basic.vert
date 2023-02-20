#version 450

// vulkan NDC:	x: -1(left), 1(right)
//				y: -1(top), 1(bottom)

vec2 vertecies[3] = vec2[](
	vec2(0.0, -0.05),
	vec2(0.05, 0.05),
	vec2(-0.05, 0.05)
);

vec3 colors[3] = vec3[](
	vec3(1.0, 0.0, 0.0),
	vec3(0.0, 1.0, 0.0),
	vec3(0.0, 0.0, 1.0)
);

layout(push_constant) uniform constants {
	mat4x4 matrix;
} positions;

layout(location = 0) out vec3 fragColor;

void main() {
	gl_Position = positions.matrix * vec4(vertecies[gl_VertexIndex], 0.0, 1.0);
	fragColor = colors[gl_VertexIndex];
}