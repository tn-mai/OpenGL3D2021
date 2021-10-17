#version 450

// ���͕ϐ�
layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexcoord;

// �o�͕ϐ�
out vec4 fragColor;

// ���j�t�H�[���ϐ�
layout(binding=0) uniform sampler2D texColor;

// TODO: �e�L�X�g���ǉ�
layout(location=200) uniform vec4 modelColor;

// �t���O�����g�V�F�[�_�[�v���O����
void main() {
  vec4 tc = texture(texColor, inTexcoord);
  fragColor = inColor * tc * modelColor;
}

