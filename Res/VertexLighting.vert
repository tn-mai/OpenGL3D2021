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

// 平行光源
struct DirectionalLight {
  vec4 direction;
  vec4 color;
};
layout(location=2) uniform DirectionalLight directionalLight;

// 点光源
struct PointLight {
  vec4 position;
  vec4 color;
};
layout(location=4) uniform PointLight pointLight;

// 頂点シェーダプログラム
void main()
{
  vec3 worldPosition = vec3(matModel * vec4(vPosition, 1));
  mat3 matNormal = transpose(inverse(mat3(matModel)));
  vec3 worldNormal = matNormal * vNormal;

  vec3 totalLightColor = vec3(0);

  // 平行光源
  {
    float theta = max(dot(worldNormal, directionalLight.direction.xyz), 0);
    totalLightColor += directionalLight.color.rgb * theta;
  }

  // 点光源
  {
    // フラグメントからライトへ向かうベクトルを計算.
    vec3 lightVector = pointLight.position.xyz - worldPosition;

    // 距離による明るさの変化量を計算.
    float lengthSq = dot(lightVector, lightVector);
    float intensity = 1.0 / (1.0 + lengthSq);

    // 面の傾きによる明るさの変化量を計算.
    float theta = 1;
    if (lengthSq > 0) {
      vec3 direction = normalize(lightVector);
      theta = max(dot(worldNormal, direction), 0);
    }

    // 変化量をかけ合わせて明るさを求め、ライトの明るさ変数に加算.
    totalLightColor += pointLight.color.rgb * theta * intensity;
  }

  outColor = vec4(vColor.rgb * totalLightColor, vColor.a);

  outTexcoord = vTexcoord;
  gl_Position = matMVP * vec4(vPosition, 1.0);
}
