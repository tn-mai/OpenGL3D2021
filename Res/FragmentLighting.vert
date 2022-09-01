#version 450

#define ENABLE_SPECULAR

// ���͕ϐ�
layout(location=0) in vec3 vPosition;
layout(location=1) in vec4 vColor;
layout(location=2) in vec2 vTexcoord;
layout(location=3) in vec3 vNormal;
layout(location=4) in uvec2 vMaterialGroup;

// �o�͕ϐ�
layout(location=0) out vec4 outColor;
layout(location=1) out vec2 outTexcoord;
layout(location=2) out vec3 outNormal;
layout(location=3) out vec3 outPosition;
#ifdef ENABLE_SPECULAR
// x: �e�N�X�`���ԍ�
// y: ���t�l�X
// z: ���^���l�X(�����=0, ����=1)
// w: �@���e�N�X�`���ԍ�
layout(location=4) out vec4 outMaterialParameters;
#else
layout(location=4) out uint outTextureNo;
#endif // ENABLE_SPECULAR

out gl_PerVertex {
  vec4 gl_Position;
};

// ���j�t�H�[���ϐ�
layout(location=0) uniform mat4 matTRS;
layout(location=1) uniform mat4 matModel;
layout(location=10) uniform vec4 materialColor[10];
#ifdef ENABLE_SPECULAR
// x: �e�N�X�`���ԍ�
// y: ���t�l�X
// z: ���^���l�X(�����=0, ����=1)
// w: �@���e�N�X�`���ԍ�
layout(location=20) uniform vec4 materialParameters[10];
#else
layout(location=20) uniform uint materialTextureNo[10];
#endif // ENABLE_SPECULAR

layout(location=30) uniform mat4 matGroupModels[32];

// ���_�V�F�[�_�v���O����
void main()
{
  mat4 matGroup = matGroupModels[vMaterialGroup.y];
  mat4 matGM = matModel * matGroup;

  // ��]�s������o��.
  mat3 matNormal = transpose(inverse(mat3(matGM)));

  // ���[���h���W�n�̖@�����v�Z.
  vec3 worldNormal = normalize(matNormal * vNormal);

  uint materialNo = vMaterialGroup.x;
  outColor = vColor * materialColor[materialNo];
  outTexcoord = vTexcoord;
  outNormal = worldNormal;
  outPosition = vec3(matGM * vec4(vPosition, 1.0));
#ifdef ENABLE_SPECULAR
  outMaterialParameters = materialParameters[materialNo];
#else
  outTextureNo = materialTextureNo[materialNo];
#endif // ENABLE_SPECULAR
  gl_Position = matTRS * (matGroup * vec4(vPosition, 1.0));
}

