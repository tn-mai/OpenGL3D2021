#version 450

#define ENABLE_SPECULAR

// ���͕ϐ�
layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexcoord;
layout(location=2) in vec3 inNormal;
layout(location=3) in vec3 inPosition;
#ifdef ENABLE_SPECULAR
// x: �e�N�X�`���ԍ�
// y: ���t�l�X
// z: ���^���l�X(�����=0, ����=1)
layout(location=4) in vec4 inMaterialParameters;
#else
layout(location=4) in flat uint inTextureNo;
#endif // ENABLE_SPECULAR

// �o�͕ϐ�
out vec4 fragColor;

// ���j�t�H�[���ϐ�
layout(binding=0) uniform sampler2D texColor0;
layout(binding=1) uniform sampler2D texShadow;
layout(binding=2) uniform sampler2D texColor1_7[7];
layout(binding=9) uniform sampler2D texNormal0_7[8];

layout(location=100) uniform mat4 matShadow;
layout(location=101) uniform vec4 cameraPosition;

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

/**
* �@�����v�Z����
*
* http://hacksoflife.blogspot.com/2009/11/per-pixel-tangent-space-normal-mapping.html
* http://www.thetenthplanet.de/archives/1180
*/
vec3 ComputeWorldNormal(vec3 V)
{
  vec3 normal = vec3(0, 0, 1);
  switch (uint(inMaterialParameters.w)) {
  case 0:  normal = texture(texNormal0_7[0], inTexcoord).rgb; break;
  case 1:  normal = texture(texNormal0_7[1], inTexcoord).rgb; break;
  case 2:  normal = texture(texNormal0_7[2], inTexcoord).rgb; break;
  case 3:  normal = texture(texNormal0_7[3], inTexcoord).rgb; break;
  case 4:  normal = texture(texNormal0_7[4], inTexcoord).rgb; break;
  case 5:  normal = texture(texNormal0_7[5], inTexcoord).rgb; break;
  case 6:  normal = texture(texNormal0_7[6], inTexcoord).rgb; break;
  case 7:  normal = texture(texNormal0_7[7], inTexcoord).rgb; break;
  }

  // �l������������ꍇ�A�@���e�N�X�`�����ݒ肳��Ă��Ȃ��Ƃ݂Ȃ��Ē��_�@����Ԃ�
  vec3 N = normalize(inNormal);
  if (dot(normal, normal) <= 0.0001) {
    return N;
  }

  // 8bit�l�ł��邱�Ƃ��l������0�`1��-1�`+1�ɕϊ�(127��0�Ƃ݂Ȃ�)
  // 0-255 - 128 / 127
  normal = normal * (255.0 / 127.0) - (128.0 / 127.0);

  // �אڃs�N�Z���Ԃ̃x�N�g�����擾
  vec3 dp1 = dFdx(-V);
  vec3 dp2 = dFdy(-V);
  vec2 duv1 = dFdx(inTexcoord);
  vec2 duv2 = dFdy(inTexcoord);

  // �^���W�F���g��Ԃ��烏�[���h��Ԃɕϊ�����t�s����v�Z
  vec3 dp1perp = cross(N, dp1);
  vec3 dp2perp = cross(dp2, N);
  vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
  vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;
  float invmax = inversesqrt(max(dot(T, T), dot(B, B)));
  mat3 TBN = mat3(T * invmax, B * invmax, N);

  // �@���e�N�X�`���̒l�����[���h��Ԃɕϊ�
  return normalize(TBN * normal);
}

