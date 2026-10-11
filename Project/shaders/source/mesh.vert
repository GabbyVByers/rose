
/*
 *   Vertex Shader [mesh.vert]
 */

#version 460 core

layout(location = 0) in vec3 Position3D;
layout(location = 1) in vec4 Color;
layout(location = 2) in vec2 TexCoords;

layout(set = 1, binding = 0) uniform _ {
  mat4 TransformMatrix;
};

layout(location = 0) out vec4 VertexColor;
layout(location = 1) out vec2 VertexTexCoords;

void main() {
  gl_Position = TransformMatrix * vec4(Position3D, 1.0f);
  VertexColor = Color;
  VertexTexCoords = TexCoords;
}

