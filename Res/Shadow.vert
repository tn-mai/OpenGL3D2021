#version 450 core

// ���͕ϐ�
layout(location=0) in vec3 vPosition;
layout(location=2) in vec2 vTexcoord;

// �o�͕ϐ�
layout(location=1) out vec2 outTexcoord;

out gl_PerVertex {
  vec4 gl_Position;
};

// ���j�t�H�[���ϐ�
layout(location=0) uniform mat4 matMVP;

// ���_�V�F�[�_�v���O����
void main()
{
  outTexcoord = vTexcoord;
  gl_Position = matMVP * vec4(vPosition, 1.0);
}