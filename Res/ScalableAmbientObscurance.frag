#version 450

layout(location=1) in vec2 inTexcoord;

out vec4 outColor;

layout(binding=0) uniform sampler2D texDepth;

// x: �T���v�����O���a
// y: �������e�̊g�嗦
// z: AO�̋��x
// w: (���g�p)
layout(location=100) uniform vec4 radiusScaleIntensity;

// �t�v���W�F�N�V�����s��
layout(location=101) uniform mat4 matInvProj;

// x: 1.0 / �X�N���[���̕�
// y: 1.0 / �X�N���[���̍���
// z: near�v���[���܂ł̋���
// w: far�v���[���܂ł̋���
layout(location=102) uniform vec4 camera;

#define NUM_SAMPLES (11) // �T���v����
#define NUM_SPIRAL_TURNS (7) // �T���v���_�̉�]��
#define PI (3.14159265) // �~����
#define TWO_PI (PI * 2.0) // 2��(=360�x)

/**
* �[�x�o�b�t�@�̒l����r���[��Ԃ�Z�l�𕜌�
*
* https://stackoverflow.com/questions/6652253/getting-the-true-z-value-from-the-depth-buffer
*/
float GetDepthVS(vec2 uv)
{
  float near = camera.z;
  float far = camera.w;
  float depth = texture(texDepth, uv).x * 2.0 - 1.0;
  return 2.0 * near * far / (near + far - depth * (far - near));
}

/**
* �r���[���W�n�̍��W���v�Z
*/
vec3 GetPositionVS(vec2 uv, float depth)
{
  // �t�@�[���ʏ�̃r���[���W�����߂�
  vec4 tmp = matInvProj * vec4(uv * 2.0 - 1.0, -1.0, 1.0);
  vec3 farPlaneVS = tmp.xyz / tmp.w;

  // ���_����t�@�[���ʂ֌������x�N�g�������߂�
  vec3 ray = normalize(farPlaneVS);

  // �s�N�Z���̃r���[���W�����߂�
  return ray * depth;
}

/**
* SAO(Scalable Ambient Obscurance)�ɂ��Օ��������߂�
*
* https://casual-effects.com/research/McGuire2012SAO/index.html
* https://gist.github.com/transitive-bullshit/6770311
*/
void main()
{
  // �s�N�Z���̃r���[���W�Ɩ@��
  float depth = GetDepthVS(inTexcoord);
  vec3 positionVS = GetPositionVS(inTexcoord, depth);
  vec3 normalVS = normalize(cross(dFdx(positionVS), dFdy(positionVS)));

  // ���[���h���W�n�ƃX�N���[�����W�n�̃T���v�����O���a
  float radiusWS = radiusScaleIntensity.x;
  float radiusSS = radiusWS * radiusScaleIntensity.y / depth;

  // �t���O�����g���Ƃɉ�]�̊J�n�p�x�����炷���ƂŌ����ڂ����P����
  // Hash function used in the HPG12 AlchemyAO paper
  // Intel GPU�ł͎O�p�֐��ɋ���Ȑ���n���ƕs���m�Ȓl��Ԃ��B���̂��߁A���̂܂܂ł̓y�[�p�[�ɏ�����Ă�����@�͎g���Ȃ��B
  // �������@�́A�Ⴆ�Έȉ���URL�ɂ���悤��mod�ōő�l�𐧌�����B�������m�C�Y�e�N�X�`������擾������@������B
  // https://bugs.freedesktop.org/show_bug.cgi?id=80018
  ivec2 iuv = ivec2(gl_FragCoord.xy);
  float startAngle = (3 * iuv.x ^ iuv.y + iuv.x * iuv.y) % 257;

  vec2 pixelSize = camera.xy;
  float r2 = radiusWS * radiusWS;
  float occlusion = 0;
  for (int i = 0; i < NUM_SAMPLES; ++i) {
    // �T���v���_�̊p�x�Ƌ��������߂�
    float ratio = (float(i) + 0.5) * (1.0 / float(NUM_SAMPLES));
    float angle = ratio * float(NUM_SPIRAL_TURNS) * TWO_PI + startAngle;
    vec2 unitOffset = vec2(cos(angle), sin(angle)); 
    ratio *= radiusSS;

    // �T���v���_�̃r���[���W�����߂�
    vec2 uv = inTexcoord + ratio * unitOffset * pixelSize;
    vec3 samplePositionVS = GetPositionVS(uv, GetDepthVS(uv));

    // �T���v���_�ւ̃x�N�g���Ɩ@���̃R�T�C�������߂�
    const float bias = 0.01;
    vec3 v = samplePositionVS - positionVS;
    float vv = dot(v, v);
    float vn = max(0.0, dot(v, normalVS) - bias);

    // �T���v���_�܂ł̋����ƃR�T�C������AO�����߂�
    float f = max(r2 - vv, 0.0);
    occlusion += f * f * f * vn / (vv + 0.01);
  }

  // ���ϒl�����߁AAO�̋�������Z����
  float intensity = radiusScaleIntensity.z;
  //occlusion = max(0.0, 1.0 - occlusion * intensity * (1.0 / NUM_SAMPLES));
  occlusion = min(1.0, occlusion * intensity * (1.0 / NUM_SAMPLES));

  // Bilateral box-filter over a quad for free, respecting depth edges
  // (the difference that this makes is subtle)
  // �k���o�b�t�@�ƃu���[���g��Ȃ��Ƃ���܂�Ӗ����Ȃ������Ɍ�����
#if 0
  if (abs(dFdx(positionVS.z)) < 0.02) {
    occlusion -= dFdx(occlusion) * ((iuv.x & 1) - 0.5);
  }
  if (abs(dFdy(positionVS.z)) < 0.02) {
    occlusion -= dFdy(occlusion) * ((iuv.y & 1) - 0.5);
  }
#endif

  outColor = vec4(occlusion);
}

