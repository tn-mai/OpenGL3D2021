#version 450

// ���͕ϐ�
layout(location=0) in vec3 vPosition;
layout(location=1) in vec4 vColor;
layout(location=2) in vec2 vTexcoord;
layout(location=3) in vec3 vNormal;
layout(location=4) in vec4 vWeights;
layout(location=5) in vec4 vJoints;

// �o�͕ϐ�
layout(location=0) out vec4 outColor;
layout(location=1) out vec2 outTexcoord;
layout(location=2) out vec3 outNormal;
layout(location=3) out vec3 outPosition;

out gl_PerVertex {
  vec4 gl_Position;
};

// ���j�t�H�[���ϐ�
layout(location=0) uniform mat4 matVP;
//layout(location=1) uniform mat4 matModel;
layout(location=10) uniform vec4 materialColor;

layout(std430, binding=0) buffer AnimationMatrices
{
  mat4 matBones[];
};

// ���_�V�F�[�_�v���O����
void main()
{
  outColor = vColor * materialColor;
  outTexcoord = vTexcoord;

  // �ŏ��̃E�F�C�g��0�ȊO�Ȃ�u�W���C���g�f�[�^����v�A
  // 0�̏ꍇ�́u�W���C���g�f�[�^�Ȃ��v�Ƃ݂Ȃ��B
  mat4 matModel;
  if (vWeights.x != 0) {
    matModel =
      matBones[int(vJoints.x)] * vWeights.x +
      matBones[int(vJoints.y)] * vWeights.y +
      matBones[int(vJoints.z)] * vWeights.z +
      matBones[int(vJoints.w)] * vWeights.w;

    // �E�F�C�g�����K������Ă��Ȃ��ꍇ�̑΍�([3][3]��1.0�ɂȂ�Ƃ͌���Ȃ�)
    matModel[3][3] = dot(vWeights, vec4(1));
  } else {
    matModel = matBones[0];
  }

  mat3 matNormal = transpose(inverse(mat3(matModel)));
  outNormal = normalize(matNormal * vNormal);

  outPosition = vec3(matModel * vec4(vPosition, 1.0));
  gl_Position = matVP * vec4(outPosition, 1.0);
}

