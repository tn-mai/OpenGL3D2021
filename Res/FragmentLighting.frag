#version 450

#define ENABLE_SPECULAR

// 入力変数
layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexcoord;
layout(location=2) in vec3 inNormal;
layout(location=3) in vec3 inPosition;
#ifdef ENABLE_SPECULAR
// x: テクスチャ番号
// y: ラフネス
// z: メタルネス(非金属=0, 金属=1)
layout(location=4) in vec4 inMaterialParameters;
#else
layout(location=4) in flat uint inTextureNo;
#endif // ENABLE_SPECULAR

// 出力変数
out vec4 fragColor;

// ユニフォーム変数
layout(binding=0) uniform sampler2D texColor0;
layout(binding=1) uniform sampler2D texShadow;
layout(binding=2) uniform sampler2D texColor1_7[7];
layout(binding=9) uniform sampler2D texNormal0_7[8];

layout(location=100) uniform mat4 matShadow;
layout(location=101) uniform vec4 cameraPosition;

// TODO: テキスト未追加
layout(location=200) uniform vec4 actorColor;

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

/**
* 法線を計算する
*
* http://hacksoflife.blogspot.com/2009/11/per-pixel-tangent-space-normal-mapping.html
* http://www.thetenthplanet.de/archives/1180
*/
vec3 ComputeWorldNormal(vec3 V)
{
  vec3 normal = vec3(0, 0, 1);
  switch (uint(inMaterialParameters.w)) {
  case 0:  normal = texture(texNormal0_7[0], inTexcoord).rgb; break;
  case 1:  normal = texture(texNormal0_7[1], inTexcoord).rgb; break;
  case 2:  normal = texture(texNormal0_7[2], inTexcoord).rgb; break;
  case 3:  normal = texture(texNormal0_7[3], inTexcoord).rgb; break;
  case 4:  normal = texture(texNormal0_7[4], inTexcoord).rgb; break;
  case 5:  normal = texture(texNormal0_7[5], inTexcoord).rgb; break;
  case 6:  normal = texture(texNormal0_7[6], inTexcoord).rgb; break;
  case 7:  normal = texture(texNormal0_7[7], inTexcoord).rgb; break;
  }

  // 値が小さすぎる場合、法線テクスチャが設定されていないとみなして頂点法線を返す
  vec3 N = normalize(inNormal);
  if (dot(normal, normal) <= 0.0001) {
    return N;
  }

  // 8bit値であることを考慮しつつ0～1を-1～+1に変換(127を0とみなす)
  // 0-255 - 128 / 127
  normal = normal * (255.0 / 127.0) - (128.0 / 127.0);

  // 隣接ピクセル間のベクトルを取得
  vec3 dp1 = dFdx(-V);
  vec3 dp2 = dFdy(-V);
  vec2 duv1 = dFdx(inTexcoord);
  vec2 duv2 = dFdy(inTexcoord);

  // タンジェント空間からワールド空間に変換する逆行列を計算
  vec3 dp1perp = cross(N, dp1);
  vec3 dp2perp = cross(dp2, N);
  vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
  vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;
  float invmax = inversesqrt(max(dot(T, T), dot(B, B)));
  mat3 TBN = mat3(T * invmax, B * invmax, N);

  // 法線テクスチャの値をワールド空間に変換
  return normalize(TBN * normal);
}

