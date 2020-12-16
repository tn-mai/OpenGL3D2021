#version 450

// 入力変数
layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexcoord;

// 出力変数
out vec4 fragColor;

// ユニフォーム変数
layout(binding=0) uniform sampler2D texColor;

// ぼかしサイズ.
const int size = 5;

// 重み配列.
const float kernel[size][size] = { 
  { 0.003765, 0.015019, 0.023792, 0.015019, 0.003765 },
  { 0.015019, 0.059912, 0.094907, 0.059912, 0.015019 },
  { 0.023792, 0.094907, 0.150342, 0.094907, 0.023792 },
  { 0.015019, 0.059912, 0.094907, 0.059912, 0.015019 },
  { 0.003765, 0.015019, 0.023792, 0.015019, 0.003765 },
};

// フラグメントシェーダプログラム
void main()
{
  vec2 unitSize = vec2(1) / vec2(textureSize(texColor, 0));

  vec2 texcoord;
  vec3 c = vec3(0);
  texcoord.y = inTexcoord.y - unitSize.y * float(size);
  for (int y = 0; y < size; ++y) {
    texcoord.x = inTexcoord.x - unitSize.x * float(size);
    for (int x = 0; x < size; ++x) {
      c += texture(texColor, texcoord).rgb * kernel[y][x];
      texcoord.x += unitSize.x;
    }
    texcoord.y += unitSize.y;
  }

  fragColor = inColor * vec4(c, 1);
}

