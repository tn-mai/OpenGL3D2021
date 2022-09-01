#version 450

// 入力変数
layout(location=1) in vec2 inTexcoord;

// 出力変数
out vec4 fragColor;

// ユニフォーム変数
layout(binding=0) uniform sampler2D texColor;

// フラグメントシェーダープログラム
void main()
{
  // テクスチャ座標系における0.75ピクセルのサイズ
  vec2 offset = 0.75 / vec2(textureSize(texColor, 0));

  fragColor =  texture(texColor, inTexcoord - offset.xy);
  fragColor += texture(texColor, inTexcoord + offset.xy);
  fragColor += texture(texColor, inTexcoord - vec2(offset.x, -offset.y));
  fragColor += texture(texColor, inTexcoord + vec2(offset.x, -offset.y));
  fragColor *= 1.0 / 4.0;
  fragColor.a = 1;
}

