#version 450 core

layout(location=0) in vec3 vPosition;
layout(location=1) in vec4 vColor;
layout(location=2) in vec2 vTexcoord;

layout(location=0) out vec4 outColor;
layout(location=1) out vec2 outTexcoord;

out gl_PerVertex {
  vec4 gl_Position;
};

layout(location=0) uniform mat4 matMVP;

void main()
{
  outColor = vColor;
  outTexcoord = vTexcoord;
  gl_Position = matMVP * vec4(vPosition, 1.0);
}
