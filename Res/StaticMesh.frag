#version 450

// ���͕ϐ�
layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexcoord;
layout(location=2) in vec3 inNormal;
layout(location=3) in vec3 inPosition;

// �o�͕ϐ�
out vec4 fragColor;

// ���j�t�H�[���ϐ�
layout(binding=0) uniform sampler2D texColor;
layout(binding=1) uniform sampler2D texShadow;

layout(location=100) uniform mat4 matShadow;
layout(location=101) uniform vec4 cameraPosition;

// ���s����
struct DirectionalLight {
  vec3 direction; // ���C�g�̌���
  vec3 color;     // ���C�g�̐F(���邳)
};
layout(location=110) uniform DirectionalLight light;
// �����̐F(���邳)
layout(location=112) uniform vec3 ambientLight;

// �e���ڂ������߂̃T���v�����O���W.
#if 1
const int sampleCount = 4;
const vec2 poissonDisk[sampleCount] = {
  { -0.942, -0.399 },
  {  0.946, -0.769 },
  { -0.094, -0.929 },
  {  0.345,  0.294 },
};
#else
const int sampleCount = 16;
const vec2 poissonDisk[sampleCount] = {
  vec2( -0.94201624, -0.39906216 ), 
  vec2( 0.94558609, -0.76890725 ), 
  vec2( -0.094184101, -0.92938870 ), 
  vec2( 0.34495938, 0.29387760 ), 
  vec2( -0.91588581, 0.45771432 ), 
  vec2( -0.81544232, -0.87912464 ), 
  vec2( -0.38277543, 0.27676845 ), 
  vec2( 0.97484398, 0.75648379 ), 
  vec2( 0.44323325, -0.97511554 ), 
  vec2( 0.53742981, -0.47373420 ), 
  vec2( -0.26496911, -0.41893023 ), 
  vec2( 0.79197514, 0.19090188 ), 
  vec2( -0.24188840, 0.99706507 ), 
  vec2( -0.81409955, 0.91437590 ), 
  vec2( 0.19984126, 0.78641367 ), 
  vec2( 0.14383161, -0.14100790 ) 
};
#endif

// �t���O�����g�V�F�[�_�v���O����
void main()
{
  fragColor = inColor * texture(texColor, inTexcoord);
//  // TODO: �e�L�X�g������
//  if (fragColor.a < 0.5) {
//    discard;
//  }

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

  // ���ʔ��˂��v�Z
  float specularExponent = 80;
  float normalizationFactor = (specularExponent + 8) / (3.14159265 * 8);
  vec3 V = normalize(cameraPosition.xyz - inPosition);
  vec3 H = normalize(V + worldNormal);
  float dotNH = max(dot(worldNormal, H), 0);

  // Schlick�ߎ����ɂ���ăt���l����F���v�Z
  float dotVH = max(dot(V, H), 0);
  float F0 = 0.04;
  float F = F0 + (1 - F0) * pow(1 - dotVH, 5);

  // �����o�[�g���˂ɂ�閾�邳���v�Z.
  float cosTheta = max(dot(worldNormal, -light.direction), 0);
  lightColor += light.color * cosTheta * (1.0 - shadow) * (1.0 - F);
  fragColor.rgb *= lightColor;

  vec3 specularColor = light.color * normalizationFactor * pow(dotNH, specularExponent) * cosTheta * (1.0 - shadow);
  fragColor.rgb += specularColor * F;
}

