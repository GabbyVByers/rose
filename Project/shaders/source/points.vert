
/*
 *   Vertex Shader [points.vert]
 */

#version 460 core

layout(location = 0) in vec2 Position2D;

layout(set = 1, binding = 0) uniform _ {
	mat4 TransformMatrix;
	vec4 Color;
	float PixelDiameter;
};

layout(location = 0) out vec4 VertexColor;
layout(location = 1) flat out float PointDiameter;

void main() {
	gl_Position = TransformMatrix * vec4(Position2D, 0.0f, 1.0f);
	gl_PointSize = PixelDiameter;
	VertexColor = Color;
	PointDiameter = PixelDiameter;
}

