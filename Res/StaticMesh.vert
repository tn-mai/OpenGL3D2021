#version 450

// 入力変数
layout(location=0) in vec3 vPosition;
layout(location=1) in vec4 vColor;
layout(location=2) in vec2 vTexcoord;
layout(location=3) in vec3 vNormal;

// 出力変数
layout(location=0) out vec4 outColor;
layout(location=1) out vec2 outTexcoord;
layout(location=2) out vec3 outNormal;
layout(location=3) out vec3 outPosition;

out gl_PerVertex {
  vec4 gl_Position;
};

// ユニフォーム変数
layout(location=0) uniform mat4 matVP;
layout(location=1) uniform mat4 matModel;
layout(location=10) uniform vec4 materialColor;

// 頂点シェーダプログラム
void main()
{
  outColor = vColor * materialColor;
  outTexcoord = vTexcoord;

  mat3 matNormal = transpose(inverse(mat3(matModel)));
  outNormal = normalize(matNormal * vNormal);

  outPosition = vec3(matModel * vec4(vPosition, 1.0));
  gl_Position = matVP * vec4(outPosition, 1.0);
}

