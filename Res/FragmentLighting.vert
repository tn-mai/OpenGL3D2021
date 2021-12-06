#version 450

// 入力変数
layout(location=0) in vec3 vPosition;
layout(location=1) in vec4 vColor;
layout(location=2) in vec2 vTexcoord;
layout(location=3) in vec3 vNormal;
layout(location=4) in uvec2 vMaterialGroup;

// 出力変数
layout(location=0) out vec4 outColor;
layout(location=1) out vec2 outTexcoord;
layout(location=2) out vec3 outNormal;
layout(location=3) out vec3 outPosition;
layout(location=4) out uint outTextureNo;

out gl_PerVertex {
  vec4 gl_Position;
};

// ユニフォーム変数
layout(location=0) uniform mat4 matTRS;
layout(location=1) uniform mat4 matModel;
layout(location=10) uniform vec4 materialColor[10];
layout(location=20) uniform uint materialTextureNo[10];
layout(location=30) uniform mat4 matGroupList[16];

// 頂点シェーダプログラム
void main()
{
  mat4 matGroup = matGroupList[vMaterialGroup.y];
  mat4 matGM = matModel * matGroup;

  // 回転行列を取り出す.
  mat3 matNormal = transpose(inverse(mat3(matGM)));

  // ワールド座標系の法線を計算.
  vec3 worldNormal = normalize(matNormal * vNormal);

  uint materialNo = vMaterialGroup.x;
  outColor = vColor * materialColor[materialNo];
  outTexcoord = vTexcoord;
  outNormal = worldNormal;
  outPosition = vec3(matGM * vec4(vPosition, 1.0));
  outTextureNo = materialTextureNo[materialNo];
  gl_Position = matTRS * (matGroup * vec4(vPosition, 1.0));
}

