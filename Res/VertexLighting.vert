#version 450

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
layout(location=0) uniform mat4 matTRS;
layout(location=1) uniform mat4 matModel;

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

// 頂点シェーダープログラム
void main() {
  // 回転行列を取り出す.
  mat3 matNormal = transpose(inverse(mat3(matModel)));

  // ワールド座標系の法線を計算.
  vec3 worldNormal = normalize(matNormal * vNormal);

  // 環境光を設定.
  vec3 lightColor = ambientLight;

  // ランバート反射による明るさを計算.
  float cosTheta = max(dot(worldNormal, -light.direction), 0);
  lightColor += light.color * cosTheta;
  outColor.rgb = vColor.rgb * lightColor;

  // 不透明度は物体の値をそのまま使う.
  outColor.a = vColor.a;

  outTexcoord = vTexcoord;
  gl_Position = matTRS * vec4(vPosition, 1.0);
}
