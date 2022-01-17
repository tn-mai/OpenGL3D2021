/**
* @file DepthOfField.frag
*/
#version 450

// ���͕ϐ�
layout(location=1) in vec2 inTexCoord;

// �o�͕ϐ�
out vec4 fragColor;

// ���j�t�H�[���ϐ�
layout(binding=0) uniform sampler2D texColor0;
layout(binding=1) uniform sampler2D texColor1;
layout(binding=2) uniform sampler2D texDepth;

/**
* �J�����̏��
*/
layout(location=102) uniform mat4 camera;
#define PIXEL_SIZE     (camera[0].xy) // NDC���W�n�ɂ�����1�s�N�Z���̕��ƍ���
#define NEAR_PLANE     (camera[0][2]) // �ߕ���(m�P��) 
#define FAR_PLANE      (camera[0][3]) // ������(m�P��)
#define FOCUS_DISTANCE (camera[1][0]) // �����Y����s���g�̍����ʒu�܂ł̋���(mm�P��)
#define FOCAL_LENGTH   (camera[1][1]) // �œ_����(�����Y�������1�_�ɏW�܂�ʒu�܂ł̋���. mm�P��)
#define APERTURE       (camera[1][2]) // �i��(mm�P��)
#define SENSOR_WIDTH   (camera[1][3]) // �����󂯂�Z���T�[�̉���(mm�P��)

// @see https://www.adriancourreges.com/blog/2018/12/02/ue4-optimized-post-effects/
const int bokehSampleCount = 16;
const vec2 bokehDisk[bokehSampleCount] = {
  {-0.1160260,-0.433013 },
  { 0.1160250,-0.433013 },
  { 0.3169880,-0.316987 },
  { 0.4330130,-0.116025 },
  {-0.3169870,-0.316988 },
  {-0.0386758,-0.144338 },
  { 0.1443380,-0.0386744 },
  { 0.4330130, 0.116026 },
  {-0.4330130,-0.116025 },
  {-0.1443370, 0.0386758 },
  { 0.0386744, 0.144338 },
  { 0.3169870, 0.316987 },
  {-0.4330120, 0.116026 },
  {-0.3169870, 0.316988 },
  {-0.1160260, 0.433013 },
  { 0.1160250, 0.433013 },
};

/**
* �����~�̔��a���v�Z����.
*
* @param objectDistance �Ώۃs�N�Z���̃J�������W�n�ɂ�����Z���W.
*
* @return �����~�̔��a(�s�N�Z���P��).
*
* @see https://docs.unrealengine.com/4.27/ja/RenderingAndGraphics/PostProcessEffects/DepthOfField/CinematicDOFMethods/
*/
float CircleOfConfusion(float objectDistance)
{
  // �Z���T�[��̍����~�̃T�C�Y���v�Z����(mm�P��)
  // - ���̂��ŕ��ʂɂ���ꍇ�ACoC��0�ɂȂ�
  // - �ŕ��ʂ��߂��قǃ{�P�₷���A�����قǃ{�P�ɂ���
  // - �œ_�����𒷂�����ƃ{�P�₷��(�����ɂ�FOV���ω����Ă��܂��̂Ń{�P�̂��߂����ɒ�������킯�ɂ͂����Ȃ�)
  // - �J��������������قǃ{�P�������Ȃ�(�����ɂ͕����I�ȉ��������邪�A�R���s���[�^�[��Ȃ�D���Ȓl���w��ł���)
  objectDistance *= 1000.0; // mm�P�ʂɕϊ����A�l�𐳂ɂ���
  float cocSize = abs(APERTURE * (FOCAL_LENGTH * (FOCUS_DISTANCE - objectDistance)) /
    (objectDistance * (FOCUS_DISTANCE - FOCAL_LENGTH)));

  // 1mm�Ɋ܂܂��s�N�Z�������v�Z
  float pixelsPerMillimeter = 1.0 / (SENSOR_WIDTH * PIXEL_SIZE.x);

  // mm�P�ʂ���s�N�Z�����ɕϊ�
  return cocSize * pixelsPerMillimeter;
}

/**
* �[�x�o�b�t�@�̐[�x�l���r���[���W�n�̐[�x�l�ɕϊ�����
*
* @param w �[�x�o�b�t�@�̐[�x�l
*
* @return w���r���[���W�n�̐[�x�l�ɕϊ������l
*/
float ToRealZ(float w)
{
  // �[�x�l��NDC���W�n�ɖ߂�
  float n = 2.0 * w - 1.0; // 0�`1��-1�`+1�ɕϊ�

  // �p�[�X�y�N�e�B�u�s��̋t�̌v�Z���s��
  // ����ɂ���ăr���[���W�n�ɕϊ������
  return 2.0 * NEAR_PLANE * FAR_PLANE /
    (FAR_PLANE + NEAR_PLANE - n * (FAR_PLANE - NEAR_PLANE));
}

// �t���O�����g�V�F�[�_�[�v���O����
void main()
{
  fragColor.rgb = texture(texColor0, inTexCoord).rgb;
  fragColor.a = 1.0;

  float z = ToRealZ(texture(texDepth, inTexCoord).r);
  float coc = CircleOfConfusion(z);
#if 1
  // �����~�̒��a��1�s�N�Z���𒴂���ꍇ�A�ڂ����������s��
  if (coc > 1.0) {
    vec2 scale = PIXEL_SIZE * coc;
    vec3 bokehColor = vec3(0);
    for (int i = 0; i < bokehSampleCount; ++i) {
      vec2 uv = inTexCoord + bokehDisk[i] * scale;
      bokehColor += texture(texColor1, uv).rgb;
    }
    bokehColor *= 1.0 / bokehSampleCount;
    fragColor.rgb = mix(fragColor.rgb, bokehColor, clamp(coc - 1.0, 0.0, 1.0)); 
  }
  //fragColor.rgb = vec3(coc * 0.1);
#else
  fragColor.rgb = texture(texColor0, inTexCoord).rgb;
  float focalPlane = FOCUS_DISTANCE * 0.001;
  float count = 1.0;
  for (int i = 0; i < bokehSampleCount; ++i) {
    vec2 uv = inTexCoord + coc * bokehDisk[i];
    float z = ToRealZ(texture(texDepth, uv).r);
    if (focalPlane <= z) {
      float coc2 = CircleOfConfusion(z);
      if (coc >= coc2) {
        fragColor.rgb += texture(texColor1, uv).rgb;
        count += 1.0;
      }
    }
  }
  fragColor.rgb /= count;
#endif
}