// フラグメントシェーダプログラム
void main()
{
  vec4 tc = vec4(1.0, 1.0, 1.0, 1.0);
#ifdef ENABLE_SPECULAR
  switch (uint(inMaterialParameters.x)) {
#else
  switch (inTextureNo) {
#endif // ENABLE_SPECULAR
  case 0:  tc = texture(texColor0, inTexcoord); break;
  case 1:  tc = texture(texColor1_7[0], inTexcoord); break;
  case 2:  tc = texture(texColor1_7[1], inTexcoord); break;
  case 3:  tc = texture(texColor1_7[2], inTexcoord); break;
  case 4:  tc = texture(texColor1_7[3], inTexcoord); break;
  case 5:  tc = texture(texColor1_7[4], inTexcoord); break;
  case 6:  tc = texture(texColor1_7[5], inTexcoord); break;
  case 7:  tc = texture(texColor1_7[6], inTexcoord); break;
  }
  fragColor = inColor * tc * actorColor;

  // TODO: テキスト未実装
  if (fragColor.a < 0.5) {
    discard;
  }

  // ワールド座標系の法線を正規化
  vec3 V = normalize(cameraPosition.xyz - inPosition);
  vec3 worldNormal = ComputeWorldNormal(V);

  // 面が裏向きの場合、法線の向きを逆にする.
  if (gl_FrontFacing == false) {
    worldNormal *= -1;
  }

  // 影
#if 0
  vec4 shadowPos = matShadow * vec4(inPosition, 1.0);
#else
  float normalBias = 1.0; // 法線方向に移動させるピクセル数
  const vec2 shadowAreaSize = vec2(100.0, 100.0); // ワールド座標系の影テクスチャの大きさ
  vec2 shadowPixelSize = shadowAreaSize / textureSize(texShadow, 0); // 1ピクセルの大きさ(ワールド座標)
  normalBias *= max(shadowPixelSize.x, shadowPixelSize.y);
  vec4 shadowPos = matShadow * vec4(inPosition + worldNormal * normalBias, 1.0);
#endif
  shadowPos.xyz *= (1.0 / shadowPos.w); // パースを解除

#if 0
  float shadow = float(texture(texShadow, shadowPos.xy).r < shadowPos.z);
#else
  // 影をぼかす
  float shadow = 0.0;
  vec2 shadowRadius = vec2(0.1); // ぼかし半径(ワールド座標)
  shadowRadius /= shadowAreaSize; // ぼかし半径をテクスチャ座標系に変換
  for (int i = 0; i < sampleCount; ++i) {
    vec2 texcoord = shadowPos.xy + poissonDisk[i] * shadowRadius;
    shadow += float(texture(texShadow, texcoord).r < shadowPos.z);
  }
  shadow *= 1.0 / float(sampleCount);
#endif

  // 環境光を設定.
  vec3 lightColor = ambientLight;

#ifdef ENABLE_SPECULAR
  // Cook-Torranceモデルのパラメータ
  float roughness = inMaterialParameters.y;
  float metalness = inMaterialParameters.z;
  float alpha = roughness * roughness;
  //vec3 V = normalize(cameraPosition.xyz - inPosition); // 視線ベクトル
  vec3 H = normalize(V + -light.direction); // 視線と光の向きのハーフベクトル

  // 微小面分布関数(Blinn-Phong NDF)による明るさを計算
  float p = 2 / (alpha * alpha) - 2;
  float C = (p + 2) / (2 * 3.14159265);
  float dotNH = max(dot(worldNormal, H), 0);
  float D = C * pow(dotNH, p);

  // Schlick-Beckman GSFによって幾何減衰項Dを計算
  // k = roughness^2 * sqrt(2.0 / 3.14159265)
  float k = alpha * 0.79788456;
  float dotNL = max(dot(worldNormal, -light.direction), 0);
  float dotNV = max(dot(worldNormal, V), 0);
  float G1_l = dotNL / (dotNL * (1 - k) + k);
  float G1_v = dotNV / (dotNV * (1 - k) + k);
  float G = min(1, G1_l * G1_v);

  // Schlick近似式によってフレネル項Fを計算
  float dotVH = max(dot(V, H), 0);
  vec3 F0 = mix(vec3(0.04), fragColor.rgb, metalness);
  vec3 F = F0 + (1 - F0) * pow(1 - dotVH, 5);

  // クックトランスモデルによる鏡面反射を計算
  vec3 specularColor = (D * F * G) / (4 * dotNL * dotNV + 0.00001);

  // ランバート反射による明るさを計算
  float cosTheta = max(dot(worldNormal, -light.direction), 0);
  if (metalness == 0) {
    lightColor += light.color * cosTheta * (1.0 - shadow) * (1 - F);
  }

  fragColor.rgb *= lightColor;
  fragColor.rgb += specularColor * (1.0 - shadow);
#else
  // ランバート反射による明るさを計算
  float cosTheta = max(dot(worldNormal, -light.direction), 0);
  lightColor += light.color * cosTheta * (1.0 - shadow);

  fragColor.rgb *= lightColor;

  // 鏡面反射を計算
  float specularExponent = 80;
  float normalizationFactor = (specularExponent + 8) / (3.14159265 * 8);
  vec3 V = normalize(cameraPosition.xyz - inPosition);
  vec3 H = normalize(V + worldNormal);
  float dotNH = max(dot(worldNormal, H), 0);
  vec3 specularColor = light.color * normalizationFactor * pow(dotNH, specularExponent) * cosTheta * (1.0 - shadow);
  fragColor.rgb += specularColor * 0.1;
#endif // ENABLE_SPECULAR
}