// �t���O�����g�V�F�[�_�v���O����
void main()
{
  vec4 tc = vec4(1.0, 1.0, 1.0, 1.0);
#ifdef ENABLE_SPECULAR
  switch (uint(inMaterialParameters.x)) {
#else
  switch (inTextureNo) {
#endif // ENABLE_SPECULAR
  case 0:  tc = texture(texColor0, inTexcoord); break;
  case 1:  tc = texture(texColor1_7[0], inTexcoord); break;
  case 2:  tc = texture(texColor1_7[1], inTexcoord); break;
  case 3:  tc = texture(texColor1_7[2], inTexcoord); break;
  case 4:  tc = texture(texColor1_7[3], inTexcoord); break;
  case 5:  tc = texture(texColor1_7[4], inTexcoord); break;
  case 6:  tc = texture(texColor1_7[5], inTexcoord); break;
  case 7:  tc = texture(texColor1_7[6], inTexcoord); break;
  }
  fragColor = inColor * tc * actorColor;

  // TODO: �e�L�X�g������
  if (fragColor.a < 0.5) {
    discard;
  }

  // ���[���h���W�n�̖@���𐳋K��
  vec3 V = normalize(cameraPosition.xyz - inPosition);
  vec3 worldNormal = ComputeWorldNormal(V);

  // �ʂ��������̏ꍇ�A�@���̌������t�ɂ���.
  if (gl_FrontFacing == false) {
    worldNormal *= -1;
  }

  // �e
#if 0
  vec4 shadowPos = matShadow * vec4(inPosition, 1.0);
#else
  float normalBias = 1.0; // �@�������Ɉړ�������s�N�Z����
  const vec2 shadowAreaSize = vec2(100.0, 100.0); // ���[���h���W�n�̉e�e�N�X�`���̑傫��
  vec2 shadowPixelSize = shadowAreaSize / textureSize(texShadow, 0); // 1�s�N�Z���̑傫��(���[���h���W)
  normalBias *= max(shadowPixelSize.x, shadowPixelSize.y);
  vec4 shadowPos = matShadow * vec4(inPosition + worldNormal * normalBias, 1.0);
#endif
  shadowPos.xyz *= (1.0 / shadowPos.w); // �p�[�X������

#if 0
  float shadow = float(texture(texShadow, shadowPos.xy).r < shadowPos.z);
#else
  // �e���ڂ���
  float shadow = 0.0;
  vec2 shadowRadius = vec2(0.1); // �ڂ������a(���[���h���W)
  shadowRadius /= shadowAreaSize; // �ڂ������a���e�N�X�`�����W�n�ɕϊ�
  for (int i = 0; i < sampleCount; ++i) {
    vec2 texcoord = shadowPos.xy + poissonDisk[i] * shadowRadius;
    shadow += float(texture(texShadow, texcoord).r < shadowPos.z);
  }
  shadow *= 1.0 / float(sampleCount);
#endif

  // ������ݒ�.
  vec3 lightColor = ambientLight;

#ifdef ENABLE_SPECULAR
  // Cook-Torrance���f���̃p�����[�^
  float roughness = inMaterialParameters.y;
  float metalness = inMaterialParameters.z;
  float alpha = roughness * roughness;
  //vec3 V = normalize(cameraPosition.xyz - inPosition); // �����x�N�g��
  vec3 H = normalize(V + -light.direction); // �����ƌ��̌����̃n�[�t�x�N�g��

  // �����ʕ��z�֐�(Blinn-Phong NDF)�ɂ�閾�邳���v�Z
  float p = 2 / (alpha * alpha) - 2;
  float C = (p + 2) / (2 * 3.14159265);
  float dotNH = max(dot(worldNormal, H), 0);
  float D = C * pow(dotNH, p);

  // Schlick-Beckman GSF�ɂ���Ċ􉽌�����D���v�Z
  // k = roughness^2 * sqrt(2.0 / 3.14159265)
  float k = alpha * 0.79788456;
  float dotNL = max(dot(worldNormal, -light.direction), 0);
  float dotNV = max(dot(worldNormal, V), 0);
  float G1_l = dotNL / (dotNL * (1 - k) + k);
  float G1_v = dotNV / (dotNV * (1 - k) + k);
  float G = min(1, G1_l * G1_v);

  // Schlick�ߎ����ɂ���ăt���l����F���v�Z
  float dotVH = max(dot(V, H), 0);
  vec3 F0 = mix(vec3(0.04), fragColor.rgb, metalness);
  vec3 F = F0 + (1 - F0) * pow(1 - dotVH, 5);

  // �N�b�N�g�����X���f���ɂ�鋾�ʔ��˂��v�Z
  vec3 specularColor = (D * F * G) / (4 * dotNL * dotNV + 0.00001);

  // �����o�[�g���˂ɂ�閾�邳���v�Z
  float cosTheta = max(dot(worldNormal, -light.direction), 0);
  if (metalness == 0) {
    lightColor += light.color * cosTheta * (1.0 - shadow) * (1 - F);
  }

  fragColor.rgb *= lightColor;
  fragColor.rgb += specularColor * (1.0 - shadow);
#else
  // �����o�[�g���˂ɂ�閾�邳���v�Z
  float cosTheta = max(dot(worldNormal, -light.direction), 0);
  lightColor += light.color * cosTheta * (1.0 - shadow);

  fragColor.rgb *= lightColor;

  // ���ʔ��˂��v�Z
  float specularExponent = 80;
  float normalizationFactor = (specularExponent + 8) / (3.14159265 * 8);
  vec3 V = normalize(cameraPosition.xyz - inPosition);
  vec3 H = normalize(V + worldNormal);
  float dotNH = max(dot(worldNormal, H), 0);
  vec3 specularColor = light.color * normalizationFactor * pow(dotNH, specularExponent) * cosTheta * (1.0 - shadow);
  fragColor.rgb += specularColor * 0.1;
#endif // ENABLE_SPECULAR
}

