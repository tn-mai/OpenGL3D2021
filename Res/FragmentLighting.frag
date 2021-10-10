#version 450

// 入力変数
layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexcoord;
layout(location=2) in vec3 inNormal;

// 出力変数
out vec4 fragColor;

// ユニフォーム変数
layout(binding=0) uniform sampler2D texColor;

// TODO: テキスト未追加
layout(location=100) uniform vec4 actorColor;

// 平行光源
struct DirectionalLight {
  vec3 direction; // ライトの向き
  vec3 color;     // ライトの色(明るさ)
};

#define MORNING 0
#define NOON    1
#define SUNSET  2
#define NIGHT   3
#define CLOUD   4
#define MOON    5

#define SKY_SCENE NOON

#if SKY_SCENE == MORNING
DirectionalLight light = {
  {-0.70,-0.59,-0.41},
  { 1.94, 1.65, 1.24},
};
// 環境光の色(明るさ)
vec3 ambientLight = { 0.15, 0.10, 0.20 };
#endif

#if SKY_SCENE == NOON
DirectionalLight light = {
  { 0.08,-0.82,-0.57},
  { 2.00, 1.88, 1.82},
};
vec3 ambientLight = { 0.10, 0.15, 0.20 };
#endif

#if SKY_SCENE == CLOUD
DirectionalLight light = {
  { 0.08,-0.82,-0.57},
  { 1.00, 0.84, 0.81},
};
vec3 ambientLight = { 0.20, 0.30, 0.40 };
#endif

#if SKY_SCENE == SUNSET
DirectionalLight light = {
  { 0.65,-0.63,-0.43},
  { 1.81, 1.16, 0.32},
};
vec3 ambientLight = { 0.15, 0.10, 0.20 };
#endif

#if SKY_SCENE == NIGHT
DirectionalLight light = {
  {-0.22,-0.80,-0.56},
  { 0.33, 0.55, 0.69},
};
vec3 ambientLight = { 0.40, 0.20, 0.30 };
#endif

#if SKY_SCENE == MOON
DirectionalLight light = {
  {-0.22,-0.80,-0.56},
  { 0.80, 0.94, 0.65},
};
vec3 ambientLight = { 0.25, 0.20, 0.30 };
#endif

// フラグメントシェーダプログラム
void main()
{
  vec4 tc = texture(texColor, inTexcoord);
  fragColor = inColor * tc * actorColor;

  // ワールド座標系の法線を正規化.
  vec3 worldNormal = normalize(inNormal);

  // 面が裏向きの場合、法線の向きを逆にする.
  if (gl_FrontFacing == false) {
    worldNormal *= -1;
  }

  // 環境光を設定.
  vec3 lightColor = ambientLight;

  // ランバート反射による明るさを計算.
  float cosTheta = max(dot(worldNormal, -light.direction), 0);
  lightColor += light.color * cosTheta;

  fragColor.rgb *= lightColor;
}

