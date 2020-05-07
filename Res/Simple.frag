#version 450 core

// 入力変数
layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexcoord;

// 出力変数
out vec4 fragColor;

// ユニフォーム変数
layout(binding=0) uniform sampler2D texColor;

// フラグメントシェーダプログラム
void main()
{
  fragColor = inColor * texture(texColor, inTexcoord);
}
