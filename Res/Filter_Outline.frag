#version 450

// 入力変数
layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexcoord;

// 出力変数
out vec4 fragColor;

// ユニフォーム変数
layout(binding=0) uniform sampler2D texColor;
layout(binding=1) uniform sampler2D texDepth;

float GetZ(vec2 offset)
{
  float w = texture(texDepth, inTexcoord + offset).r;
  //return w;
  float near = 0.1;
  float far = 500;
  float n = w;//2 * w - 1; // ±1の範囲に変換.
  n = -2 * near * far / (far + near - w * (far - near));
  return n / (far - near);
}

// フラグメントシェーダプログラム
void main()
{
  // 8近傍ラプラシアンフィルタ
  vec2 unitSize = vec2(1) / vec2(textureSize(texDepth, 0));
  float c0 = GetZ(vec2(0));
  float c = 8 * c0;
  c -= GetZ(vec2(-1, 1) * unitSize);
  c -= GetZ(vec2( 0, 1) * unitSize);
  c -= GetZ(vec2( 1, 1) * unitSize);
  c -= GetZ(vec2(-1, 0) * unitSize);
  c -= GetZ(vec2( 1, 0) * unitSize);
  c -= GetZ(vec2(-1,-1) * unitSize);
  c -= GetZ(vec2( 0,-1) * unitSize);
  c -= GetZ(vec2( 1,-1) * unitSize);
  c = 1 - smoothstep(0.0, 0.1, abs(c) / -c0);

  // カラーを取得.
  fragColor = inColor * texture(texColor, inTexcoord);
  fragColor.rgb *= vec3(c);
}
