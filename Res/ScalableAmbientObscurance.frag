#version 450

layout(location=1) in vec2 inTexcoord;

out vec4 outColor;

layout(binding=0) uniform sampler2D texDepth;

// x: サンプリング半径
// y: 透視投影の拡大率
// z: AOの強度
// w: (未使用)
layout(location=100) uniform vec4 radiusScaleIntensity;

// 逆プロジェクション行列
layout(location=101) uniform mat4 matInvProj;

// x: 1.0 / スクリーンの幅
// y: 1.0 / スクリーンの高さ
// z: nearプレーンまでの距離
// w: farプレーンまでの距離
layout(location=102) uniform vec4 camera;

#define NUM_SAMPLES (11) // サンプル数
#define NUM_SPIRAL_TURNS (7) // サンプル点の回転回数
#define PI (3.14159265) // 円周率
#define TWO_PI (PI * 2.0) // 2π(=360度)

/**
* 深度バッファの値からビュー空間のZ値を復元
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
* ビュー座標系の座標を計算
*/
vec3 GetPositionVS(vec2 uv, float depth)
{
  // ファー平面上のビュー座標を求める
  vec4 tmp = matInvProj * vec4(uv * 2.0 - 1.0, -1.0, 1.0);
  vec3 farPlaneVS = tmp.xyz / tmp.w;

  // 視点からファー平面へ向かうベクトルを求める
  vec3 ray = normalize(farPlaneVS);

  // ピクセルのビュー座標を求める
  return ray * depth;
}

/**
* SAO(Scalable Ambient Obscurance)により遮蔽項を求める
*
* https://casual-effects.com/research/McGuire2012SAO/index.html
* https://gist.github.com/transitive-bullshit/6770311
*/
void main()
{
  // ピクセルのビュー座標と法線
  float depth = GetDepthVS(inTexcoord);
  vec3 positionVS = GetPositionVS(inTexcoord, depth);
  vec3 normalVS = normalize(cross(dFdx(positionVS), dFdy(positionVS)));

  // ワールド座標系とスクリーン座標系のサンプリング半径
  float radiusWS = radiusScaleIntensity.x;
  float radiusSS = radiusWS * radiusScaleIntensity.y / depth;

  // フラグメントごとに回転の開始角度をずらすことで見た目を改善する
  // Hash function used in the HPG12 AlchemyAO paper
  // Intel GPUでは三角関数に巨大な数を渡すと不正確な値を返す。そのため、そのままではペーパーに書かれている方法は使えない。
  // 解決方法は、例えば以下のURLにあるようにmodで最大値を制限する。乱数をノイズテクスチャから取得する方法もある。
  // https://bugs.freedesktop.org/show_bug.cgi?id=80018
  ivec2 iuv = ivec2(gl_FragCoord.xy);
  float startAngle = (3 * iuv.x ^ iuv.y + iuv.x * iuv.y) % 257;

  vec2 pixelSize = camera.xy;
  float r2 = radiusWS * radiusWS;
  float occlusion = 0;
  for (int i = 0; i < NUM_SAMPLES; ++i) {
    // サンプル点の角度と距離を求める
    float ratio = (float(i) + 0.5) * (1.0 / float(NUM_SAMPLES));
    float angle = ratio * float(NUM_SPIRAL_TURNS) * TWO_PI + startAngle;
    vec2 unitOffset = vec2(cos(angle), sin(angle)); 
    ratio *= radiusSS;

    // サンプル点のビュー座標を求める
    vec2 uv = inTexcoord + ratio * unitOffset * pixelSize;
    vec3 samplePositionVS = GetPositionVS(uv, GetDepthVS(uv));

    // サンプル点へのベクトルと法線のコサインを求める
    const float bias = 0.01;
    vec3 v = samplePositionVS - positionVS;
    float vv = dot(v, v);
    float vn = max(0.0, dot(v, normalVS) - bias);

    // サンプル点までの距離とコサインからAOを求める
    float f = max(r2 - vv, 0.0);
    occlusion += f * f * f * vn / (vv + 0.01);
  }

  // 平均値を求め、AOの強さを乗算する
  float intensity = radiusScaleIntensity.z;
  //occlusion = max(0.0, 1.0 - occlusion * intensity * (1.0 / NUM_SAMPLES));
  occlusion = min(1.0, occlusion * intensity * (1.0 / NUM_SAMPLES));

  // Bilateral box-filter over a quad for free, respecting depth edges
  // (the difference that this makes is subtle)
  // 縮小バッファとブラーを使わないとあんまり意味がなさそうに見える
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

