/**
* @file DepthOfField.vert
*/
#version 450

// ���͕ϐ�
layout(location=0) in vec3 vPosition;
layout(location=2) in vec2 vTexCoord;

// �o�͕ϐ�
layout(location=1) out vec2 outTexCoord;
out gl_PerVertex {
  vec4 gl_Position;
};

// ���j�t�H�[���ϐ�
layout(location=0) uniform mat4 matMVP;

// ���_�V�F�[�_�[�v���O����
void main()
{
  outTexCoord = vTexCoord;
  gl_Position = matMVP * vec4(vPosition, 1.0);
}
