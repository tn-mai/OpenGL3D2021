#version 450

#define USE_REAL_Z 0

// 入力変数
layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexcoord;

// 出力変数
out vec4 fragColor;

// ユニフォーム変数
layout(binding=0) uniform sampler2D texColor;
layout(binding=1) uniform sampler2D texHatching;
layout(binding=2) uniform sampler2D texDepth;

// しきい値.
const float threshold = 0.25;

float GetZ(vec2 offset)
{
  float w = texture(texDepth, inTexcoord + offset).r;
#if USE_REAL_Z
  float near = 1;
  float far = 500;
  float n = 2 * w - 1; // ±1の範囲に変換.
  n = -2 * near * far / (far + near - n * (far - near));
  return -n / (far - near);
#else
  return w;
#endif
}

// フラグメントシェーダプログラム
void main()
{
  // カラーを取得.
  fragColor = inColor * texture(texColor, inTexcoord);
  vec2 screenSize = vec2(1280, 720);
  vec2 texPencilSize = textureSize(texHatching, 0);
  vec2 texcoord = fract(inTexcoord * (screenSize / texPencilSize));
  vec3 pencil = texture(texHatching, texcoord).rgb;

  // 最も明るい要素を全体の明るさとする.
  // YUVなどの輝度計算では青がほぼ死んでしまうため、影の判定には向いていない.
  float brightness = max(fragColor.r, max(fragColor.g, fragColor.b));

  // 明るさ0.5未満を「暗い領域」として、0.3までなめらかに合成比率を変化させる.
  float ratio = smoothstep(0.2, 0.4, brightness); // 合成比率.

  // 暗い領域は斜線テクスチャの色、明るい領域は白色になるように合成.
  pencil = mix(pencil, vec3(1), ratio);

  // 周囲8個のピクセルとの差分を計算(ラプラシアンフィルタ).
  vec2 pixelSize = vec2(1) / textureSize(texDepth, 0);
  float outline = 8 * texture(texDepth, inTexcoord).r;
  outline -= texture(texDepth, inTexcoord + pixelSize * vec2(-1, 1)).r;
  outline -= texture(texDepth, inTexcoord + pixelSize * vec2( 0, 1)).r;
  outline -= texture(texDepth, inTexcoord + pixelSize * vec2( 1, 1)).r;
  outline -= texture(texDepth, inTexcoord + pixelSize * vec2(-1, 0)).r;
  outline -= texture(texDepth, inTexcoord + pixelSize * vec2( 1, 0)).r;
  outline -= texture(texDepth, inTexcoord + pixelSize * vec2(-1,-1)).r;
  outline -= texture(texDepth, inTexcoord + pixelSize * vec2( 0,-1)).r;
  outline -= texture(texDepth, inTexcoord + pixelSize * vec2( 1,-1)).r;
  outline *= texture(texDepth, inTexcoord).r; // 奥行きによる変化量の差をなくす.
  outline = 1 - smoothstep(0.0, 0.01, abs(outline)); // 輪郭線ではない部分を1にする.

  // 斜線と輪郭の色を元の色に乗算.
  fragColor.rgb *= pencil * outline;
}
