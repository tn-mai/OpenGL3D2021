#version 450 core

// 入力変数
layout(location=0) in vec3 vPosition;
layout(location=1) in vec4 vColor;
layout(location=2) in vec2 vTexcoord;
layout(location=3) in vec3 vNormal;

// 出力変数
layout(location=0) out vec4 outColor;
layout(location=1) out vec2 outTexcoord;
layout(location=2) out vec3 outPosition;
layout(location=3) out vec3 outNormal;
out gl_PerVertex {
  vec4 gl_Position;
};

// ユニフォーム変数
layout(location=0) uniform mat4 matMVP;
layout(location=1) uniform mat4 matModel;
layout(location=2) uniform vec4 objectColor;

// 頂点シェーダプログラム
void main()
{
  outColor = vColor * objectColor;
  outTexcoord = vTexcoord;
  outPosition = vec3(matModel * vec4(vPosition, 1));
  mat3 matNormal = transpose(inverse(mat3(matModel)));
  outNormal = matNormal * vNormal;
  gl_Position = matMVP * vec4(vPosition, 1.0);
}

