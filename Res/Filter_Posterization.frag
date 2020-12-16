#version 450

// 入力変数
layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexcoord;

// 出力変数
out vec4 fragColor;

// ユニフォーム変数
layout(binding=0) uniform sampler2D texColor;

// 階調数.
const float levels = 5;

// フラグメントシェーダプログラム
void main()
{
  // カラーを取得.
  fragColor = inColor * texture(texColor, inTexcoord);

  // 最も明るい要素を全体の明るさとする.
  // YUVなどの輝度計算では青がほぼ死んでしまうためポスター化に向いていない.
  float brightness = max(fragColor.r, max(fragColor.g, fragColor.b));

  // level段階で量子化された明るさを計算.
  float quantizedBrightness = floor(brightness * levels + 0.5) / levels;

  // 量子化された明るさと実際の明るさの比をカラーに掛けることで、実際のカラーを量子化する.
  fragColor.rgb *= quantizedBrightness / brightness;
}

