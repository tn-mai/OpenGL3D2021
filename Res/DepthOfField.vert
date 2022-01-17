/**
* @file DepthOfField.vert
*/
#version 450

// 入力変数
layout(location=0) in vec3 vPosition;
layout(location=2) in vec2 vTexCoord;

// 出力変数
layout(location=1) out vec2 outTexCoord;
out gl_PerVertex {
  vec4 gl_Position;
};

// ユニフォーム変数
layout(location=0) uniform mat4 matMVP;

// 頂点シェーダープログラム
void main()
{
  outTexCoord = vTexCoord;
  gl_Position = matMVP * vec4(vPosition, 1.0);
}
