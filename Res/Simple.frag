#version 450

// 入力変数
layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexcoord;

// 出力変数
out vec4 fragColor;

// ユニフォーム変数
layout(binding=0) uniform sampler2D texColor;

// フラグメントシェーダープログラム
void main() {
  vec4 tc = texture(texColor, inTexcoord);
  fragColor = inColor * tc;
}

