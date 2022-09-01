#version 450

// ���͕ϐ�
layout(location=1) in vec2 inTexcoord;

// �o�͕ϐ�
out vec4 fragColor;

// ���j�t�H�[���ϐ�
layout(binding=0) uniform sampler2D texColor;

// �t���O�����g�V�F�[�_�[�v���O����
void main()
{
  // �e�N�X�`�����W�n�ɂ�����0.75�s�N�Z���̃T�C�Y
  vec2 offset = 0.75 / vec2(textureSize(texColor, 0));

  fragColor =  texture(texColor, inTexcoord - offset.xy);
  fragColor += texture(texColor, inTexcoord + offset.xy);
  fragColor += texture(texColor, inTexcoord - vec2(offset.x, -offset.y));
  fragColor += texture(texColor, inTexcoord + vec2(offset.x, -offset.y));
  fragColor *= 1.0 / 4.0;
  fragColor.a = 1;
}

