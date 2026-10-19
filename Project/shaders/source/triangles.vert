
/*
 *   Vertex Shader [triangles.vert]
 */

#version 460 core

layout(location = 0) in vec2 Position2D;

layout(set = 1, binding = 0) uniform _ {
	mat4 TransformMatrix;
	vec4 Color;
};

layout(location = 0) out vec4 VertexColor;

void main() {
	gl_Position = TransformMatrix * vec4(Position2D, 0.0f, 1.0f);
	VertexColor = Color;
}

