#version 450

// 入力変数
layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexcoord;

// 出力変数
out vec4 fragColor;

// ユニフォーム変数
layout(binding=0) uniform sampler2D texColor;
layout(binding=1) uniform sampler2D texHatching;

// しきい値.
const float threshold = 0.25;

// フラグメントシェーダプログラム
void main()
{
  // カラーを取得.
  fragColor = inColor * texture(texColor, inTexcoord);
  vec3 hatching = texture(texHatching, gl_FragCoord.xy * (1.0 / 64)).rgb;

  // 最も明るい要素を全体の明るさとする.
  // YUVなどの輝度計算では青がほぼ死んでしまうため、影の判定には向いていない.
  float brightness = max(fragColor.r, max(fragColor.g, fragColor.b));

  // 
  fragColor.rgb *= mix(hatching, vec3(1), smoothstep(0, threshold, brightness));
}
