
#version 460 core

layout(location = 0) in vec2 Position2D;
layout(location = 1) in vec2 TexCoords;

layout(set = 1, binding = 0) uniform _ {
  mat4 TransformMatrix;
  vec4 TintColor;
};

layout(location = 0) out vec4 VertexColor;
layout(location = 1) out vec2 VertexTexCoords;

void main() {
  gl_Position = TransformMatrix * vec4(Position2D, 0.0f, 1.0f);
  VertexColor = TintColor;
  VertexTexCoords = TexCoords;
}

