#version 450

// 入力変数
layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexcoord;
layout(location=2) in vec3 inPosition;
layout(location=3) in vec3 inNormal;

// 出力変数
out vec4 fragColor;

// ユニフォーム変数
layout(binding=0) uniform sampler2D texColor;

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

// フラグメントシェーダプログラム
void main()
{
  vec3 worldNormal = normalize(inNormal);
  vec3 totalLightColor = vec3(0);

  // 平行光源
  {
    float theta = max(dot(worldNormal, directionalLight.direction.xyz), 0);
    totalLightColor += directionalLight.color.rgb * theta;
  }

  // 点光源
  {
    // フラグメントからライトへ向かうベクトルを計算.
    vec3 lightVector = pointLight.position.xyz - inPosition;

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

  fragColor = inColor * texture(texColor, inTexcoord);
  fragColor.rgb *= totalLightColor;
}

