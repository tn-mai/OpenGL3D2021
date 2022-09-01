#version 450

// ���͕ϐ�
layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexcoord;
layout(location=2) in vec3 inNormal;
layout(location=3) in vec3 inPosition;

// �o�͕ϐ�
out vec4 fragColor;

// ���j�t�H�[���ϐ�
layout(binding=0) uniform sampler2DArray texColor;
layout(binding=1) uniform sampler2D texShadow;
layout(binding=2) uniform sampler2D texMap;

layout(location=100) uniform mat4 matShadow;
layout(location=101) uniform vec4 mapSize; // �}�b�v�̍L��

// TODO: �e�L�X�g���ǉ�
layout(location=200) uniform vec4 actorColor;

// ���s����
struct DirectionalLight {
  vec3 direction; // ���C�g�̌���
  vec3 color;     // ���C�g�̐F(���邳)
};
layout(location=110) uniform DirectionalLight light;
// �����̐F(���邳)
layout(location=112) uniform vec3 ambientLight;

// �e���ڂ������߂̃T���v�����O���W.
const int sampleCount = 4;
const vec2 poissonDisk[sampleCount] = {
  { -0.942, -0.399 },
  {  0.946, -0.769 },
  { -0.094, -0.929 },
  {  0.345,  0.294 },
};

// �t���O�����g�V�F�[�_�v���O����
void main()
{
  const float tileSize = 4.0; // �}�X�ڂ̑傫��

  // �e�N�X�`���ԍ����擾
  vec2 texcoord = inPosition.xz / tileSize + mapSize.xy * 0.5;
  float tileNo = texelFetch(texMap, ivec2(texcoord), 0).r * 255.0;

  vec4 tc = texture(texColor, vec3(fract(texcoord), tileNo));
  fragColor = inColor * tc * actorColor;

  // ���[���h���W�n�̖@���𐳋K��.
  vec3 worldNormal = normalize(inNormal);

  // �ʂ��������̏ꍇ�A�@���̌������t�ɂ���.
  if (gl_FrontFacing == false) {
    worldNormal *= -1;
  }

  // �e
  float normalBias = 1.0; // �@�������Ɉړ�������s�N�Z����
  const vec2 shadowAreaSize = vec2(100.0, 100.0); // ���[���h���W�n�̉e�e�N�X�`���̑傫��
  vec2 shadowPixelSize = shadowAreaSize / textureSize(texShadow, 0); // 1�s�N�Z���̑傫��(���[���h���W)
  normalBias *= max(shadowPixelSize.x, shadowPixelSize.y);
  vec4 shadowPos = matShadow * vec4(inPosition + worldNormal * normalBias, 1.0);
  shadowPos.xyz *= (1.0 / shadowPos.w); // �p�[�X������

  // �e���ڂ���
  float shadow = 0.0;
  vec2 shadowRadius = vec2(0.1); // �ڂ������a(���[���h���W)
  shadowRadius /= shadowAreaSize; // �ڂ������a���e�N�X�`�����W�n�ɕϊ�
  for (int i = 0; i < sampleCount; ++i) {
    vec2 texcoord = shadowPos.xy + poissonDisk[i] * shadowRadius;
    shadow += float(texture(texShadow, texcoord).r < shadowPos.z);
  }
  shadow *= 1.0 / float(sampleCount);

  // ������ݒ�.
  vec3 lightColor = ambientLight;

  // �����o�[�g���˂ɂ�閾�邳���v�Z.
  float cosTheta = max(dot(worldNormal, -light.direction), 0);
  lightColor += light.color * cosTheta * (1.0 - shadow);

  fragColor.rgb *= lightColor;
}
