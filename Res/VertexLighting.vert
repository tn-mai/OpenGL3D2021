#version 450 core

// 入力変数
layout(location=0) in vec3 vPosition;
layout(location=1) in vec4 vColor;
layout(location=2) in vec2 vTexcoord;
layout(location=3) in vec3 vNormal;

// 出力変数
layout(location=0) out vec4 outColor;
layout(location=1) out vec2 outTexcoord;
out gl_PerVertex {
  vec4 gl_Position;
};

// ユニフォーム変数
layout(location=0) uniform mat4 matMVP;
layout(location=1) uniform mat4 matModel;

// 並行光源
struct DirectionalLight {
  vec4 direction;
  vec4 color;
};
layout(location=2) uniform DirectionalLight directionalLight;

// 頂点シェーダプログラム
void main()
{
  mat3 matNormal = transpose(inverse(mat3(matModel)));
  vec3 worldNormal = matNormal * vNormal;
  float theta = max(dot(worldNormal, directionalLight.direction.xyz), 0);
  outColor = vColor;
  outColor.rgb *= directionalLight.color.rgb * theta;
  outTexcoord = vTexcoord;
  gl_Position = matMVP * vec4(vPosition, 1.0);
}
