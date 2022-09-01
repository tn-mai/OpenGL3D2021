#version 450

// ���͕ϐ�
layout(location=1) in vec2 inTexcoord;

// �o�͕ϐ�
out vec4 fragColor;

// ���j�t�H�[���ϐ�
layout(binding=0) uniform sampler2D texColor;

const float threshold = 1.0; // ���邢�Ɣ��肷��臒l

// �t���O�����g�V�F�[�_�v���O����
void main()
{
  // �ǂݎ����W�����炷�������v�Z
  vec2 offset = 0.75 / vec2(textureSize(texColor, 0));

  // �ȈՂڂ����t�B���^(4x4�e�N�Z���̉��d���ς��v�Z)
  fragColor =  texture(texColor, inTexcoord - offset.xy);
  fragColor += texture(texColor, inTexcoord + offset.xy);
  fragColor += texture(texColor, inTexcoord - vec2(offset.x, -offset.y));
  fragColor += texture(texColor, inTexcoord + vec2(offset.x, -offset.y));
  fragColor *= 1.0 / 4.0; // ���ς����߂邽��1/4�ɂ���
  fragColor.a = 1;

  // RGB�̂��������Ƃ����邢�������s�N�Z���̖��邳�Ƃ���
  float brightness = max(fragColor.r, max(fragColor.g, fragColor.b));

  // ���邢�����̔䗦���v�Z
  float highBrightnessPart = max(brightness - threshold, 0);
  float ratio = highBrightnessPart / max(brightness, 0.00001);

  // ���邢�������v�Z
  fragColor.rgb *= ratio;
}

