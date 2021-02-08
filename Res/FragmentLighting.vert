#version 450 core

// 入力変数
layout(location=0) in vec3 vPosition;
layout(location=1) in vec4 vColor;
layout(location=2) in vec2 vTexcoord;
layout(location=3) in vec3 vNormal;

layout(location=4) in vec3 vMorphPosition;
layout(location=5) in vec3 vMorphNormal;

layout(location=6) in vec3 vPrevBaseMeshPosition;
layout(location=7) in vec3 vPrevBaseMeshNormal;
layout(location=8) in vec3 vPrevMorphTargetPosition;
layout(location=9) in vec3 vPrevMorphTargetNormal;

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
layout(location=8) uniform vec4 objectColor;
layout(location=10) uniform vec3 morphWeight;

// 頂点シェーダプログラム
void main()
{
  // モーフィング
  vec3 curPosition = mix(vPosition, vMorphPosition, morphWeight.x);
  vec3 prevPosition = mix(vPrevBaseMeshPosition,
    vPrevMorphTargetPosition, morphWeight.y);
  vec3 position = mix(curPosition, prevPosition, morphWeight.z);

  vec3 curNormal = mix(vNormal, vMorphNormal, morphWeight.x);
  vec3 prevNormal = mix(vPrevBaseMeshNormal,
    vPrevMorphTargetNormal, morphWeight.y);
  vec3 normal = normalize(mix(curNormal, prevNormal, morphWeight.z));

  outColor = vColor * objectColor;
  outTexcoord = vTexcoord;
  outPosition = vec3(matModel * vec4(position, 1));
  mat3 matNormal = transpose(inverse(mat3(matModel)));
  outNormal = matNormal * normal;

  gl_Position = matMVP * vec4(position, 1.0);
}

