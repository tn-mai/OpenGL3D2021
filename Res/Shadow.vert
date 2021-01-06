#version 450 core

// 入力変数
layout(location=0) in vec3 vPosition;
layout(location=2) in vec2 vTexcoord;
layout(location=4) in vec3 vPosition2;

// 出力変数
layout(location=1) out vec2 outTexcoord;

out gl_PerVertex {
  vec4 gl_Position;
};

// ユニフォーム変数
layout(location=0) uniform mat4 matMVP;
layout(location=10) uniform float morphWeight;

// 頂点シェーダプログラム
void main()
{
  vec3 position = mix(vPosition, vPosition2, morphWeight);

  outTexcoord = vTexcoord;
  gl_Position = matMVP * vec4(position, 1.0);
}
