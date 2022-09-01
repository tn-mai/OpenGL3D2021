#version 450

// 入力変数
layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexcoord;
layout(location=2) in vec3 inNormal;
layout(location=3) in vec3 inPosition;

// 出力変数
out vec4 fragColor;

// ユニフォーム変数
layout(binding=0) uniform sampler2DArray texColor;
layout(binding=1) uniform sampler2D texShadow;
layout(binding=2) uniform sampler2D texMap;

layout(location=100) uniform mat4 matShadow;
layout(location=101) uniform vec4 mapSize; // マップの広さ

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
const int sampleCount = 4;
const vec2 poissonDisk[sampleCount] = {
  { -0.942, -0.399 },
  {  0.946, -0.769 },
  { -0.094, -0.929 },
  {  0.345,  0.294 },
};

// フラグメントシェーダプログラム
void main()
{
  const float tileSize = 4.0; // マス目の大きさ

  // テクスチャ番号を取得
  vec2 texcoord = inPosition.xz / tileSize + mapSize.xy * 0.5;
  float tileNo = texelFetch(texMap, ivec2(texcoord), 0).r * 255.0;

  vec4 tc = texture(texColor, vec3(fract(texcoord), tileNo));
  fragColor = inColor * tc * actorColor;

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

  // ランバート反射による明るさを計算.
  float cosTheta = max(dot(worldNormal, -light.direction), 0);
  lightColor += light.color * cosTheta * (1.0 - shadow);

  fragColor.rgb *= lightColor;
}
