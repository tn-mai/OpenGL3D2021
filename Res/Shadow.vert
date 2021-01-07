#version 450 core

// 入力変数
layout(location=0) in vec3 vPosition;
layout(location=2) in vec2 vTexcoord;
layout(location=4) in vec3 vMorphPosition;
layout(location=6) in vec3 vPrevBaseMeshPosition;
layout(location=8) in vec3 vPrevMorphTargetPosition;

// 出力変数
layout(location=1) out vec2 outTexcoord;

out gl_PerVertex {
  vec4 gl_Position;
};

// ユニフォーム変数
layout(location=0) uniform mat4 matMVP;
layout(location=10) uniform vec3 morphWeight;

// 頂点シェーダプログラム
void main()
{
  // モーフィング
  vec3 curPosition = mix(vPosition, vMorphPosition, morphWeight.x);
  vec3 prevPosition = mix(vPrevBaseMeshPosition,
    vPrevMorphTargetPosition, morphWeight.y);
  vec3 position = mix(curPosition, prevPosition, morphWeight.z);

  outTexcoord = vTexcoord;
  gl_Position = matMVP * vec4(position, 1.0);
}
