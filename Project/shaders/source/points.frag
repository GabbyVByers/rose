
/*
 *   Fragment Shader [points.frag]
 */

#version 460 core

layout(location = 0) in vec4 VertexColor;
layout(location = 1) flat in float PointDiameter;
layout(location = 0) out vec4 FragColor;

void main() {
	vec2 offset = (gl_PointCoord - 0.5f) * PointDiameter;
	float cutoff = max(PointDiameter * 0.5f, 1.0f);
	if (length(offset) > cutoff) discard;
	FragColor = VertexColor;
}

