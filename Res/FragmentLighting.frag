#version 450

// 入力変数
layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexcoord;
layout(location=2) in vec3 inPosition;
layout(location=3) in vec3 inNormal;

// 出力変数
out vec4 fragColor;

// ユニフォーム変数
layout(binding=0) uniform sampler2D texColor;
layout(binding=1) uniform sampler2D texNormal;
layout(binding=2) uniform sampler2D texMetallicSmoothness;

// 平行光源
struct DirectionalLight {
  vec4 direction;
  vec4 color;
};
layout(location=2) uniform DirectionalLight directionalLight;

// 点光源
struct PointLight {
  vec4 position;
  vec4 color;
};
layout(location=4) uniform PointLight pointLight;

// 環境光
layout(location=6) uniform vec3 ambientLight;

// 視点座標
layout(location=7) uniform vec3 viewPosition;

const uvec2 screenSize = uvec2(1280, 720); // 画面の大きさ.
const uvec2 tileCount = uvec2(8, 4); // 視錐台の分割数.
const vec2 tileSize =
  vec2(screenSize) / vec2(tileCount); // 分割区画の大きさ.
const uint maxLightCountInTile = 64; // 区画に含まれるライトの最大数.
const uint maxLightCount = 1024; // シーン全体で使えるライトの最大数.

// ライトの種類.
const float Type_PointLight = 0;
const float Type_SpotLight = 1;

const float pi = 3.14159265358979323846264338327950288;

/**
* ライト.
*/
struct Light {
  vec4 positionAndType;     // ライトの座標と種類.
  vec4 colorAndRange;       // ライトの色(明るさ)と、光の届く範囲.
  vec4 direction;           // ライトが照らす方向.
  vec4 coneAndFalloffAngle; // スポットライトが照らす角度と減衰開始角度.
};

/**
* 描画に関係するライトの情報.
*/
layout(std430, binding=0) readonly restrict buffer TileData 
{
  uint lightIndices[tileCount.y][tileCount.x][maxLightCountInTile];
  uint lightCounts[tileCount.y][tileCount.x];
  Light lights[maxLightCount];
};

/**
* 法線を計算する.
*
* http://hacksoflife.blogspot.com/2009/11/per-pixel-tangent-space-normal-mapping.html
* http://www.thetenthplanet.de/archives/1180
*/
vec3 computeNormal(vec3 V)
{
  vec3 N = normalize(inNormal);
  vec3 normal = texture(texNormal, inTexcoord).rgb;
  if (dot(normal, normal) <= 0.01) {
    return N;
  }

  vec3 dp1 = dFdx(V);
  vec3 dp2 = dFdy(V);
  vec2 duv1 = dFdx(inTexcoord);
  vec2 duv2 = dFdy(inTexcoord);

  vec3 dp2perp = cross(dp2, N);
  vec3 dp1perp = cross(N, dp1);
  vec3 T = -dp2perp * duv1.x + dp1perp * duv2.x;
  vec3 B = dp2perp * duv1.y - dp1perp * duv2.y;

  // the transpose of texture-to-eye space matrix
  //float invmax = inversesqrt(max(dot(T, T), dot(B, B)));
  //mat3 TBN = mat3(T * invmax, B * invmax, N);
  T = normalize(cross(B, N));
  mat3 TBN = mat3(normalize(T), normalize(B), N);

  // transform the normal to eye space 
  normal = normal * 2 - 1;
  return TBN * normal;
}

/**
* フレネル反射率を計算する.
*
* @param F0  角度が0度のときのフレネル反射率.
* @param L   光の入射方向.
* @param V   視線ベクトル.
*/
float Fresnel(float F0, vec3 L, vec3 V)
{
  // 8.656170 -> https://seblagarde.wordpress.com/2011/08/17/hello-world/
  // ハーフベクトルの定義から、dot(direction, H)とdot(V, H)は同じ結果になる.
  // 視点から見たマイクロファセット法線の平均はハーフベクトルと等しい.
  // dot(V, N)でなくdot(V, H)を使うことでマイクロファセットを考慮したフレネル項が得られる.
  vec3 H = normalize(L + V);
  return F0 + (1 - F0) * exp2(-8.656170 * max(dot(V, H), 0));
}

