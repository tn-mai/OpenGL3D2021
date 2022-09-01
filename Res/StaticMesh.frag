#version 450

// 入力変数
layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexcoord;
layout(location=2) in vec3 inNormal;
layout(location=3) in vec3 inPosition;

// 出力変数
out vec4 fragColor;

// ユニフォーム変数
layout(binding=0) uniform sampler2D texColor;
layout(binding=1) uniform sampler2D texShadow;

layout(location=100) uniform mat4 matShadow;
layout(location=101) uniform vec4 cameraPosition;

// 平行光源
struct DirectionalLight {
  vec3 direction; // ライトの向き
  vec3 color;     // ライトの色(明るさ)
};
layout(location=110) uniform DirectionalLight light;
// 環境光の色(明るさ)
layout(location=112) uniform vec3 ambientLight;

// 影をぼかすためのサンプリング座標.
#if 1
const int sampleCount = 4;
const vec2 poissonDisk[sampleCount] = {
  { -0.942, -0.399 },
  {  0.946, -0.769 },
  { -0.094, -0.929 },
  {  0.345,  0.294 },
};
#else
const int sampleCount = 16;
const vec2 poissonDisk[sampleCount] = {
  vec2( -0.94201624, -0.39906216 ), 
  vec2( 0.94558609, -0.76890725 ), 
  vec2( -0.094184101, -0.92938870 ), 
  vec2( 0.34495938, 0.29387760 ), 
  vec2( -0.91588581, 0.45771432 ), 
  vec2( -0.81544232, -0.87912464 ), 
  vec2( -0.38277543, 0.27676845 ), 
  vec2( 0.97484398, 0.75648379 ), 
  vec2( 0.44323325, -0.97511554 ), 
  vec2( 0.53742981, -0.47373420 ), 
  vec2( -0.26496911, -0.41893023 ), 
  vec2( 0.79197514, 0.19090188 ), 
  vec2( -0.24188840, 0.99706507 ), 
  vec2( -0.81409955, 0.91437590 ), 
  vec2( 0.19984126, 0.78641367 ), 
  vec2( 0.14383161, -0.14100790 ) 
};
#endif

// フラグメントシェーダプログラム
void main()
{
  fragColor = inColor * texture(texColor, inTexcoord);
//  // TODO: テキスト未実装
//  if (fragColor.a < 0.5) {
//    discard;
//  }

  // ワールド座標系の法線を正規化.
  vec3 worldNormal = normalize(inNormal);

  // 面が裏向きの場合、法線の向きを逆にする.
  if (gl_FrontFacing == false) {
    worldNormal *= -1;
  }

  // 影
  float normalBias = 1.0; // 法線方向に移動させるピクセル数
  const vec2 shadowAreaSize = vec2(100.0, 100.0); // ワールド座標系の影テクスチャの大きさ
  vec2 shadowPixelSize = shadowAreaSize / textureSize(texShadow, 0); // 1ピクセルの大きさ(ワールド座標)
  normalBias *= max(shadowPixelSize.x, shadowPixelSize.y);
  vec4 shadowPos = matShadow * vec4(inPosition + worldNormal * normalBias, 1.0);
  shadowPos.xyz *= (1.0 / shadowPos.w); // パースを解除

  // 影をぼかす
  float shadow = 0.0;
  vec2 shadowRadius = vec2(0.1); // ぼかし半径(ワールド座標)
  shadowRadius /= shadowAreaSize; // ぼかし半径をテクスチャ座標系に変換
  for (int i = 0; i < sampleCount; ++i) {
    vec2 texcoord = shadowPos.xy + poissonDisk[i] * shadowRadius;
    shadow += float(texture(texShadow, texcoord).r < shadowPos.z);
  }
  shadow *= 1.0 / float(sampleCount);

  // 環境光を設定.
  vec3 lightColor = ambientLight;

  // 鏡面反射を計算
  float specularExponent = 80;
  float normalizationFactor = (specularExponent + 8) / (3.14159265 * 8);
  vec3 V = normalize(cameraPosition.xyz - inPosition);
  vec3 H = normalize(V + worldNormal);
  float dotNH = max(dot(worldNormal, H), 0);

  // Schlick近似式によってフレネル項Fを計算
  float dotVH = max(dot(V, H), 0);
  float F0 = 0.04;
  float F = F0 + (1 - F0) * pow(1 - dotVH, 5);

  // ランバート反射による明るさを計算.
  float cosTheta = max(dot(worldNormal, -light.direction), 0);
  lightColor += light.color * cosTheta * (1.0 - shadow) * (1.0 - F);
  fragColor.rgb *= lightColor;

  vec3 specularColor = light.color * normalizationFactor * pow(dotNH, specularExponent) * cosTheta * (1.0 - shadow);
  fragColor.rgb += specularColor * F;
}

