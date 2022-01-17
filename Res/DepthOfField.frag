/**
* @file DepthOfField.frag
*/
#version 450

// 入力変数
layout(location=1) in vec2 inTexCoord;

// 出力変数
out vec4 fragColor;

// ユニフォーム変数
layout(binding=0) uniform sampler2D texColor0;
layout(binding=1) uniform sampler2D texColor1;
layout(binding=2) uniform sampler2D texDepth;

/**
* カメラの情報
*/
layout(location=102) uniform mat4 camera;
#define PIXEL_SIZE     (camera[0].xy) // NDC座標系における1ピクセルの幅と高さ
#define NEAR_PLANE     (camera[0][2]) // 近平面(m単位) 
#define FAR_PLANE      (camera[0][3]) // 遠平面(m単位)
#define FOCUS_DISTANCE (camera[1][0]) // レンズからピントの合う位置までの距離(mm単位)
#define FOCAL_LENGTH   (camera[1][1]) // 焦点距離(レンズから光が1点に集まる位置までの距離. mm単位)
#define APERTURE       (camera[1][2]) // 絞り(mm単位)
#define SENSOR_WIDTH   (camera[1][3]) // 光を受けるセンサーの横幅(mm単位)

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
* 錯乱円の半径を計算する.
*
* @param objectDistance 対象ピクセルのカメラ座標系におけるZ座標.
*
* @return 錯乱円の半径(ピクセル単位).
*
* @see https://docs.unrealengine.com/4.27/ja/RenderingAndGraphics/PostProcessEffects/DepthOfField/CinematicDOFMethods/
*/
float CircleOfConfusion(float objectDistance)
{
  // センサー上の錯乱円のサイズを計算する(mm単位)
  // - 物体が焦平面にある場合、CoCは0になる
  // - 焦平面が近いほどボケやすく、遠いほどボケにくい
  // - 焦点距離を長くするとボケやすい(現実にはFOVが変化してしまうのでボケのためだけに長くするわけにはいかない)
  // - 開口を小さくするほどボケが強くなる(現実には物理的な下限があるが、コンピューター上なら好きな値を指定できる)
  objectDistance *= 1000.0; // mm単位に変換し、値を正にする
  float cocSize = abs(APERTURE * (FOCAL_LENGTH * (FOCUS_DISTANCE - objectDistance)) /
    (objectDistance * (FOCUS_DISTANCE - FOCAL_LENGTH)));

  // 1mmに含まれるピクセル数を計算
  float pixelsPerMillimeter = 1.0 / (SENSOR_WIDTH * PIXEL_SIZE.x);

  // mm単位からピクセル数に変換
  return cocSize * pixelsPerMillimeter;
}

/**
* 深度バッファの深度値をビュー座標系の深度値に変換する
*
* @param w 深度バッファの深度値
*
* @return wをビュー座標系の深度値に変換した値
*/
float ToRealZ(float w)
{
  // 深度値をNDC座標系に戻す
  float n = 2.0 * w - 1.0; // 0～1を-1～+1に変換

  // パースペクティブ行列の逆の計算を行う
  // これによってビュー座標系に変換される
  return 2.0 * NEAR_PLANE * FAR_PLANE /
    (FAR_PLANE + NEAR_PLANE - n * (FAR_PLANE - NEAR_PLANE));
}

// フラグメントシェーダープログラム
void main()
{
  fragColor.rgb = texture(texColor0, inTexCoord).rgb;
  fragColor.a = 1.0;

  float z = ToRealZ(texture(texDepth, inTexCoord).r);
  float coc = CircleOfConfusion(z);
#if 1
  // 錯乱円の直径が1ピクセルを超える場合、ぼかし処理を行う
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