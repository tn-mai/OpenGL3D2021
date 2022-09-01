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
  // �e�N�X�`�����W�n�ɂ�����0.5�s�N�Z���̃T�C�Y
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

