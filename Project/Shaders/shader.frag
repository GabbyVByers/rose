
#version 460 core

layout(location = 0) in vec4 VertexColor;
layout(location = 1) in vec2 VertexTexCoords;

layout(set = 2, binding = 0) uniform sampler2D AlbedoTexture;

layout(location = 0) out vec4 FragColor;

void main() {
  FragColor = texture(AlbedoTexture, VertexTexCoords) * VertexColor;
}

