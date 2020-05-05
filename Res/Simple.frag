#version 450 core

layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexcoord;

layout(binding=0) uniform sampler2D texColor;

out vec4 fragColor;

void main()
{
  fragColor = inColor * texture(texColor, inTexcoord);
}
