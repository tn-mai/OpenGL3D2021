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
  // テクスチャ座標系における0.5ピクセルのサイズ
  vec2 offset = 0.5 / vec2(textureSize(texColor, 0));

  fragColor =  texture(texColor, inTexcoord + vec2(-offset.x * 2, 0));
  fragColor += texture(texColor, inTexcoord + vec2(-offset.x,     offset.y    )) * 2;
  fragColor += texture(texColor, inTexcoord + vec2(0,             offset.y * 2));
  fragColor += texture(texColor, inTexcoord + vec2(offset.x,      offset.y    )) * 2;
  fragColor += texture(texColor, inTexcoord + vec2(offset.x * 2,  0));
  fragColor += texture(texColor, inTexcoord + vec2(offset.x,     -offset.y    )) * 2;
  fragColor += texture(texColor, inTexcoord + vec2(0,            -offset.y * 2));
  fragColor += texture(texColor, inTexcoord + vec2(-offset.x,    -offset.y    )) * 2;
  fragColor *= 1.0 / 12.0;
  fragColor.a = 1;
}