/**
* スペキュラを計算する(Blinn-Phong).
*/
float computeSpecular(vec3 direction, vec3 worldNormal, vec3 V, float shininess, float normalizationFactor)
{
  vec3 H = normalize(direction + V);

  // Blinn-Phong BRDFの正規化係数についての議論は以下のサイトを参照.
  // http://www.rorydriscoll.com/2009/01/25/energy-conservation-in-games/
  // 上記サイトのコメントによると、正規化係数"(n+8)/8π"は、真の正規化係数"(n+2)(n+4)/8π(2^(-n/2)+n)"の現実的な近似であるらしい.
  // 正規化係数の導出はこれ -> http://www.farbrausch.de/~fg/stuff/phong.pdf
  // なお"(n+2)/2π"はPhong BRDFの正規化係数なのだが、多くのサイトが誤ってBlinn-Phongに適用しているようだ。
  //
  // Tri-Aceによる同様の導出では正規化係数を"(n+2)/4π(2-2^(-n/2)"とし、近似を"(n+2.04)/8π"としている.
  // http://research.tri-ace.com/Data/BasicOfReflectance.ppt
  // これはBlinn-Phongの正規化係数であり、Blinn-Phong BRDFの正規化係数ではない点に注意.
  // PhongもBlinn-PhongもBRDFかどうかで正規化係数が異なるので注意.
  // BRDFバージョンはPhongまたはBlinn-Phong計算のあと、さらにdot(N, L)を掛ける.
  // このコードではこの関数の外で行っている.
#if 1
  // Normalized Blinn-Phong BRDF
  // http://www0.cs.ucl.ac.uk/staff/j.kautz/GameCourse/04_PointLights.pdf
  return pow(max(dot(worldNormal, H), 0), shininess) * normalizationFactor;
#else
  // Velvet Assassinエンジンで採用された、フレネル項を"dot(L,H)^-3"で近似する方法.
  // F0が存在しないため、光沢度をそのままスペキュラ係数にしてしまうとやたら光沢感が出る.
  // http://www.thetenthplanet.de/archives/255
  float LdotH = pow(max(dot(direction, H), 0), 3);
  return pow(max(dot(worldNormal, H), 0), shininess) * (shininess + 1) / (8.0 * pi * LdotH);
#endif
}

