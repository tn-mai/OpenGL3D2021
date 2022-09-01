#version 450

// 入力変数
layout(location=1) in vec2 inTexcoord;

// 出力変数
out vec4 fragColor;

// ユニフォーム変数
layout(binding=0) uniform sampler2D texColor;

const float threshold = 1.0; // 明るいと判定する閾値

// フラグメントシェーダプログラム
void main()
{
  // 読み取り座標をずらす距離を計算
  vec2 offset = 0.75 / vec2(textureSize(texColor, 0));

  // 簡易ぼかしフィルタ(4x4テクセルの加重平均を計算)
  fragColor =  texture(texColor, inTexcoord - offset.xy);
  fragColor += texture(texColor, inTexcoord + offset.xy);
  fragColor += texture(texColor, inTexcoord - vec2(offset.x, -offset.y));
  fragColor += texture(texColor, inTexcoord + vec2(offset.x, -offset.y));
  fragColor *= 1.0 / 4.0; // 平均を求めるため1/4にする
  fragColor.a = 1;

  // RGBのうちもっとも明るい成分をピクセルの明るさとする
  float brightness = max(fragColor.r, max(fragColor.g, fragColor.b));

  // 明るい成分の比率を計算
  float highBrightnessPart = max(brightness - threshold, 0);
  float ratio = highBrightnessPart / max(brightness, 0.00001);

  // 明るい成分を計算
  fragColor.rgb *= ratio;
}

