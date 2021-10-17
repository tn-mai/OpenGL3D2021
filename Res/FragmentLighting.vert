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
layout(location=0) uniform mat4 matTRS;
layout(location=1) uniform mat4 matModel;

// 頂点シェーダプログラム
void main()
{
  // 回転行列を取り出す.
  mat3 matNormal = transpose(inverse(mat3(matModel)));

  // ワールド座標系の法線を計算.
  vec3 worldNormal = normalize(matNormal * vNormal);

  outColor = vColor;
  outTexcoord = vTexcoord;
  outNormal = worldNormal;
  outPosition = vec3(matModel * vec4(vPosition, 1.0));
  gl_Position = matTRS * vec4(vPosition, 1.0);
}

