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
layout(location=4) out vec4 outMaterialParameters;
#else
layout(location=4) out uint outTextureNo;
#endif // ENABLE_SPECULAR

out gl_PerVertex {
  vec4 gl_Position;
};

// ���j�t�H�[���ϐ�
layout(location=0) uniform mat4 matVP;
layout(location=1) uniform mat4 matModel;
layout(location=10) uniform vec4 materialColor[10];
#ifdef ENABLE_SPECULAR
// x: �e�N�X�`���ԍ�
// y: ���t�l�X
// z: ���^���l�X(�����=0, ����=1)
layout(location=20) uniform vec4 materialParameters[10];
#else
layout(location=20) uniform uint materialTextureNo[10];
#endif // ENABLE_SPECULAR

struct InstanceData
{
  mat4 matModel;
  vec4 color;
};

// SSBO
layout(std430, binding=0) buffer InstanceDataBuffer
{
  InstanceData instances[];
};

// ���_�V�F�[�_�v���O����
void main()
{
  mat4 matInstanceModel = matModel * instances[gl_InstanceID].matModel;

  // ��]�s������o��.
  mat3 matNormal = transpose(inverse(mat3(matInstanceModel)));

  // ���[���h���W�n�̖@�����v�Z.
  vec3 worldNormal = normalize(matNormal * vNormal);

  uint materialNo = vMaterialGroup.x;
  outColor = vColor * materialColor[materialNo] * instances[gl_InstanceID].color;
  outTexcoord = vTexcoord;
  outNormal = worldNormal;
  outPosition = vec3(matInstanceModel * vec4(vPosition, 1.0));

#ifdef ENABLE_SPECULAR
  outMaterialParameters = materialParameters[materialNo];
#else
  outTextureNo = materialTextureNo[materialNo];
#endif // ENABLE_SPECULAR

  gl_Position = matVP * vec4(outPosition, 1.0);

}