// フラグメントシェーダプログラム
void main()
{
  float metallic = texture(texMetallicSmoothness, inTexcoord).r; // 0=非金属 1=金属
  float smoothness = texture(texMetallicSmoothness, inTexcoord).g;
  float shininess = exp2(smoothness * 10 + 3);
  //smoothness *= 0.5; // VAエンジンの式を使う場合の係数. 数値は適当にそれっぽくなるように選んだ.

  // 正規化係数を計算.
  float normalizationFactor = (shininess + 8) * (1.0 / (8.0 * pi));

  // 視線と法線が一致(角度が0)する場合のフレネル係数.
  // 非金属(誘電体)のF0は2%～6%程度に収まる. ただし、宝石類は10～20%に達する場合がある.
  // 金属(導体)のF0は50%～100%程度でRGBごとに違っている. 例えば金は(1.0, 0.71, 0.29)など.
  const float ior = 1.5;
  float f0 = mix(0.04, 1.0, metallic);//pow((ior - 1) / (ior + 1), 2);
  // 光沢度をF0に転用するバージョン.
  //f0 = smoothness;

  vec3 viewVector = normalize(viewPosition - inPosition);
  vec3 worldNormal = computeNormal(viewVector);
  vec3 totalLightColor = ambientLight;
  vec3 totalSpecularColor = vec3(0);

  // 平行光源
  {
    float F = Fresnel(f0, -directionalLight.direction.xyz, viewVector);

    float theta = max(dot(worldNormal, -directionalLight.direction.xyz), 0);
    totalLightColor += directionalLight.color.rgb * theta * (1 - F);

    totalSpecularColor += directionalLight.color.rgb * theta *
      computeSpecular(-directionalLight.direction.xyz, worldNormal, viewVector, shininess, normalizationFactor) * F;
  }

  // 点光源
  uvec2 tileId = uvec2(gl_FragCoord.xy * (vec2(1) / tileSize));
  const uint lightCount = lightCounts[tileId.y][tileId.x];
  for (uint i = 0; i < lightCount; ++i) {
    uint lightIndex = lightIndices[tileId.y][tileId.x][i];
    if (lightIndex >= maxLightCount) {
      continue;
    }

    // フラグメントからライトへ向かうベクトルを計算.
    vec3 lightVector = lights[lightIndex].positionAndType.xyz - inPosition;

    // 距離による明るさの変化量を計算.
    float lengthSq = dot(lightVector, lightVector);
    float intensity = 1.0 / (1.0 + lengthSq);

    lightVector = normalize(lightVector);

    // 範囲外に光が影響しないように制限する.
    // 物理的には不正確(星の光がどれだけの距離を飛んで地球に届いているかを考えるとよい)だが、ゲームではそれっぽければ十分なので.
    // 参考: https://www.3dgep.com/forward-plus/
    const float fallOff = 0.75; // 減衰を開始する距離(比率).
    float rangeSq = lights[lightIndex].colorAndRange.a;
    rangeSq *= rangeSq;
    float attenuation = smoothstep(rangeSq,
      rangeSq * (fallOff * fallOff), lengthSq);
    if (attenuation <= 0) {
      continue;
    }
    intensity *= attenuation;

    // スポットライトの場合、円錐外周部の減衰を計算する.
    if (lights[lightIndex].positionAndType.w == Type_SpotLight) {
      vec3 direction = lights[lightIndex].direction.xyz;
      float coneAngle = lights[lightIndex].coneAndFalloffAngle.x;
      // ライトからフラグメントへ向かうベクトルと、スポットライトのベクトルのなす角が
      // ライトが照らす角度以上なら範囲外.
      // cosθで比較しているため、不等号が逆になることに注意.
      float angle = dot(direction, -lightVector);
      if (angle <= coneAngle) {
        continue;
      }
      // 減衰開始角度と外周角度の間で補間.
      float falloffAngle = lights[lightIndex].coneAndFalloffAngle.y;
      intensity *= 1 - smoothstep(falloffAngle, coneAngle, angle);
      if (intensity <= 0) {
        continue;
      }
    }

    // 面の傾きによる明るさの変化量を計算.
    float theta = 1;
    if (lengthSq > 0) {
      theta = max(dot(worldNormal, lightVector), 0);
    }

    float F = Fresnel(f0, -directionalLight.direction.xyz, viewVector);

    // 変化量をかけ合わせて明るさを求め、ライトの明るさ変数に加算.
    totalLightColor += lights[lightIndex].colorAndRange.rgb * theta * intensity * (1 - F);

    totalSpecularColor += lights[lightIndex].colorAndRange.rgb * theta * intensity *
      computeSpecular(lightVector, worldNormal, viewVector, shininess, normalizationFactor) * F;
  }

  fragColor = inColor * texture(texColor, inTexcoord);
  vec3 diffuse  = fragColor.rgb * (1 - metallic) * (1 / pi);
  vec3 specular = mix(vec3(1), fragColor.rgb, metallic);
  fragColor.rgb = (diffuse * totalLightColor) + (specular * totalSpecularColor) +
    (ambientLight.rgb * fragColor.rgb * metallic);

  //fragColor.rgb = texture(texNormal, inTexcoord).rgb;
  //fragColor.rgb = fragColor.rgb * 0.0000001 + (worldNormal * 0.5 + 0.5);
  //fragColor.rgb = fragColor.rgb * 0.0001 + vec3((shininess - 2) / 48);
#if 0
  fragColor.rgb = vec3(
    smoothstep(40, 64, float(lightCount)),
    smoothstep(20, 40, float(lightCount)),
    smoothstep(0, 20, float(lightCount)) + dot(fragColor.rgb, vec3(0.3, 0.6, 0.1)) * 0.1;
#endif
}

