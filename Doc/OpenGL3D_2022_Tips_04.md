[OpenGL 3D 2022 Tips 第04回]

# glTFファイルのアニメーション

## 習得目標

* バインドポーズ行列、逆バインドポーズ行列の役割を説明できる。
* ボーンIDとウェイトの役割を説明できる。
* ボーンIDとウェイトを使ってスケルタルアニメーションを行うシェーダを作成できる。

## 1. 関節を行列であらわす

### 1.1 関節とローカル座標

ロボットアームのモデルについて考えてみましょう。このロボットアームには根本、肘、手首の3つの関節があります。先端は溶接機になっていて指はありません。

<p align="center">
<img src="images/Tips_04_robotarm.png" width="30%" />
</p>

ロボットアームを単一のモデルとして作ってしまうと、関節を動かせません。そこで、根本、上腕、下腕、手の4つのモデルに分割します。これらは別々のローカル座標系を持ちますので、ロボットアームのローカル座標系は合計4つということになります。

<p align="center">
<img src="images/Tips_04_robotarm_local_coordinates.png" width="30%" />
</p>

ロボットアームの場合、根本モデルを親として、その子が上腕モデル、孫が下腕モデル、ひ孫が手モデルという関係になります。この関係を行列で書くと次のようになります。

```txt
根本のローカル→ワールド変換行列 = 根本のローカル行列
上腕のローカル→ワールド変換行列 = 根本のローカル行列 * 上腕のローカル行列
下腕のローカル→ワールド変換行列 = 根本のローカル行列 * 上腕のローカル行列 * 下腕のローカル行列
手のローカル→ワールド変換行列   = 根本のローカル行列 * 上腕のローカル行列 * 下腕のローカル行列 *
  手のローカル行列
```

それぞれの式をよく見ると、自分自身の変換行列は、すぐ上の変換行列と自分自身のローカル行列の乗算であることが分かります。このため、式を次のように変形できます。

```txt
根本のローカル→ワールド変換行列 = 根本のローカル行列
上腕のローカル→ワールド変換行列 = 根本のローカル→ワールド変換行列 * 上腕のローカル行列
下腕のローカル→ワールド変換行列 = 上腕のローカル→ワールド変換行列 * 下腕のローカル行列
手のローカル→ワールド変換行列   = 下腕のローカル→ワールド変換行列 * 手のローカル行列
```

ロボットアームの根本モデルはワールド座標系に配置されます。これは、ワールド座標系が親になると考えられます。ワールド座標系の行列を追加すると、次のようになります。

```txt
根本のローカル→ワールド変換行列 = ワールド→ワールド変換行列       * 根本のローカル行列
上腕のローカル→ワールド変換行列 = 根本のローカル→ワールド変換行列 * 上腕のローカル行列
下腕のローカル→ワールド変換行列 = 上腕のローカル→ワールド変換行列 * 下腕のローカル行列
手のローカル→ワールド変換行列   = 下腕のローカル→ワールド変換行列 * 手のローカル行列
```

これは、全てモデルの変換行列が次の式で表されると考えられます。

```txt
自分のローカル→ワールド変換行列 = 親の変換行列 * 自分のローカル行列
```

つまり、座標系の親子関係は、このたったひとつのルールによって作られているわけです。このような、関節をつないでモデルを制御する仕組みを「スケルトン」といいます。

そして、スケルトンをアニメーションさせることを「スケルタル・アニメーション」、スケルタルアニメーションによって制御されるメッシュのことを「スケルタル・メッシュ」または「スキン・メッシュ」といいます。

>`skin`(スキン)は「<ruby>皮膚<rt>ひふ</rt></ruby>」という意味です。

<br>

### 1.2 関節と単一モデル

ロボットアームのようにパーツに分割できるモデルについては、ここまで説明してきた手法で親子関係を作って動かすことができます。しかし、人間のように分割できないモデルは少し工夫が必要です。

>**【箱人間でいいのなら】**<br>
>マインクラフトのキャラクターのように、モデルを非常に単純化すれば対応できます。

そこで、モデルごとにボーンを割り当てるのではなく、頂点ごとにボーンを割り当てることを考えます。これができれば、頂点ごとにボーンによってアニメーションさせることができます。

ローカル座標系はボーンごとに存在するので、ひとつのモデルに複数のローカル座標系が存在できなければなりません。

しかし、ひとつのモデルにはひとつのローカル座標系しか存在できません。この矛盾を解決しないかぎり、分割できないモデルの関節を動かすことはできません。

ロボットアームのようにパーツごとに分割できるモデルでは、全てのモデルは原点にあります。そして、対応するボーンのローカル行列によって、ワールド座標系の適切な位置に移動していくわけです。

<p align="center">
<img src="images/Tips_04_robotarm_all_parts_in_local.png" width="20%" />→
<img src="images/Tips_04_robotarm_all_parts_in_world.png" width="30%" /><br>
[左:初期状態 右:ローカル行列を適用]
</p>

さて、仮にロボットアームが単一モデルであり、頂点ごとにボーンが割り当てられているとします。このモデルの頂点に対応するボーンのローカル行列を適用すると、次のようにパーツの位置や角度が大きくずれてしまいます。

<p align="center">
<img src="images/Tips_04_robotarm_in_local.png" width="20%" />→
<img src="images/Tips_04_robotarm_in_world.png" width="30%" /><br>
[左:初期状態 右:ローカル行列を適用]
</p>

単一モデルの各頂点の位置や角度は、パーツ分割モデルにローカル行列を掛けたあとの状態と同じです。つまり、既にローカル行列を掛けてあるわけです。そこにさらにローカル行列を掛けてしまうと、ローカル行列を2回掛けたことになってしまうわけです。

### 1.3 逆バインドポーズ行列

この問題を解決するためには、最初のローカル行列の影響を打ち消さなくてはなりません。そのために、「バインドポーズ行列」と「逆バインドポーズ行列」という、2種類の行列を導入します。

>バインドポーズ行列と逆バインドポーズ行列は、OBJファイルをグループ単位で座標変換させるときに使ったのと同じものです。

「バインド・ポーズ」というのは、モデルのローカル座標系で定義された、何のアニメーションもしていない素の状態のボーンの姿勢のことです。バインドポーズ行列は、ボーンのローカル座標系をモデルのローカル座標系に変換します。

<p align="center">
<img src="images/Tips_04_robotarm_bindpose.png" width="30%" /><br>
[ボーンローカル座標系→モデルローカル座標系]
</p>

つまりバインドポーズ行列は、「バインドポーズに変換するためのローカル行列」ということです。パーツ分割モデルにバインドポーズ行列を掛けると、各パーツは単一モデルと同じ位置に表示されます。

逆バインドポーズ行列は、名前の通りバインドポーズ行列の逆の操作をします。つまり、座標をモデルのローカル座標系からボーンのローカル座標系へと変換します。

<p align="center">
<img src="images/Tips_04_robotarm_inverse_bindpose.png" width="30%" /><br>
[モデルローカル座標系→ボーンローカル座標系]
</p>

モデルの頂点に逆バインドポーズ行列を掛けると、パーツ分割モデルにローカル行列を掛ける前と同じ状態を作り出すことができます。つまり、最初のローカル行列の影響を打ち消すことができるわけです。

そうなれば、あとはパーツ分割モデルと全く同じ方法で、ワールド座標系へと変換することができます。単一モデルに逆バインドポーズ行列を掛けると、各パーツはパーツ分割モデルと同じ位置に表示されます。

逆バインドポーズ行列によって頂点をボーンのローカル座標系へと変換し、次にバインドポーズ行列によってモデル座標系に戻す、この2つの操作がスケルタルメッシュの基本的な仕組みです。

### 1.4 ボーンウェイトと座標補間

人間の関節を観察すると、関節の根本にある皮膚は、完全に一方の関節だけに引っ張られるのではないことが分かります。この挙動を再現するために、スケルタルメッシュでは「ボーンウェイト」という数値を使ってボーンの影響度合いを設定します。

すべての頂点には、その頂点に影響を与える複数の「ボーンID」と、各ボーンが及ぼす影響の強さを示す「ボーンウェイト」というパラメータを設定します。

実際のアニメーションでは、まずボーンIDに対応する座標変換行列を使って座標をワールド座標系に変換します。これを影響するボーンの数だけ行います。

>```c++
>// ボーン1, 2, 5, 7が影響する場合
>ワールド頂点座標1 = ボーン1の座標変換行列 * ローカル頂点座標
>ワールド頂点座標2 = ボーン2の座標変換行列 * ローカル頂点座標
>ワールド頂点座標5 = ボーン5の座標変換行列 * ローカル頂点座標
>ワールド頂点座標7 = ボーン7の座標変換行列 * ローカル頂点座標
>```

影響するボーンの全ての座標変換がすんだら、それらをボーンウェイトの比率で合成し、最終的な座標を得ます。

>```c++
>// ボーン1, 2, 5, 7のボーンウェイトが(0.1, 0.3, 0.5, 0.1)の場合
>ワールト頂点座標 = ワールド頂点座標1 * 0.1
>                  + ワールド頂点座標2 * 0.3
>                  + ワールド頂点座標5 * 0.5
>                  + ワールド頂点座標7 * 0.1
>```

これが基本ですが、実際のプログラムでは計算回数を減らすためにちょっと工夫をします。といっても、計算順序を変えるだけです。つまり、座標変換をしてから座標を合成するのではなく、座標変換行列を合成してから座標変換を行います。

>```c++
>// ボーン1, 2, 5, 7が影響し、ボーンウェイトが(0.1, 0.3, 0.5, 0.1)の場合
>座標変換行列 = ボーン1の座標変換行列 * 0.1
>              + ボーン2の座標変換行列 * 0.3
>              + ボーン5の座標変換行列 * 0.5
>              + ボーン7の座標変換行列 * 0.1
>ワールド頂点座標 = 座標変換行列 * ローカル頂点座標
>```

影響するボーンが8本あるとします。行列とベクトルの乗算(座標変換)に必要な計算回数は加算`12`回+乗算`16`回、ベクトルの合成にはボーン1本につき加算`4`回、乗算`4`回の計算が必要です。また、行列の合成にはボーン1本につき加算`16`回+乗算`16`回の計算が必要です。

8本のボーンについて座標変換をしてからベクトルを合成する場合、その計算回数は

>加算: `12*8 + 4*7 = 124`<br>
>乗算: `16*8 + 4*8 = 160`
>合計`284`回

となります。行列を合成してから座標変換する場合、その計算回数は

>加算: `16*7 + 12 = 124`<br>
>乗算: `16*8 + 16 = 144`
>合計`268`回

となります。このように、行列を合成してから座標変換するほうが計算回数が少なくなるのです。そのため、多くのスケルタルアニメーション・プログラムでは、「行列を合成してから座標変換する」という順序を採用しています。

>**【ボーン数が少ない場合は？】**<br>
>影響するボーンの数が3本以下の場合、座標変換してからベクトルを合成するほうが、計算回数が少なくなります。もし影響するボーン数を3本以下に制限できる場合は「座標変換→合成」の手順を使うとよいでしょう。

### 1.5 glTFのデータ型を追加する

まずglTFの構造を再掲します。

<p align="center">
<img src="images/Tips_03_gltf_concept_view.png" width="50%" />
</p>

今回は`scene`(シーン)、`node`(ノード)、`skin`(スキン)、`animation`(アニメーション)を読み込みます。アニメーションは手間がかかるので、`scene`、`node`、`skin`の3つから読み込んでいきます。

「シーン」はglTFのノードの始点を決めるデータです。glTFファイルには複数のシーンが存在する可能性がありますが、一度に表示されるのはひとつのシーンだけです。

glTFを正しく表示するには、ひとつのシーンに含まれるすべてのメッシュを描画する必要があります。

`node`と`skin`にはスケルタルメッシュの情報が格納されています。ノードには「ボーンの親子関係」が記録されています。

スキンには「ボーンとして使用するノードIDの配列」と「逆バインドポーズ行列」という2つのデータが含まれます。

<p align="center">
<img src="images/Tips_04_gltf_node_and_skin.png" width="75%" />
</p>

glTFではすべての「親子関係」は「ノード」で表されます。つまり、ボーンだけでなく、カメラの位置や姿勢、さまざまなモデルの配置情報などを含んでいる可能性があります。

そのため、「どのノードがスケルトンなのか」をあらわす「スキン」データが必要なわけです。

シーンとノード、およびスキンを読み込むために、これらを表す構造体を追加します。
`GltfMesh.h`を開き、`GltfMesh`構造体の定義の下に次のプログラムを追加してください。

```diff
   std::string name; // メッシュ名
   std::vector<GltfPrimitive> primitives;
 };
+
+/**
+* スキン
+*/
+struct GltfSkin
+{
+  std::string name; // スキン名
+
+  // ジョイント(ボーン)データ
+  struct Joint {
+    int nodeId;
+    glm::mat4 matInverseBindPose;
+  };
+  std::vector<Joint> joints;
+};
+
+/**
+* ノード
+*/
+struct GltfNode
+{
+  std::string name; // ノード名
+  int mesh = -1;    // メッシュ番号
+  int skin = -1;    // スキン番号
+  GltfNode* parent = nullptr;         // 親ノード
+  std::vector<GltfNode*> children;    // 子ノード配列
+  glm::mat4 matLocal = glm::mat4(1);  // ローカル行列
+  glm::mat4 matGlobal = glm::mat4(1); // グローバル行列
+};
+
+/**
+* シーン
+*/
+struct GltfScene
+{
+  std::vector<const GltfNode*> nodes;
+  std::vector<const GltfNode*> meshNodes;
+};

 /**
 * ファイル
 */
 struct GltfFile
 {
   std::string name; // ファイル名
+  std::vector<GltfScene> scenes;
+  std::vector<GltfNode> nodes;
+  std::vector<GltfSkin> skins;
   std::vector<GltfMesh> meshes;
   std::vector<GltfMaterial> materials;
 };
```

glTFでは「ボーン」のことを「ジョイント」と呼びます。ジョイントに対応するノード番号と、逆バインドポーズ行列を持ちます。

ところで、glTFファイルの「ノード」データには子ノードの配列しかありません。しかし、プログラムでは子から親をたどれると便利です。そこで、`GltfNode`構造体には、親ノードをあらわす`parent`メンバを追加しています。

### 1.6 バインディングポイントとデフォルト頂点データを追加する

それではジョイントとウェイトからを読み込んでいきましょう。まず、バインディングポイント定義とデフォルト頂点データにジョイントとウェイトを追加します。

`BindingPoint`列挙型の定義に、次のプログラムを追加してください。

```diff
   bpColor,
   bpTexcoord0,
   bpNormal,
+  bpWeights0,
+  bpJoints0,
 };

 /**
 * デフォルト頂点データ
```

続いて、`DefaultVertexData`構造体の定義に、次のプログラムを追加してください。

```diff
   glm::vec4 color = glm::vec4(1);
   glm::vec2 texcoord = glm::vec2(0);
   glm::vec3 normal = glm::vec3(0, 0, -1);
+  glm::vec4 joints = glm::vec4(0);
+  glm::vec4 weights = glm::vec4(0, 0, 0, 0);
 };

 } // unnamed namespace
```

今回は、頂点ごとに最大4つのジョイントを扱えるようにしていきます。GPUは3D座標を扱うために作られたものなので、`vec4`のような4個単位のデータを効率的に扱うことができます。

そのため、多くのボーン制御プログラムは4本までのボーンを扱えるようになっています。本テキストでも同様の方針をとります。

### 1.7 頂点アトリビュートを設定する

次に、追加したバインディングポイントとデフォルト頂点データを使って、頂点アトリビュートを設定します。`AddFromFile`関数の中にある「頂点アトリビュートを設定するプログラム」に、次のプログラムを追加してください。

```diff
         SetDefaultAttribute(prim.vao, bpNormal, buffer,
           3, offsetof(DefaultVertexData, normal));
       }
+
+      // 頂点アトリビュート(ジョイント番号)
+      if (attributes["JOINTS_0"].is_number()) {
+        const int id = attributes["JOINTS_0"].int_value();
+        SetAttribute(prim.vao, bpJoints0, buffer,
+          accessors[id], bufferViews, binaryList);
+      } else {
+        SetDefaultAttribute(prim.vao, bpJoints0, buffer,
+          4, offsetof(DefaultVertexData, joints));
+      }
+
+      // 頂点アトリビュート(ジョイントウェイト)
+      if (attributes["WEIGHTS_0"].is_number()) {
+        const int id = attributes["WEIGHTS_0"].int_value();
+        SetAttribute(prim.vao, bpWeights0, buffer,
+          accessors[id], bufferViews, binaryList);
+      } else {
+        SetDefaultAttribute(prim.vao, bpWeights0, buffer,
+          4, offsetof(DefaultVertexData, weights));
+      }

      prim.materialNo = currentPrim["material"].int_value();

      // 作成したプリミティブを配列に追加
      mesh.primitives.push_back(prim);
```

ジョイント番号のアクセッサインデックスは`JOINT_0`という名前で取得します。ひとつのジョイントデータは4個のジョイント番号に相当します。もし頂点あたり5個以上のジョイントに対応する場合は`JOINT_1`, `JOINT_2`, ...を追加します。

ウェイトのアクセッサインデックスは`WEIGHT_0`という名前で取得します。ジョイントと同様に4個のウェイトがひとまとまりになっています。5個以上のウェイトに対応する場合は
`WEIGHT_1`, `WEIGHT_2`, ...を追加します。

### 1.8 JSON配列をvec3に変換する関数を定義する

次はノードを読み込みますが、その前にノードのローカル座標変換行列を計算する関数を作成しておきます。ノードのローカル座標変換行列は、行列またはスケール・回転・平行移動の組のどちらかで記録されています。

ただし、JSONには`vec3`のようなデータ型は定義されていません。そのため、glTFではベクトルや行列のデータを`float`の配列として記録しています。そこで、まず`float`の配列からベクトルや行列を作成する関数を作ります。

これらの関数は他のCPPファイルから使うことはないので、無名名前空間に定義します。<br>
まず`vec3`から定義しましょう。`SetDefaultAttribute`関数の定義の下に、次のプログラムを追加してください。

```diff
   vao->SetAttribute(index, index, elementCount, GL_FLOAT, GL_FALSE, 0);
   vao->SetVBO(index, buffer, offset, 0);
 }
+
+/**
+* JSONの配列データをglm::vec3に変換する
+*
+* @param json 変換元となる配列データ
+*
+* @return jsonを変換してできたvec3の値
+*/
+glm::vec3 GetVec3(const Json& json)
+{
+  const std::vector<Json>& a = json.array_items();
+  if (a.size() < 3) {
+    return glm::vec3(0);
+  }
+  return glm::vec3(
+    a[0].number_value(),
+    a[1].number_value(),
+    a[2].number_value());
+}

 // バインディングポイント
 enum BindingPoint
```

### 1.9 JSON配列をquatに変換する関数を定義する

次に`quat`(クォート、クォータニオン)に変換する関数を定義します。GLMライブラリのクォータニオン型を使うには、`quaternion.hpp`をインクルードする必要があります。
`GltfMesh.cpp`の先頭に次のプログラムを追加してください。

```diff
 #include "GLContext.h"
 #include "GameEngine.h"
 #include "json11/json11.hpp"
+#include <glm/gtc/quaternion.hpp>
 #include <fstream>
 #include <filesystem>
```

それでは、`GetQuat`(ゲット・クォート)関数を定義しましょう。`GetVec3`関数の定義の下に次のプログラムを追加してください。

```diff
     a[1].number_value(),
     a[2].number_value());
 }
+
+/**
+* JSONの配列データをglm::quatに変換する
+*
+* @param json 変換元となる配列データ
+*
+* @return jsonを変換してできたquatの値
+*/
+glm::quat GetQuat(const Json& json)
+{
+  const std::vector<Json>& a = json.array_items();
+  if (a.size() < 4) {
+    return glm::quat(0, 0, 0, 1);
+  }
+  return glm::quat(
+    static_cast<float>(a[3].number_value()),
+    static_cast<float>(a[0].number_value()),
+    static_cast<float>(a[1].number_value()),
+    static_cast<float>(a[2].number_value()));
+}

 // バインディングポイント
 enum BindingPoint
```

glTFとGLMライブラリのクォータニオンでは、データの順序が異なる点に注意してください。glTFのクォータニオンの順番は`x, y, z, w`で、GLMの`quat`型では`w, x, y, z`です。

`GetQuat`関数はこの違いを吸収するように作る必要があります。<br>
そこで、配列の添字を`3, 0, 1, 2`としています。

また、`vec3`や`mat4`などのコンストラクタはどんな型でも受け付けますが、`quat`型のコンストラクタ引数は`float`型にしか対応していません。

しかし、JSONの浮動小数点数は`double`型です。そこで、コンパイラの警告を回避するために`float`型にキャストしています。

### 1.10 JSON配列をmat4に変換する関数を定義する

`vec3`、`quat`の次は`mat4`に変換する`GetMat4`(ゲット・マット・フォー)関数を定義します。`GetQuat`関数の定義の下に、次のプログラムを追加してください。

```diff
+
+/**
+* JSONの配列データをglm::mat4に変換する
+*
+* @param json 変換元となる配列データ
+*
+* @return jsonを変換してできたmat4の値
+*/
+glm::mat4 GetMat4(const Json& json)
+{
+  const std::vector<Json>& a = json.array_items();
+  if (a.size() < 16) {
+    return glm::mat4(1);
+  }
+  glm::mat4 m;
+  for (int y = 0; y < 4; ++y) {
+    for (int x = 0; x < 4; ++x) {
+      m[y][x] = static_cast<float>(a[y * 4 + x].number_value());
+    }
+  }
+  return m;
+}

 // バインディングポイント
 enum BindingPoint
```

16回`a[N].number_value()`を書く方法でも良かったのですが、今回は素直に2重ループでデータを取得してみました。

### 1.11 ローカル座標変換行列を計算する関数を定義する

必要なデータを取得できるようになったので、ローカル座標変換行列を計算する関数を定義しましょう。ノードの座標変換行列は次のどちらかの方法で格納されています。

>1. `matrix`という名前で行列がそのまま書いてある。
>2. `translation`, `rotation`, `scale`という3つの名前に分けられている。

1の場合は行列を取得すれば終わりです。2の場合は3つの座標変換行列を作って掛け合わせる必要があります。`GetMat4`関数の定義の下に、次のプログラムを追加してください。

```diff
   }
   return m;
 }
+
+/**
+* ノードのローカル座標変換行列を計算する
+*
+* @param node gltfノード
+*
+* @return nodeのローカル座標変換行列
+*/
+glm::mat4 CalcLocalMatrix(const Json& node)
+{
+  // 行列データがある場合、行列データを読み取って返す。
+  if (node["matrix"].is_array()) {
+    return GetMat4(node["matrix"]);
+  }
+
+  // 行列データがない場合、適用順がスケール→回転→平行移動となるように
+  // 掛け合わせた行列を返す。
+  glm::mat4 m(1);
+  if (node["translation"].is_array()) {
+    m *= glm::translate(glm::mat4(1), GetVec3(node["translation"]));
+  }
+  if (node["rotation"].is_array()) {
+    m *= glm::mat4_cast(GetQuat(node["rotation"]));
+  }
+  if (node["scale"].is_array()) {
+    m *= glm::scale(glm::mat4(1), GetVec3(node["scale"]));
+  }
+  return m;
+}

 // バインディングポイント
 enum BindingPoint
```

ノードに`matrix`という名前の配列データが存在する場合、配列を行列に変換して返します。
`matrix`配列が存在しない場合は、`translation`、`rotation`、`scale`という名前の配列データを調べます。

配列データが存在する場合、平行移動、スケールは`vec3`、回転は`quat`としてデータを取得し、さらにそのデータから行列を作成して合成します。

なお、合成の順序はglTFの仕様で決まっています。その順序は「スケール→回転→平行移動」です。また、GLMライブラリの行列の`*=`演算子は左辺が先に適用される行列になります。

そこで、上記のプログラムでは、

>`((glm::mat4(1) * 平行移動) * 回転) * スケール`

の順となるようにしています。

glTFの仕様によると、回転はクォータニオンで表現されます。GLMライブラリでは`quat * vec3`は定義されているのですが、`mat4 * quat`や`quat * mat4`は定義されていません。

そのため、行列との乗算を行うには`mat4_cast`(マット・フォー・キャスト)関数によって、クォータニオンを行列に変換する必要があります。

<pre class="tnmai_code"><strong>【書式】</strong><code>
glm::mat4 glm::mat4_cast(変換元のクォータニオン);
</code></pre>

これでローカル座標変換行列を計算できるようになりました。

### 1.12 ノードデータを取得する

ローカル座標変換行列を作成できるようになったので、ようやく「ノード」を取得できます。
`LoadFromFile`関数に戻り、マテリアルを作成するプログラムの下に次のプログラムを追加してください。

```diff
     // 取得したデータからマテリアルを作成
     file->materials.push_back({ baseColor, texBaseColor });
   }
+
+  // ノード
+  {
+    const std::vector<Json>& nodes = gltf["nodes"].array_items();
+    file->nodes.resize(nodes.size());
+    for (size_t i = 0; i < nodes.size(); ++i) {
+      const Json& node = nodes[i];
+      GltfNode& n = file->nodes[i];
+
+      // ノード名を取得
+      n.name = node["name"].string_value();
+
+      // メッシュIDを取得
+      const Json& meshId = nodes[i]["mesh"];
+      if (meshId.is_number()) {
+        n.mesh = meshId.int_value();
+      }
+
+      // スキンIDを取得
+      const Json& skinId = nodes[i]["skin"];
+      if (skinId.is_number()) {
+        n.skin = skinId.int_value();
+      }
+    }
+  } // ノード

   // 作成したメッシュを連想配列に追加
   file->name = filename;
   files.emplace(filename, file);
```

このプログラムは、全てのノードに対して「ノード名」、「ノードに表示するメッシュのID」、「メッシュに適用するスキンのID」を取得します。

また、「あとで配列内要素のアドレスを取得する必要がある」という理由から、ループ前に
`resize`関数を使ってノード配列のサイズを確定させています。

>**【resizeやreserveを積極的に使おう】**<br>
>事前に`resize`や`reserve`を利用して必要なメモリを確保しておくと、メモリの再確保による効率低下を避けられます。これはC++では重要なテクニックです。<br>
>`resize`と`reserve`のどちらを使うかは場合によりけりですが、基本的には<br>
>`reserve`を使ってください。今回のテキストでは`resize`を多用していますが、これは、サイズが確定している状況では`resize`のほうが効率的だからです。どちらを使うべきか判断がつかない場合は`reserve`を使ってください。

次に「親ノードポインタの設定」と「ローカル座標変換行列の作成」を行います。スキンIDを取得するプログラムの下に、次のプログラムを追加してください。

```diff
       if (skinId.is_number()) {
         n.skin = skinId.int_value();
       }
+
+      // 自分の子ノードに対して親ノードポインタを設定
+      const std::vector<Json>& children = node["children"].array_items();
+      n.children.reserve(children.size());
+      for (const Json& e : children) {
+        GltfNode& childNode = file->nodes[e.int_value()];
+        n.children.push_back(&childNode);
+        if (!childNode.parent) {
+          childNode.parent = &n;
+        }
+      }
+
+      // ローカル座標変換行列を計算
+      n.matLocal = CalcLocalMatrix(node);
     }
   } // ノード

   // 作成したメッシュを連想配列に追加
```

このプログラムは、全てのノードを取得し、名前と子オブジェクトへのポインタを設定します。

### 1.13 ノードのグローバル座標変換行列を作成する

親子関係にあるノードの場合、子ノードの姿勢は親ノードのローカル座標系で定義されます。

子ノードのローカル座標系で定義された頂点をワールド座標系に変換するには、まず子ノードのローカル座標変換行列を掛け、次に親ノードのローカル座標変換行列を掛ける必要があるということです。

親の親、親の親の親が存在する場合は、それらもすべて掛ける必要があります。これを座標変換が必要になるたびに行うのは無駄が多いので、事前にすべてを掛け合わせた「グローバル座標変換行列」を作成しておきます。

ローカル座標変換行列を作成するプログラムの下に、次のプログラムを追加してください。

```diff
       // ローカル座標変換行列を計算
       n.matLocal = CalcLocalMatrix(nodes[i]);
     }
+
+    // すべてのノードの親子関係とローカル座標行列を設定したので、
+    // 親をたどってグローバル座標変換行列を計算する
+    for (GltfNode& e : file->nodes) {
+      e.matGlobal = e.matLocal;
+      const GltfNode* parent = e.parent;
+      while (parent) {
+        e.matGlobal = parent->matLocal * e.matGlobal;
+        parent = parent->parent;
+      }
+    }
   } // ノード

   // 作成したメッシュを連想配列に追加
```

### 1.14 シーンデータを取得する

「ノード」の次は「シーン」を取得します。glTFの「シーン」オブジェクトには、「シーンに含まれるノードIDの配列」が格納されています。そこでこのプログラムでは、ノードIDからノードポインタを取得して配列に記録しています。

ただし、実際に描画されるのはメッシュを持つノードだけです。なので、メッシュを持つノードをリストアップしておくと効率的に描画を行えます。

そこで、メッシュを持つノードをリストアップする関数を定義します。`GetLocalMatrix`関数の定義の下に、次のプログラムを追加してください。

```diff
     return m;
   }
 }
+
+/**
+* メッシュを持つノードをリストアップする
+*/
+void GetMeshNodeList(const GltfNode* node, std::vector<const GltfNode*>& list)
+{
+  if (node->mesh >= 0) {
+    list.push_back(node);
+  }
+  for (const GltfNode* child : node->children) {
+    GetMeshNodeList(child, list);
+  }
+}

 // バインディングポイント
 enum BindingPoint
```

`GetMeshNodeList`(ゲット・メッシュ・ノード・リスト)関数は、指定されたノードがメッシュを持っている場合はそのノードをリストに追加します。そしてさらに、全ての子ノードに対して`GetMeshNodeList`を再帰的に呼び出します。

それでは「シーン」を取得しましょう。`LoadFromFile`関数に戻り、ノードを取得するプログラムの下に次のプログラムを追加してください。

```diff
       }
     }
   } // ノード
+
+  // シーン
+  {
+    // シーンに含まれるノードを取得
+    const std::vector<Json>& scenes = gltf["scenes"].array_items();
+    file->scenes.resize(scenes.size());
+    for (size_t i = 0; i < scenes.size(); ++i) {
+      GltfScene& s = file->scenes[i];
+      const std::vector<Json>& nodes = scenes[i]["nodes"].array_items();
+      s.nodes.resize(nodes.size());
+      for (size_t j = 0; j < nodes.size(); ++j) {
+        const int nodeId = nodes[j].int_value();
+        const GltfNode* n = &file->nodes[nodeId];
+        s.nodes[j] = n;
+        GetMeshNodeList(n, s.meshNodes);
+      }
+    }
+  } // シーン

   // 作成したメッシュを連想配列に追加
   file->name = filename;
```

### 1.15 バイナリデータを取得する関数を定義する

次は「スキン」を取得しますが、その前に「バイナリファイルに含まれるデータの位置を返す関数」を定義します。

これは、スキンに必要な逆バインドポーズ行列のデータがバイナリファイルに格納されているためです。`GetBufferStride`関数の定義の下に、次のプログラムを追加してください。

```diff
   // 要素のサイズをストライドとする
   return componentSize * elementCount;
 }
+
+/**
+* アクセッサが示すバッファ内のアドレスを取得する
+*
+* @param accessor    アクセッサ
+* @param bufferViews バッファビュー配列
+* @param binaryList  バイナリデータ配列
+*
+* @return バッファ内のアドレス
+*/
+const void* GetBuffer(const Json& accessor, const Json& bufferViews,
+  const BinaryList& binaryList)
+{
+  // アクセッサから必要な情報を取得
+  const int byteOffset = accessor["byteOffset"].int_value();
+  const int bufferViewId = accessor["bufferView"].int_value();
+
+  // バッファビューから必要な情報を取得
+  const Json& bufferView = bufferViews[bufferViewId];
+  const int bufferId = bufferView["buffer"].int_value();
+  const int baseByteOffset = bufferView["byteOffset"].int_value();
+
+  // アクセッサが参照するデータの先頭を計算
+  return binaryList[bufferId].bin.data() + baseByteOffset + byteOffset;
+}

 /**
 * 頂点アトリビュートを設定する
```

### 1.16 スキンを取得する

それでは「スキン」を取得しましょう。`LoadFromFile`関数に戻り、シーンを取得するプログラムの下に、次のプログラムを追加してください。

```diff
       }
     }
   } // シーン
+
+  // スキン
+  {
+    const std::vector<Json>& skins = gltf["skins"].array_items();
+    file->skins.resize(skins.size());
+    for (size_t i = 0; i < skins.size(); ++i) {
+      const Json& skin = skins[i];
+      GltfSkin& s = file->skins[i];
+
+      // スキン名を設定
+      s.name = skin["name"].string_value();
+
+      // 逆バインドポーズ行列の配列を取得
+      const int inverseBindMatricesAccessorId = skin["inverseBindMatrices"].int_value();
+      const Json& accessor = accessors[inverseBindMatricesAccessorId];
+      const glm::mat4* inverseBindMatrices =
+        static_cast<const glm::mat4*>(GetBuffer(accessor, bufferViews, binaryList));
+
+      // 関節データを取得
+      const std::vector<Json>& joints = skin["joints"].array_items();
+      s.joints.resize(joints.size());
+      for (size_t jointId = 0; jointId < joints.size(); ++jointId) {
+        s.joints[jointId].nodeId = joints[jointId].int_value();
+        s.joints[jointId].matInverseBindPose = inverseBindMatrices[jointId];
+      }
+    }
+  } // スキン

   // 作成したメッシュを連想配列に追加
   file->name = filename;
   files.emplace(filename, file);
```

スキンの取得でポイントとなるのは「逆バインドポーズ行列」の取得方法です。glTFの逆バインドポーズ行列はバイナリファイルに格納されているので、前節で作成した`GetBuffer`関数でバイナリファイル内のアドレスを取得しています。

>**【バイナリファイルのデータはGPUメモリに配置したはずなのに、なぜCPU側にも必要なのか？】**<br>
>glTFのバイナリファイルは、基本的にはGPUメモリに配置するためのデータです。<br>
>そのため、バイナリファイルに格納されている逆バインドポーズ行列をCPU側で取得しているのは不思議に思うかもしれません。<br>
>実際、本来なら逆バインドポーズ行列を含むすべての計算はGPU側で行われるので、CPU側で取得する必要はありません。しかし、アニメーションのすべての計算をGPUで行う場合、ボーンアニメーションに直接関係しない、シェーダ間のやりとりに関するプログラムが増えてしまい、ボーンアニメーションの理解を妨げてしまいます。そこで本テキストでは「計算方法を理解すること」を重視して、CPU側でボーンアニメーションを計算することにしました。<br>
>まとめると、「CPU側のメモリに逆バインドポーズ行列を読み込んでいる理由」は「ボーンアニメーションの計算内容を理解するためにはCPU側で扱うほうが都合がよいから」です。

<br>

>**【1章のまとめ】**
>
>* バインドポーズ行列は、ボーンのローカル座標系からモデルのローカル座標系に変換する行列。
>* 逆バインドポーズ行列は、モデルのローカル座標系からボーンのローカル座標系に変換する行列。
>* glTFのスケルタルメッシュは「シーン」、「ノード」、「スキン」によって定義されている。
>* JSONには`vec3`や`mat4`などの型はないので、配列として記録されている。

<div style="page-break-after: always"></div>

## 2. アニメーション

### 2.1 アニメーションデータの構成

この章ではアニメーションを取得します。アニメーションの取得は、ノードと同じくらいのプログラミングが必要なので章を分けました。

さて、改めてアニメーションデータの構成を確認しましょう。glTF概要図によると、glTFのアニメーションデータは以下の構造になっています。

<p align="center">
<img src="images/Tips_04_animation_data.png" width="33%" />
</p>

この図で示されているように、アニメーションは`channels`(チャネルズ)と`samplers`(サンプラーズ)という2つの配列を持ちます。

`channels`はチャネル(またはチャンネル)の配列で、アニメーションの`target`(ターゲット、対象)と、ターゲットに適用するアニメーションデータを指す`sampler`(サンプラ)を指定します。

`target`には「ノードID」を示す`node`(ノード)、「座標変換の種類」を示す`path`(パス)という2つのデータがあります。`sampler`は`samplers`配列のインデックスを示します(上図の矢印)。「座標変換の種類」は以下の3種類です

| path | 型 | 種類 |
|:-:|:-:|:-:|
| <ruby>translation<rt>トランスレーション</rt></ruby> | vec3 | 平行移動 |
| <ruby>rotation<rt>ローテーション</rt></ruby> | quat | 回転 |
| <ruby>scale<rt>スケール</rt></ruby> | vec3 | 拡大・縮小 |

>pathには`weights`(ウェイツ)というデータもありますが、今回は対応しません。

`path`に指定された文字列によってデータの型が異なる点に注意してください。今回はデータごとに対応するクラスを作るのではなく、クラステンプレートとして定義することにします。

`sampler`にはキーフレームの「時刻」のアクセッサIDを示す`input`(インプット)と、チャネルに適用する「<ruby>値<rt>あたい</rt></ruby>」のアクセッサIDを示す`output`(アウトプット)、そしてキーフレーム間の補間方法を示す`interpolation`(インターポレーション)、という3つのデータがあります。

>**【キーフレームって？】**<br>
>「キーフレーム」は「アニメーションが大きく変化する瞬間を表す」フレームのことです。例えば、強力なパンチのアニメーションを30フレームで作成するとします。<br>
>しかし、全てのフレームの平行移動・回転・拡大縮小を設定するのは手間がかかりすぎます。そこで、腕の動きが大きく変化するフレーム(例えば0, 5, 20, 25, 30フレーム目)だけを作成し(これがキーフレーム)、中間部分はコンピューターに補間させます。

<p align="center">
<img src="images/Tips_04_animation_input_output.png" width="45%" /><br>
[input=キーフレーム]
</p>

### 2.2 アニメーションを扱うクラスを定義する

それでは、アニメーションを管理する構造体を定義しましょう。`GltfMesh.h`を開き、先行宣言の下に次のプログラムを追加してください。

```diff
 using TexturePtr = std::shared_ptr<Texture>;
 class VertexArrayObject;
 using VertexArrayObjectPtr = std::shared_ptr<VertexArrayObject>;
+
+/**
+* アニメーションの補間方法
+*/
+enum class GltfInterpolation
+{
+  step,        // 補間なし
+  linear,      // 線形補間
+  cubicSpline, // 3次スプライン補間
+};
+
+/**
+* アニメーションのキーフレーム
+*/
+template<typename T> struct GltfKeyframe
+{
+  float time; // 時刻
+  T value;    // 適用する値
+};
+
+/**
+* アニメーションのチャネル
+*/
+template<typename T> struct GltfChannel
+{
+  int targetNodeId;                       // 値を適用するノードID
+  GltfInterpolation interpolation;        // 補間方法
+  std::vector<GltfKeyframe<T>> keyframes; // キーフレーム配列
+};
+
+/**
+* アニメーション
+*/
+struct GltfAnimation
+{
+  std::string name; // アニメーション名
+  std::vector<GltfChannel<glm::vec3>> translations; // 平行移動チャネルの配列
+  std::vector<GltfChannel<glm::quat>> rotations;    // 回転チャネルの配列
+  std::vector<GltfChannel<glm::vec3>> scales;       // 拡大縮小チャネルの配列
+  float totalTime = 0;
+};
+using GltfAnimationPtr = std::shared_ptr<GltfAnimation>;

 /**
 * マテリアル
```

`GltfAnimation`(ジーエルティーエフ・アニメーション)構造体が、ひとつのアニメーションを表します。個々のアニメーションは平行移動、回転、拡大縮小の3つのターゲットに対応するチャネル配列を持ち、各チャネルはキーフレーム配列を持ちます。

つまり、次のような関係になっています。

>```txt
>GltfAnimation → GltfChannel(ノード1の平行移動) → GltfKeyframe[Na]
>                 GltfChannel(ノード1の回転)     → GltfKeyframe[Nb]
>                 GltfChannel(ノード3の拡大縮小) → GltfKeyframe[Nc]
>                 GltfChannel(ノード4の回転)     → GltfKeyframe[Nd]
>                 GltfChannel(ノード7の拡大縮小) → GltfKeyframe[Ne]
>※Na～Neは任意の整数
>```

次に、`GltfFile`構造体の定義にアニメーションの配列を追加してください。

```diff
   std::vector<GltfSkin> skins;
   std::vector<GltfMesh> meshes;
   std::vector<GltfMaterial> materials;
+  std::vector<GltfAnimationPtr> animations;
 };
 using GltfFilePtr = std::shared_ptr<GltfFile>;
```

これで構造体の準備は完了です。

### 2.3 アニメーションデータを取得する

追加したアニメーション配列に、glTFファイルから取得したアニメーションデータを設定しましょう。`GltfMesh.cpp`を開き、`LoadFromFile`関数のスキンを取得するプログラムの下に、次のプログラムを追加してください。

```diff
       }
     }
   } // スキン
+
+  // アニメーション
+  {
+    const std::vector<Json>& animations = gltf["animations"].array_items();
+    file->animations.resize(animations.size());
+    for (size_t i = 0; i < animations.size(); ++i) {
+      const Json& animation = animations[i];
+      const std::vector<Json>& channels = animation["channels"].array_items();
+      const std::vector<Json>& samplers = animation["samplers"].array_items();
+
+      // 名前を設定
+      GltfAnimationPtr a = std::make_shared<GltfAnimation>();
+      a->name = animation["name"].string_value();
+
+      // アニメーションを設定
+      file->animations[i] = a;
+    }
+  } // アニメーション

   // 作成したメッシュを連想配列に追加
   file->name = filename;
   files.emplace(filename, file);
```

このプログラムでは、アニメーションデータ配列の参照変数を定義し、次にアニメーション名を取得しています。そして最後に、作成したアニメーションオブジェクトを配列に設定します。

次にチャネルの容量を予約します。アニメーションの名前を設定するプログラムの下に、次のプログラムを追加してください。

```diff
       // 名前を設定
       GltfAnimationPtr a = std::make_shared<GltfAnimation>();
       a->name = animation["name"].string_value();
+
+      // チャネル配列の容量を予約
+      // 一般的に、平行移動・回転・拡大縮小の3つはセットで指定するので、
+      // 各チャネル配列のサイズは「総チャネル数 / 3」になる可能性が高い。
+      // 安全のため、予測サイズが必ず1以上になるように1を足している。
+      const size_t predictedSize = channels.size() / 3 + 1; // 予測サイズ
+      a->translations.reserve(predictedSize);
+      a->rotations.reserve(predictedSize);
+      a->scales.reserve(predictedSize);

       // アニメーションを設定
       file->animations[i] = a;
     }
```

`predictedSize`(プレディクテッド・サイズ、予測サイズ)変数は、3つのチャネル配列の予測サイズを表します。

`channels`で取得できるチャネル配列には、平行移動・回転・拡大縮小がまとめて格納されています。通常、これらは同じキーフレームで作成されるので、チャネル配列のサイズは「いずれかの`path`の3倍」になることが多いです。

そこで、予測サイズを「総チャネル数 / 3」としています。ただし、総チャネル数が2以下の場合に3で割ると、予測サイズが0になってしまいます。そのため、0になることを避けるために1を足しています。

続いて、チャネルを取得する`for`ループを追加します。チャネルの容量を予約するプログラムの下に、次のプログラムを追加してください。

```diff
       a->translations.reserve(predictedSize);
       a->rotations.reserve(predictedSize);
       a->scales.reserve(predictedSize);
+
+      // チャネル配列を設定
+      a->totalTime = 0;
+      for (const Json& e : channels) {
+        const int samplerId = e["sampler"].int_value();
+        const Json& sampler = samplers[samplerId];
+        const Json& target = e["target"];
+        const int targetNodeId = target["node"].int_value();
+        if (targetNodeId < 0) {
+          continue; // 対象ノードIDが無効
+        }
+
+        // 時刻の配列を取得
+        const int inputAccessorId = sampler["input"].int_value();
+        const int inputCount = accessors[inputAccessorId]["count"].int_value();
+        const GLfloat* pTimes = static_cast<const GLfloat*>(
+          GetBuffer(accessors[inputAccessorId], bufferViews, binaryList));
+
+        // 値の配列を取得
+        const int outputAccessorId = sampler["output"].int_value();
+        const void* pValues =
+          GetBuffer(accessors[outputAccessorId], bufferViews, binaryList);
+      }

       // アニメーションを設定
       file->animations[i] = a;
```

キーフレームの「時刻」の配列と「値」の配列は、どちらもバイナリデータに記録されています。そこで、`GetBuffer`関数を使ってデータのアドレスを計算します。

また、glTFの仕様では、キーフレームは常に`GLfloat`(=`float`)型と決められています。そのため、`componentType`を調べなくても`GetBuffer`の戻り値を`const GLfloat*`にキャストできます。

>安全のために`componentType`が`GL_FLOAT`であることをチェックするのは良い考えです。型が違う場合、そのglTFファイルのデータは信頼できないので、`false`を返して関数を終了するとよいでしょう。

値は`vec3`と`quat`のどちらかですが、どちらなのかは`path`を見るまで分かりません。とりあえず`const void*`型の`pValues`(ピー・バリューズ)変数に入れておきます。

次に「補間方法」を取得します。値の配列を取得するプログラムの下に、次のプログラムを追加してください。

```diff
         const int outputAccessorId = sampler["output"].int_value();
         const void* pValues =
           GetBuffer(accessors[outputAccessorId], bufferViews, binaryList);
+
+        // 補間方法を取得
+        const std::string& interpolation = target["interpolation"].string_value();
+        GltfInterpolation interp = GltfInterpolation::linear;
+        if (interpolation == "LINEAR") {
+          interp = GltfInterpolation::linear;
+        } else if (interpolation == "STEP") {
+          interp = GltfInterpolation::step;
+        } else if (interpolation == "CUBICSPLINE") {
+          interp = GltfInterpolation::cubicSpline;
+        }
       }

       // アニメーションを設定
       file->animations[i] = a;
```

「補間方法」はキーフレームのあいだの値を作り出す方法を指定します。以下の3つのうちいずれかの文字列として定義されます。

| 文字列 | 補間方法 |
|:-:|:--|
| <ruby>STEP<rt>ステップ</rt></ruby> | 補間しない |
| <ruby>LINEAR<rt>リニア</rt></ruby> | 線形補間 |
| <ruby>CUBICSPLINE<rt>キュービック・スプライン</rt></ruby> | 3次スプライン補間 |

文字列のままではプログラムで扱いにくい(比較が遅い、メモリが余分に必要、インテリセンスが効かない)ので、列挙値に変換しています。

### 2.4 アニメーションチャネルを取得する

アニメーションは「チャネルの集合」です。ただし、チャネルには「平行移動」、「回転」、「拡大縮小」の3つの種類があり、それぞれ扱い方が異なります。そこで、`GltfAnimation`構造体は「3種類のチャネルの集合」として定義しています。

チャネルの種類によって値の型が異なるため、`GltfChannel`と`GltfKeyframe`はクラス(構造体)テンプレートとして定義したのでした。

「値の型が異なる」ということは、`pValues`が指すアドレスに格納されている型も異なるということです。そのため、値を取得する関数は、「型ごとに作る」か「関数テンプレートで作る」のどちらかを選ぶことになります。

「型ごとに作る」のは簡単ですが、同じような処理を複数書く必要があります。「関数テンプレートで作る」場合は1回書けば終わりです。書く量が少なくて済むので、今回は関数テンプレートで書くことにします。

`GetMeshNodeList`関数の定義の下に、次のプログラムを追加してください。

```diff
     GetMeshNodeList(child, list);
   }
 }
+
+/**
+* アニメーションチャネルを作成する
+*
+* @param pTimes       時刻の配列のアドレス
+* @param pValues      値の配列のアドレス
+* @param inputCount   配列の要素数
+* @param targetNodeId 値の適用対象となるノードID
+* @param interp       補間方法
+* @param totalTime    総再生時間を格納する変数のアドレス
+*
+* @return 作成したアニメーションチャネル
+*/
+template<typename T>
+GltfChannel<T> MakeAnimationChannel(
+  const GLfloat* pTimes, const void* pValues, size_t inputCount,
+  int targetNodeId, GltfInterpolation interp, float* totalTime)
+{
+  // 時刻と値の配列からキーフレーム配列を作成
+  const T* pData = static_cast<const T*>(pValues);
+  GltfChannel<T> channel;
+  channel.keyframes.resize(inputCount);
+  for (int i = 0; i < inputCount; ++i) {
+    *totalTime = std::max(*totalTime, pTimes[i]); // 総再生時間を更新
+    channel.keyframes[i] = { pTimes[i], pData[i] };
+  }
+
+  // 適用対象ノードIDと補間方法を設定
+  channel.targetNodeId = targetNodeId;
+  channel.interpolation = interp;
+
+  return channel; // 作成したチャネルを返す
+}

 // バインディングポイント
 enum BindingPoint
```

それでは、`path`を調べて対応するチャネル配列を作成しましょう。保管方法を取得するプログラムの下に、次のプログラムを追加してください。

```diff
             interp = GltfInterpolation::cubicSpline;
           }
         }
+
+        // 時刻と値の配列からチャネルを作成し、pathに対応する配列に追加
+        const std::string& path = target["path"].string_value();
+        if (path == "translation") {
+          a->translations.push_back(MakeAnimationChannel<glm::vec3>(
+            pTimes, pValues, inputCount, targetNodeId, interp, &a->totalTime));
+        } else if (path == "rotation") {
+          a->rotations.push_back(MakeAnimationChannel<glm::quat>(
+            pTimes, pValues, inputCount, targetNodeId, interp, &a->totalTime));
+        } else if (path == "scale") {
+          a->scales.push_back(MakeAnimationChannel<glm::vec3>(
+            pTimes, pValues, inputCount, targetNodeId, interp, &a->totalTime));
+        }
       }

       // アニメーションを設定
       file->animations[i] = a;
```

### 2.5 アニメーション名をデバッグ出力する

アプリ実行時に、どのアニメーションが読み込まれているのかを確認できるように、デバッグ出力を追加します。`LoadFromFile`関数のデバッグ情報を出力するプログラムに、次のプログラムを追加してください。

```diff
   for (size_t i = 0; i < file->meshes.size(); ++i) {
     std::cout << "  meshes[" << i << "]=\"" << file->meshes[i].name << "\"\n";
   }
+  for (size_t i = 0; i < file->animations.size(); ++i) {
+    std::string name = file->animations[i]->name;
+    if (name.size() <= 0) {
+      name = "<NO NAME>";
+    } else {
+      name = std::string("\"") + name + "\"";
+    }
+    std::cout << "  animations[" << i << "]=" << name << "\n";
+  }

   return true;
 }
```

これで、アニメーションデータの読み込みは完了です。

### 2.6 姿勢行列を計算する関数を宣言する

スケルタルアニメーションは、「ボーンの姿勢を表す行列(姿勢行列)」が時間によって変化することでアニメーションを行います。

>ここでいう「姿勢を表す行列、または姿勢行列」は「平行移動・回転・拡大縮小を合成した行列」のことで、つまり「座標変換行列」を言い換えたものです。

アニメーションを行うためには「ある時間における姿勢行列」を計算しなくてはなりません。そこで、姿勢行列を計算する関数を作成します。

関数名は`CalcAnimationMatrices`(カルク・アニメーション・マトリシーズ)とします。
`GltfMesh.h`を開き、`GltfFilePtr`型の定義の下に次のプログラムを追加してください。

```diff
   std::vector<GltfAnimationPtr> animations;
 };
 using GltfFilePtr = std::shared_ptr<GltfFile>;
+
+// アニメーション用座標変換行列の配列
+using GltfAnimationMatrices = std::vector<glm::mat4>;
+
+GltfAnimationMatrices CalcAnimationMatrices(const GltfFilePtr& file,
+  const GltfNode* meshNode, const GltfAnimation* animation,
+  const std::vector<int>& nonAnimatedNodes, float time);

 /**
 * メッシュを管理するクラス
```

### 2.7 値を補間する関数を定義する

`CalcAnimationMatrices`関数を定義する前に、2つの補助的な関数を定義しておきます。

ひとつは値の補間を行う`Interpolate`(インターポレート)関数で、もうひとつは、ローカル姿勢行列からグローバル姿勢行列を作成する`CalcGlobaNodeMatrix`(カルク・グローバル・ノード・マトリックス)関数です。

とりあえず、必要なヘッダファイルをインクルードしましょう。`GltfMesh.cpp`を開き、
`algorithm`(アルゴリズム)ヘッダファイルをインクルードしてください。

```diff
 #include <fstream>
 #include <filesystem>
 #include <iostream>
+#include <algorithm>

 using namespace json11;
```

それでは、`Interpolate`関数を定義しましょう(`Interpolate`は「補間する、修正する」という意味です)。`GltfMesh.cpp`を開き、`MakeAnimationChannel`関数の定義の下に、次のプログラムを追加してください。

```diff
   channel.interpolation = interp;

   return channel; // 作成したチャネルを返す
 }
+
+/**
+* チャネル上の指定した時刻の値を求める
+*
+* @param channel 対象のチャネル
+* @param time    値を求める時刻
+*
+* @return 時刻に対応する値
+*/
+template<typename T>
+T Interpolate(const GltfChannel<T>& channel, float time)
+{
+  // time以上の時刻を持つ、最初のキーフレームを検索
+  const auto curOrOver = std::lower_bound(
+    channel.keyframes.begin(), channel.keyframes.end(), time,
+    [](const GltfKeyframe<T>& keyframe, float time) {
+      return keyframe.time < time; });
+
+  // timeが先頭キーフレームの時刻と等しい場合、先頭キーフレームの値を返す
+  if (curOrOver == channel.keyframes.begin()) {
+    return channel.keyframes.front().value;
+  }
+
+  // timeが末尾キーフレームの時刻より大きい場合、末尾キーフレームの値を返す
+  if (curOrOver == channel.keyframes.end()) {
+    return channel.keyframes.back().value;
+  }
+
+  // timeが先頭と末尾の間だった場合...
+
+  // キーフレーム間の時間におけるtimeの比率を計算
+  const auto prev = curOrOver - 1; // ひとつ前の(time未満の時刻を持つ)キーフレーム
+  const float frameTime = curOrOver->time - prev->time;
+  const float ratio = glm::clamp((time - prev->time) / frameTime, 0.0f, 1.0f);
+
+  // 比率によって補間した値を返す
+  // 今は常に線形補間をしているが、本来は補間方法によって処理を分けるべき
+  return glm::mix(prev->value, curOrOver->value, ratio);
+}

 // バインディングポイント
 enum BindingPoint
```

チャネルは`glm::vec3`または`glm::quat`の2パターンがありうるため、関数テンプレートとして定義しています。

`lower_bound`(ローワー・バウンド)関数は「ソート済みの範囲」に対して使える関数で、「指定した値以上の値を持つ、最初の要素の位置」を返します。

<pre class="tnmai_code"><strong>【書式】</strong><code>
iterator lower_bound(検索範囲の先頭, 検索範囲の終端, 検索する値, 比較関数);
</code></pre>

上記のプログラムでは、比較関数に`keyframe.time < time`を返すラムダ式を指定することで、時刻による検索を行っています。

`lower_bound`はソート済み範囲を非常に高速に検索することができます(ソートされていない場合は`find_if`を使います)。キーフレームは時刻順になっているので、`lower_bound`で検索することができるのです。

検索の結果、見つかったキーフレームが先頭、または末尾より後ろのキーフレームだった、つまり`time`がチャネルの範囲外だった場合は、先頭または末尾の値をそのまま返します。

チャネルの範囲内だった場合は補間処理を行います。本来はチャネルの`interpolation`変数によって補間方法を変える必要がありますが、今回は手を抜いて全て線形補間を使っています。

>スケルタルアニメーションでは、線形補間以外が設定されることはまずありません。ですので、これは特に問題にはならないと考えています。

線形補間を行うには`mix`(ミックス)関数を使います。

<pre class="tnmai_code"><strong>【書式】</strong><code>
補間後の値 mix(補間前の値A, 補間前の値B, AとBの合成比率);
</code></pre>

>型によって`mix`関数の動作が異なることには注意が必要かもしれません。通常は線形補間が使われますが、クォータニオンに限り球面線形補間が使われます。

### 2.8 グローバル姿勢行列を作成する関数を定義する

続いて`CalcGlobalNodeMatrix`関数を定義します。`Interpolate`関数の定義の下に、次のプログラムを追加してください。

```diff
   // 比率によって補間した値を返す
   return glm::mix(prev->value, curOrOver->value, ratio);
 }
+
+/**
+* アニメーション計算用の中間データ型
+*/
+struct NodeMatrix
+{
+  glm::mat4 m = glm::mat4(1); // 姿勢行列
+  bool isCalculated = false;  // 計算済みフラグ
+};
+using NodeMatrices = std::vector<NodeMatrix>;
+
+/**
+* ノードのグローバル姿勢行列を計算する
+*/
+const glm::mat4& CalcGlobalNodeMatrix(const std::vector<GltfNode>& nodes,
+  const GltfNode& node, NodeMatrices& matrices)
+{
+  const intptr_t currentNodeId = &node - &nodes[0];
+  NodeMatrix& nodeMatrix = matrices[currentNodeId];
+
+  // 「計算済み」の場合は自分の姿勢行列を返す
+  if (nodeMatrix.isCalculated) {
+    return nodeMatrix.m;
+  }
+
+  // 「計算済みでない」場合、親の姿勢行列を合成する
+  if (node.parent) {
+    // 親の行列を取得(再帰呼び出し)
+    const glm::mat4& matParent =
+      CalcGlobalNodeMatrix(nodes, *node.parent, matrices);
+
+    // 親の姿勢行列を合成
+    nodeMatrix.m = matParent * nodeMatrix.m;
+  }
+
+  // 「計算済み」にする
+  nodeMatrix.isCalculated = true;
+
+  // 自分の姿勢行列を返す
+  return nodeMatrix.m;
+}

 // バインディングポイント
 enum BindingPoint
```

この関数は、自分自身を再帰的に呼び出すことで、自分自身とすべての親の「ローカル姿勢行列」を合成した「グローバル姿勢行列」を作成します。

`isCalculated`(イズ・カルキュレーテッド)変数には、ノードのグローバル姿勢行列の計算が完了していれば`true`、まだなら`false`が設定されます。

計算のためにノードの親を辿っていくと、どこかで他のノードと同じ親に到達します。
`isCalculated`がない場合、たとえ別のノードが同じ親ノードの姿勢行列を計算したことがあったとしても、もう一度同じ計算を行う必要があります。

行列の計算には時間がかかるため、これは相当な時間の無駄になります。そこで、「既に別のノードによって計算が終わっている場合は計算済みの結果を返す」とすることで計算時間を短縮しています。

### 2.9 アニメーション行列を計算する関数を定義する

必要な関数を準備することができたので、ようやく`CalcAnimationMatrices`関数を定義することができます。

ちょっと長いので、少しずつ書いていきましょう。無名名前空間の定義の下に、次のプログラムを追加してください。

```diff
   glm::vec4 weights = glm::vec4(0, 0, 0, 0);
 };

 } // unnamed namespace
+
+/**
+* アニメーションを適用した姿勢行列を計算する
+*
+* @param file             meshNodeを所有するファイルオブジェクト
+* @param meshNode         メッシュを持つノード
+* @param animation        計算の元になるアニメーション
+* @param nonAnimatedNodes アニメーションしないノードIDの配列
+* @param time             アニメーションの再生位置
+*
+* @return アニメーションを適用した姿勢行列の配列
+*/
+GltfAnimationMatrices CalcAnimationMatrices(const GltfFilePtr& file,
+  const GltfNode* meshNode, const GltfAnimation* animation,
+  const std::vector<int>& nonAnimatedNodes, float time)
+{
+  GltfAnimationMatrices matBones;
+  if (!file || !meshNode) {
+    return matBones;
+  }
+
+  // アニメーションが設定されていない場合...
+  if (!animation) {
+    return matBones;
+  }
+
+  // アニメーションが設定されている場合...
+
+  NodeMatrices matrices;
+  const auto& nodes = file->nodes;
+  matrices.resize(nodes.size());
+
+  return matBones;
+}

 /**
 * コンストラクタ
```

アニメーション可能なメッシュといっても、常にアニメーションが設定されているとは限りません。そのため、アニメーションの設定状態によって処理を分ける必要があります。

まずは、簡単なほうの「アニメーションが設定されていない場合」を書きましょう。アニメーションが設定されていない場合の`if`文の中に、次のプログラムを追加してください。

```diff
   // アニメーションが設定されていない場合...
   if (!animation) {
+    // ノードのグローバル座標変換行列を使う
+    size_t size = 1;
+    if (meshNode->skin >= 0) {
+      size = file->skins[meshNode->skin].joints.size();
+    }
+    matBones.resize(size, meshNode->matGlobal);
     return matBones;
   }

   // アニメーションが設定されている場合...
```

アニメーションが設定されていない場合は、ボーンの姿勢行列として、glTFファイルを読み込んだときに作成した「グローバル座標変換行列」を使います。メッシュにスキンIDが設定されている場合はジョイント数、なければ1つだけコピーします。

ジョイントがない場合でも1つはコピーを行う理由は、アニメーション用シェーダではSSBOに設定した行列がモデル行列を兼ねており、他にモデル行列を受け取る方法がないためです。

### 2.10 ローカル姿勢行列を設定する

次に、「アニメーションが設定されている場合」を書いていきます。これは以下の4段階で行います。

>1. アニメーションしないノードのローカル姿勢行列を設定
>2. アニメーションするノードのローカル姿勢行列を計算
>3. グローバル姿勢行列を計算
>4. 姿勢行列と逆バインドポーズ行列を合成

最初に、アニメーションしないノードのローカル姿勢行列を設定します。`matrices`(マトリシーズ)変数をリサイズするプログラムの下に、次のプログラムを追加してください。

```diff
   NodeMatrices matrices;
   const auto& nodes = file->nodes;
   matrices.resize(nodes.size());
+
+  // アニメーションしないノードのローカル姿勢行列を設定
+  for (const auto e : nonAnimatedNodes) {
+    matrices[e].m = nodes[e].matLocal;
+  }

   return matBones;
 }
```

アニメーションしないノードには、glTFファイルを読み込んだときに作成した「ローカル座標変換行列」を設定しておきます(グローバルではありません)。

ここで、グローバル座標変換行列ではなくローカル座標変換行列を使う理由は、アニメーションを適用した結果、親ノードの座標変換行列が変化する可能性があるからです。

どのノードが変化するか分からないので、とりあえずローカル行列を設定しておき、アニメーションの適用が完了したあとで、ローカル行列からグローバル行列を計算します。

次に、アニメーションするノードのローカル姿勢行列を計算します。アニメーションしないノードのローカル姿勢行列を設定するプログラムの下に、次のプログラムを追加してください。

```diff
   for (const auto e : nonAnimatedNodes) {
     matrices[e].m = nodes[e].matLocal;
   }
+
+  // アニメーションするノードのローカル姿勢行列を計算
+  // (拡大縮小→回転→平行移動の順で適用)
+  for (const auto& e : animation->translations) {
+    NodeMatrix& nodeMatrix = matrices[e.targetNodeId];
+    const glm::vec3 translation = Interpolate(e, time);
+    nodeMatrix.m *= glm::translate(glm::mat4(1), translation);
+  }
+  for (const auto& e : animation->rotations) {
+    NodeMatrix& nodeMatrix = matrices[e.targetNodeId];
+    const glm::quat rotation = Interpolate(e, time);
+    nodeMatrix.m *= glm::mat4_cast(rotation);
+  }
+  for (const auto& e : animation->scales) {
+    NodeMatrix& nodeMatrix = matrices[e.targetNodeId];
+    const glm::vec3 scale = Interpolate(e, time);
+    nodeMatrix.m *= glm::scale(glm::mat4(1), scale);
+  }

   return matBones;
 }
```

このプログラムは、`translations`、`rotations`、`scales`の3つのチャネル配列について、`Interpolate`関数によって補間した値をローカル姿勢行列に合成します。

3つのチャネルを実行する順序に注意してください。`glm::mat4`の`*=`演算子は次のように解釈されます。

`A *= B → A = A * B`

そのため、平行移動→回転→拡大縮小の順番で書くと、次のように書いた場合と同じになります。

`m = ((m * T) * R) * S → m = m * T * R * S`

そして、座標変換の適用順は「右から左」なので、「拡大縮小→回転→平行移動」という適用順になります。

### 2.11 グローバル姿勢行列を計算する

続いて、グローバル姿勢行列を計算します。ローカル姿勢行列を計算するプログラムの下に、次のプログラムを追加してください。

```diff
     const glm::vec3 scale = Interpolate(e, time);
     nodeMatrix.m *= glm::scale(glm::mat4(1), scale);
   }
+
+  // アニメーションを適用したグローバル姿勢行列を計算
+  if (meshNode->skin >= 0) {
+    for (const auto& joint : file->skins[meshNode->skin].joints) {
+      CalcGlobalNodeMatrix(nodes, nodes[joint.nodeId], matrices);
+    }
+  } else {
+    // ジョイントがないのでメッシュノードだけ計算
+    CalcGlobalNodeMatrix(nodes, *meshNode, matrices);
+  }

   return matBones;
 }
```

グローバル姿勢行列は全てのジョイントについて計算する必要があります。ジョイントと親子関係にないノードは描画に影響しないため、計算の必要はありません。

また、スキンが設定されていない(=ジョイントがない)場合はメッシュノードだけを計算します。ジョイントの計算でメッシュノードを入れていない理由は、ジョイントと親子関係にあるなら自動的に計算され、なければ描画に影響しないからです。

glTFのアニメーションデータは、座標変換行列とバインドポーズ行列を合成した結果になるように作成されるルールになっています。そのため、この時点で「ローカル座標変換行列→バインドポーズ行列」の合成までが完了したことになります。

### 2.12 逆バインドポーズ行列を合成する

残る作業は「逆バインドポーズ行列を合成する」ことだけです。それでは「逆バインドポーズ行列の合成」を行うプログラムを書いていきましょう。グローバル姿勢行列を計算するプログラムの下に、次のプログラムを追加してください。

```diff
     // ジョイントがないのでメッシュノードだけ計算
     CalcGlobalNodeMatrix(nodes, *meshNode, matrices);
   }
+
+  // 逆バインドポーズ行列を合成
+  if (meshNode->skin >= 0) {
+    // jointsにはノード番号が格納されているが、頂点データのJOINTS_n属性には
+    // ノード番号ではなく「joints配列のインデックス」が格納されている。
+    // つまり、姿勢行列をjoints配列の順番でSSBOに格納する必要がある。
+    matBones.resize(joints.size());
+    for (size_t i = 0; i < joints.size(); ++i) {
+      const auto& joint = joints[i];
+      matBones[i] = matrices[joint.nodeId].m * joint.matInverseBindPose;
+    }
+  } else {
+    // ジョイントがないので逆バインドポーズ行列も存在しない
+    const size_t nodeId = meshNode - &nodes[0];
+    matBones.resize(1, matrices[nodeId].m);
+  }

   return matBones;
 }
```

ここでも、スキンが設定されている(=ジョイントがある)かどうかで処理が分かれます。ジョイントがある場合、ジョイント配列の順序に従って「逆バインドポーズ行列を合成した姿勢行列」を格納していきます。

頂点アトリビュートのジョイント番号は「ジョイント配列のインデックス」だという点には注意が必要です。ノード番号で姿勢行列を設定してしまうと、メッシュが正しく表示されません。

当然のことですが、ジョイントがない場合は逆バインドポーズ行列も存在しません。そのため、メッシュノードの姿勢行列を格納するだけで十分です。

これで、アニメーション用の座標変換行列を作成できるようになりました。

>**【2章のまとめ】**
>
>* glTFのアニメーションデータは`channels`と`samplers`の2つで構成される。
>* チャネルは、アニメーションするノードとアニメーションデータを持つサンプラによって定義される。
>* サンプラには、キーフレームデータと値データの場所が書かれれている。
>* キーフレームの補間には、基本的には線形補間を使う。
>* glTFの場合、アニメーションするノードとしないノードが混ざっていて、アニメーションの有無によって扱い方を変える必要がある。

<div style="page-break-after: always"></div>

## 3. 姿勢行列の管理

### 3.1 SSBOを追加する

スケルタルアニメーションの表示は次の3つの手順で行います。

>1. \[CPU\] ボーンの<ruby>姿勢<rt>しせい</rt></ruby>を表す行列を計算。
>2. \[CPU\] ボーンの姿勢を表す行列をGPUメモリにコピー。
>3. \[シェーダ\] 姿勢を表す行列を使って頂点座標を変換。

姿勢を表す行列(姿勢行列)の数はボーン数に比例するので、モデルによってはかなりの数になる可能性があります。数が多すぎてユニフォーム変数では扱いきれないので、`SSBO`を使うことにします。

まずSSBOを使うための先行宣言を行います。`GltfMesh.h`を開き、次の先行宣言を追加してください。

```diff
 using TexturePtr = std::shared_ptr<Texture>;
 class VertexArrayObject;
 using VertexArrayObjectPtr = std::shared_ptr<VertexArrayObject>;
+class ShaderStorageBuffer;
+using ShaderStorageBufferPtr = std::shared_ptr<ShaderStorageBuffer>;

 /**
 * アニメーションの補間方法
```

続いて、`GltfFileBuffer`クラスの定義に次のプログラムを追加してください。

```diff
 class GltfFileBuffer
 {
 public:
-  explicit GltfFileBuffer(size_t maxBufferSize);
+  GltfFileBuffer(size_t maxBufferSize, size_t maxMatrixSize);
   ~GltfFileBuffer();

   // コピーを禁止
   GltfFileBuffer(const GltfFileBuffer&) = delete;
   GltfFileBuffer& operator=(const GltfFileBuffer&) = delete;

   bool AddFromFile(const char* filename);
   GltfFilePtr GetFile(const char* filename) const;
+
+  // アニメーションの姿勢行列バッファの管理
+  using AnimationMatrices = std::vector<glm::mat4>;
+  void ClearAnimationBuffer();
+  GLintptr AddAnimationMatrices(const AnimationMatrices& matBones);
+  void UploadAnimationBuffer();
+  void BindAnimationBuffer(
+    GLuint bindingPoint, GLintptr offset, GLsizeiptr size);
+  void UnbindAnimationBuffer(GLuint bindingPoint);

 private:
   // glTF用のバッファオブジェクト
   GLuint buffer = 0;
   GLsizei maxBufferSize = 0;
   GLsizei curBufferSize = 0;
  
   // メッシュファイル管理用の連想配列
   std::unordered_map<std::string, GltfFilePtr> files;
+
+  // アニメーションの姿勢行列バッファ
+  ShaderStorageBufferPtr ssbo;
+  AnimationMatrices matrixBuffer;
 };
 using GltfFileBufferPtr = std::shared_ptr<GltfFileBuffer>;
```

`AnimationMatrices`(アニメーション・マトリシーズ)は「姿勢行列の配列」を表す型です。
`matrixBuffer`(マトリックス・バッファ)変数は、GPUメモリにコピーする姿勢行列を一時的に格納するためのバッファです。

上記プログラムで追加したメンバ関数は、以下の手順で使用することを想定しています。

| 順序 | メンバ関数名 | 説明 |
|:-:|:--|:--|
| 1 | <ruby>ClearAnimationBuffer<rt>クリア・アニメーション・バッファ</rt></ruby> | フレーム開始時に`matrixBuffer`をクリア。 |
| 2 | <ruby>AddAnimationMatrices<rt>アド・アニメーション・マトリシーズ</rt></ruby> | アクターごとに現在フレームの姿勢行列を計算して<br>`matrixBuffer`に格納。 |
| 3 | <ruby>UploadAnimationBuffer<rt>アップロード・アニメーション・バッファ</rt></ruby> | 全ての姿勢行列の格納が終わったら、`matrixBuffer`をSSBOにコピー。 |
| 4 | <ruby>BindAnimationBuffer<rt>バインド・アニメーション・バッファ</rt></ruby> | 描画するアクターの姿勢行列が格納されたSSBO領域をバインド(`AddAnimationMatrices`の戻り値を使用)。 |
| 5 | <ruby>UnbindAnimationBuffer<rt>アンバインド・アニメーション・バッファ</rt></ruby> | 全てのアクターを描画したら、SSBO領域のバインドを解除。 |

<br>

アニメーションは毎フレーム更新されるので、当然SSBOも毎フレーム更新することになります。さらに、アニメーションするメッシュが複数存在するとき、それらのメッシュの姿勢行列はすべてGPUメモリへコピーしておく必要があります。

しかし、GPUメモリへのコピーは時間のかかる処理なので、フレーム中に何度も実行したくありません。そこで、まず全てのメッシュの姿勢行列を`matrixBuffer`に格納します。

そして、描画を行う前に`matrixBuffer`の全ての内容をGPUメモリにコピーします。この方法はCPU側のメモリを必要としますが、かわりにGPUメモリへのコピーを毎フレーム1回で済ませられるようになります。

### 3.2 SSBOを初期化する

それでは、姿勢行列用のSSBOと`matrixBuffer`を扱うメンバ関数を定義していきましょう。まず`GltfMesh.cpp`を開き、`ShaderStorageBuffer.h`をインクルードしてください。

```diff
 * @file GltfMesh.cpp
 */
 #include "GltfMesh.h"
 #include "VertexArrayObject.h"
+#include "ShaderStorageBuffer.h"
 #include "GLContext.h"
 #include "GameEngine.h"
```

次に、`GltfFileBuffer`コンストラクタの定義を次のように変更してください。

```diff
 * コンストラクタ
 *
 * @param maxBufferSize メッシュ格納用バッファの最大バイト数
+* @param maxMatrixSize アニメーション用SSBOに格納できる最大行列数
 */
-GltfFileBuffer::GltfFileBuffer(size_t maxBufferSize)
+GltfFileBuffer::GltfFileBuffer(size_t maxBufferSize, size_t maxMatrixSize)
 {
   const GLsizei defaultDataSize = static_cast<GLsizei>(sizeof(DefaultVertexData));

   this->maxBufferSize = static_cast<GLsizei>(maxBufferSize + defaultDataSize);
   buffer = GLContext::CreateBuffer(this->maxBufferSize, nullptr);

   // バッファの先頭にダミーデータを設定
   const DefaultVertexData defaultData;
   CopyData(buffer, 1, 0, defaultDataSize, &defaultData);
   curBufferSize = defaultDataSize;
+
+  // アニメーションの姿勢行列用バッファを作成
+  ssbo = std::make_shared<ShaderStorageBuffer>(maxMatrixSize * sizeof(glm::mat4));
+  matrixBuffer.reserve(maxMatrixSize);
 }

 /**
 * デストラクタ
```

### 3.3 ClearAnimationBuffer関数を定義する

次に`ClearAnimationBuffer`(クリア・アニメーション・バッファ)関数を定義します。
`GetFile`メンバ関数の定義の下に、次のプログラムを追加してください。

```diff
+
+/**
+* アニメーションメッシュの描画用データをすべて削除
+*
+* フレームの最初に呼び出すこと。
+*/
+void GltfFileBuffer::ClearAnimationBuffer()
+{
+  matrixBuffer.clear();
+}
```

この関数は`matrixBuffer`をクリアするだけの簡単な関数です。しかし、この関数をフレームの最初に呼び出さない限り、以降の関数を適切に運用することはできません。このように、「関数の長さ」と「関数の重要性」は無関係です。

### 3.4 AddAnimationMatrices関数を定義する

続いて`AddAnimationMatrices`(アド・アニメーション・マトリシーズ)関数を定義します。
`ClearAnimationBuffer`関数の定義の下に、次のプログラムを追加してください。

```diff
 {
   matrixBuffer.clear();
 }
+
+/**
+* アニメーションメッシュの描画用データを追加
+*
+* @return BindAnimationBufferの引数として使用するオフセット
+*/
+GLintptr GltfFileBuffer::AddAnimationMatrices(const AnimationMatrices& matBones)
+{
+  GLintptr offset = static_cast<GLintptr>(matrixBuffer.size() * sizeof(glm::mat4));
+  matrixBuffer.insert(matrixBuffer.end(), matBones.begin(), matBones.end());
+
+  // SSBOのオフセットアライメント条件を満たすために、256バイト境界(mat4の4個分)に配置する。
+  // 256はOpenGL仕様で許される最大値。
+  matrixBuffer.resize(((matrixBuffer.size() + 3) / 4) * 4);
+  return offset;
+}
```

`offset`(オフセット)変数は、行列のコピー先バイトオフセットを示します。この値は、
`BindAnimationBuffer`関数の`offset`引数として使うことを想定しています。

OpenGLの仕様では、バインド時に指定するオフセットは特定の境界値になっていることを要求します。境界値はバインドするバッファによって異なりますが、SSBOの場合は最大でも256バイトであることが保証されています。

つまり、オフセットが常に256バイト境界になるようにすれば、どのようなGPUやドライバであっても意図したとおりに動作すると期待できるわけです。`mat4`は64バイトなので、`mat4`4つ単位でオフセットを設定しています。

>**【OpenGLの仕様ってどこにあるの？】**<br>
>SSBOの境界値は`GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT`というパラメータで取得できます。また、このパラメータで取得できるオフセット境界値が最大256であることは、以下のOpenGL 4.5 仕様書の表23.64に書かれています。<br>
>`https://www.khronos.org/registry/OpenGL/specs/gl/glspec45.core.pdf`

### 3.5 UploadAnimationBuffer関数を定義する

次に`UploadAnimationBuffer`(アップロード・アニメーション・バッファ)関数を定義します。`AddAnimationMatrices`関数の定義の下に、次のプログラムを追加してください。

```diff
   matrixBuffer.resize(((matrixBuffer.size() + 3) / 4) * 4);
   return offset;
 }
+
+/**
+* アニメーションメッシュの描画用データをGPUメモリにコピー
+*/
+void GltfFileBuffer::UploadAnimationBuffer()
+{
+  ssbo->BufferSubData(0, matrixBuffer.size() * sizeof(glm::mat4), matrixBuffer.data());
+  ssbo->SwapBuffers();
+}
```

この関数を実行することで、`matrixBuffer`に蓄積された姿勢行列がGPUメモリにコピーされます。

### 3.6 BindAnimationBuffer関数を定義する

次に`BindAnimationBuffer`(バインド・アニメーション・バッファ)関数を定義します。
`UploadAnimationBuffer`関数の定義の下に、次のプログラムを追加してください。

```diff
   ssbo->BufferSubData(0, matrixBuffer.size() * sizeof(glm::mat4), matrixBuffer.data());
   ssbo->SwapBuffers();
 }
+
+/**
+* アニメーションメッシュの描画に使うSSBO領域を割り当てる
+*
+* @param bindingPoint バインディングポイント
+* @param offset       バインドする範囲の先頭を示すバイトオフセット
+* @param size         バインドする範囲のバイト数
+*/
+void GltfFileBuffer::BindAnimationBuffer(
+  GLuint bindingPoint, GLintptr offset, GLsizeiptr size)
+{
+  if (size > 0) {
+    ssbo->Bind(bindingPoint, offset, size);
+  }
+}
```

### 3.7 UnbindAnimationBuffer関数を定義する

最後に`UnbindAnimationBuffer`(アンバインド・アニメーション・バッファ)関数を定義します。`BindAnimationBuffer`関数の定義の下に、次のプログラムを追加してください。

```diff
     ssbo->Bind(bindingPoint, offset, size);
   }
 }
+
+/**
+* アニメーションメッシュの描画に使うSSBO領域の割り当てを解除する
+*/
+void GltfFileBuffer::UnbindAnimationBuffer(GLuint bindingPoint)
+{
+  ssbo->Unbind(bindingPoint);
+}
```

これで、アニメーション用のSSBOを管理する機能は完成です。

<div style="page-break-after: always"></div>

### 3.8 ファイルバッファの初期化を修正する

`GltfFileBuffer`のコンストラクタを変更したので、コンストラクタを呼び出しているプログラムも修正しなくてはなりません。`GameEngine.cpp`を開き、`Initialize`関数の定義を次のように変更してください。

```diff
     }

     // glTFファイル用バッファを初期化
+    const size_t maxAnimeActorCount = 64;   // アニメーションするアクター数
+    const size_t maxAnimeMatrixCount = 256; // アクターのボーン数
-    engine->gltfFileBuffer = std::make_shared<GltfFileBuffer>(128 * 1024 * 1024);
+    engine->gltfFileBuffer = std::make_shared<GltfFileBuffer>(256 * 1024 * 1024,
+      sizeof(glm::mat4) * maxAnimeActorCount * maxAnimeMatrixCount);

     // ImGuiの初期化
     ImGui::CreateContext();
```

`maxAnimeActorCount`(マックス・アニメ・アクター・カウント)と`maxAnimeMatrixCount`
(マックス・アニメ・マトリックス・カウント)の数値は適当に決めました。

指先までしっかりボーンが設定されたメッシュの場合、100本程度のボーンは必要です。さらに髪の毛や服装などにもボーンを入れると200本を超えることもあります。そこで余裕を見て256本としています。

アクター数の64は、1体256ボーンとした場合にSSBOのサイズが1MBになる数値にしました。1MBに特に根拠はなくて、単にキリのいい数値というだけです。

### 3.9 アニメーション用バッファを消去する

次に`ClearAnimationBuffer`の呼び出しを追加します。フレームの最初に呼び出す必要があるので、`NewFrame`関数で呼び出すことにします。`NewFrame`関数の定義に次のプログラムを追加してください。

```diff
   ImGui_ImplOpenGL3_NewFrame();
   ImGui_ImplGlfw_NewFrame();
   ImGui::NewFrame();
+
+  // 描画に備えてアニメーションバッファをクリア
+  gltfFileBuffer->ClearAnimationBuffer();

   // 音声の更新
   Audio::Get().Update();
```

>**【無意味なコメントは書くべきでない。けれど…】**<br>
>`ClearAnimationBuffer`という名前の関数のコメントに「アニメーションバッファをクリア」と書くのは冗長で無駄なので(関数名から自明)、本来であれば避けるべきです。しかし、就職活動では無意味であってもコメントが多いほうが喜ばれたりします。

### 3.10 アニメーションバッファをGPUメモリにコピーする

続いて`UploadAnimationBuffer`の呼び出しを追加します。これは、フレームのアクターのアニメーション状態の計算が完了した後、アクターを描画するまでのあいだならどこでも構いません。

今回は`RenderDefault`関数の先頭で呼び出すことにしました。`RenderDefault`関数の定義に、次のプログラムを追加してください。

```diff
 void GameEngine::RenderDefault()
 {
+  // アニメーションバッファをGPUメモリにコピーする
+  gltfFileBuffer->UploadAnimationBuffer();
+
   // シェーダの切り替えによる描画効率の低下を防ぐため、アクターをシェーダ別に分ける
   std::vector<std::vector<Actor*>> shaderGroup;
```

### 3.11 アニメーション用シェーダを定義する

アニメーションを行うアクターのために、アニメーション用のシェーダとレンダラを追加する必要があります。まずシェーダを作成しましょう。プロジェクトの`Res`フォルダに
`AnimatedMesh.vert`という頂点シェーダファイルを追加してください。

追加したファイルを開き、次のプログラムを追加してください。

```diff
+#version 450
+
+// 入力変数
+layout(location=0) in vec3 vPosition;
+layout(location=1) in vec4 vColor;
+layout(location=2) in vec2 vTexcoord;
+layout(location=3) in vec3 vNormal;
+layout(location=4) in vec4 vWeights;
+layout(location=5) in vec4 vJoints;
+
+// 出力変数
+layout(location=0) out vec4 outColor;
+layout(location=1) out vec2 outTexcoord;
+layout(location=2) out vec3 outNormal;
+layout(location=3) out vec3 outPosition;
+
+out gl_PerVertex {
+  vec4 gl_Position;
+};
+
+// ユニフォーム変数
+layout(location=0) uniform mat4 matVP;
+layout(location=10) uniform vec4 materialColor;
+
+layout(std430, binding=0) buffer AnimationMatrices
+{
+  mat4 matBones[];
+};
+
+// 頂点シェーダプログラム
+void main()
+{
+  outColor = vColor * materialColor;
+  outTexcoord = vTexcoord;
+
+  // 最初のウェイトが0以外なら「ジョイントデータあり」、
+  // 0の場合は「ジョイントデータなし」とみなす。
+  mat4 matModel;
+  if (vWeights.x != 0) {
+    // ボーン行列にウェイトを掛けて加算合成
+    matModel =
+      matBones[int(vJoints.x)] * vWeights.x +
+      matBones[int(vJoints.y)] * vWeights.y +
+      matBones[int(vJoints.z)] * vWeights.z +
+      matBones[int(vJoints.w)] * vWeights.w;
+
+    // ウェイトが正規化されていない場合の対策([3][3]が1.0になるとは限らない)
+    matModel[3][3] = dot(vWeights, vec4(1));
+  } else {
+    matModel = matBones[0];
+  }
+
+  mat3 matNormal = transpose(inverse(mat3(matModel)));
+  outNormal = normalize(matNormal * vNormal);
+
+  outPosition = vec3(matModel * vec4(vPosition, 1.0));
+  gl_Position = matVP * vec4(outPosition, 1.0);
+}
```

スケルタルアニメーションを行う頂点シェーダはジョイントIDとウェイトが必要なるため、
`in`変数の種類が多くなります。

ジョイントIDは`matBones`配列の添字になります。プログラムを見ると分かるように、スケルタルアニメーションのポイントは、複数のボーン(姿勢)行列をウェイトによって加算合成するところです。

また、ユニフォーム変数からモデル行列が消えています。今回は、モデル行列をボーン行列にかけ合わせてからGPUメモリにコピーする方針だからです。

>モデル行列の扱いは実装方針によって異なります。本テキストのように事前にボーン行列に掛けることもあれば、`uniform`変数を使って加算合成後にシェーダで掛ける場合もあります。

スケルタルアニメーションの頂点シェーダとはいっても、モデル行列の作成方法以外は通常の頂点シェーダと同じです。

### 3.12 シェーダの種類を追加する

次にシェーダの種類を追加しましょう。`Actor.h`を開き、`Shader`列挙型の定義に次のプログラムを追加してください。

```diff
   FragmentLighting,
   InstancedMesh,
   StaticMesh,
+  AnimatedMesh,
   GroundMap,
 };
-static size_t shaderCount = 4; // シェーダの種類数
+static size_t shaderCount = 5; // シェーダの種類数

 /**
 * 衝突判定の種類
```

>**【シェーダの種類数について】**<br>
>`shaderCount`定数の値は、実際に定義している種類数に合わせること。

### 3.13 シェーダを読み込む

それではシェーダを読み込みましょう。`GameEngine.h`を開き、`GameEngine`クラス定義に次のプログラムを追加してください。

```diff
   std::shared_ptr<ProgramPipeline> pipelineDoF;
   std::shared_ptr<ProgramPipeline> pipelineInstancedMesh;
   std::shared_ptr<ProgramPipeline> pipelineStaticMesh;
+  std::shared_ptr<ProgramPipeline> pipelineAnimatedMesh;
   std::shared_ptr<Sampler> sampler;
   std::shared_ptr<Sampler> samplerUI;
```

変数名は`pipelineAnimatedMesh`(パイプライン・アニメーテッド・メッシュ)としました。

次に`GameEngine.cpp`を開き、`Initialize`関数の定義に次のプログラムを追加してください。

```diff
       "Res/InstancedMesh.vert", "Res/FragmentLighting.frag"));
     engine->pipelineStaticMesh.reset(new ProgramPipeline(
       "Res/StaticMesh.vert", "Res/StaticMesh.frag"));
+    engine->pipelineAnimatedMesh.reset(new ProgramPipeline(
+      "Res/AnimatedMesh.vert", "Res/StaticMesh.frag"));

     engine->sampler = std::shared_ptr<Sampler>(new Sampler(GL_REPEAT));
     engine->samplerUI.reset(new Sampler(GL_CLAMP_TO_EDGE));
```

スケルタルメッシュのフラグメントシェーダには、スタティックメッシュと同じシェーダを使うことができます。

### 3.14 描画データリストにアニメーション用シェーダを追加する

追加したシェーダを使って描画するために、描画データリストを修正します。
`RenderDefault`関数にある、影を描画するプログラムを次のように変更してください。

```diff
       { Shader::FragmentLighting, pipeline.get() },
       { Shader::GroundMap, pipelineGround.get() },
       { Shader::StaticMesh, pipelineStaticMesh.get() },
+      { Shader::AnimatedMesh, pipelineAnimatedMesh.get() },
     };
     RenderShaderGroups(shaderGroup, renderingList, matShadowProj * matShadowView);
```

次に、アクターを描画するプログラムを次のように変更してください。

```diff
     { Shader::InstancedMesh, pipelineInstancedMesh.get() },
     { Shader::FragmentLighting, pipeline.get() },
     { Shader::StaticMesh, pipelineStaticMesh.get() },
+    { Shader::AnimatedMesh, pipelineAnimatedMesh.get() },
   };
   RenderShaderGroups(shaderGroup, renderingList, matProj * matView);
```

それから、`AnimatedMesh.vert`を実行するにはビュープロジェクション行列を設定する必要があります。`RenderShaderGroups`関数の定義を次のように変更してください。

```diff
     // 前処理
     if (e.shaderType == Shader::InstancedMesh ||
-      e.shaderType == Shader::StaticMesh) {
+      e.shaderType == Shader::StaticMesh ||
+      e.shaderType == Shader::AnimatedMesh) {
       e.pipeline->SetUniform(Renderer::locMatTRS, matVP);
     } else if (e.shaderType == Shader::GroundMap) {
       texMap->Bind(2);
```

これで、アニメーションデータの読み込みとシェーダの作成は完了です。

>**【3章のまとめ】**
>
>* アニメーション用の行列は数が多くてユニフォーム変数に格納しきれないので、SSBOなどを使う必要がある。
>* 複数のデータをSSBOにコピーする場合、コピー回数を減らすために、コピーするデータを貯めておくバッファを使うとよい。
>* スケルタルアニメーションのポイントは、複数のボーン(姿勢)行列をウェイトによって加算合成すること。ボーン行列は事前に計算してSSBOにコピーしておく。
>* 動かないメッシュと比較したとき、スケルタルアニメーションには大量の追加の計算が必要になる。動かないメッシュと同じように扱うことはできない。

<div style="page-break-after: always"></div>

## 4. アニメーション用レンダラの追加

### 4.1 RendererにUpdate関数を定義する

アニメーション用のレンダラクラスの役割は、アニメーションの状態を管理すること、姿勢行列をバッファに追加することの2つです。

ただ、現在の`Renderer`クラスには、「姿勢行列をバッファに追加する」のような、状態の更新に対応した仮想関数がありません。そこで、状態を更新するための仮想関数を追加することにします。

`Renderer.h`を開き、`Renderer`クラスの定義に次のプログラムを追加してください。

```diff
   Renderer() = default;
   virtual ~Renderer() = default;
   virtual RendererPtr Clone() const = 0;
+  virtual void Update(Actor& actor, float deltaTime) {}
+  virtual void PreDraw(const Actor& actor) {}
   virtual void Draw(const Actor& actor,
     const ProgramPipeline& pipeline, const glm::mat4& matVP) = 0;
```

`Update`(アップデート)仮想関数はレンダラの状態を更新します。`PreDraw`(プリ・ドロー)仮想関数は描画前に実行するべき処理(もしあれば)を実行します。

これらの仮想関数は必須ではなく、必要とするレンダラだけがオーバーライドすればよいので、純粋仮想にはしないことにしました。

次に、`Update`関数を呼び出す処理を追加します。これはゲームエンジンの役割になります。
`GameEngine.cpp`を開き、`UpdateActors`関数の定義に次のプログラムを追加してください。

```diff
         actors[i]->animation->Update(deltaTime);
       }

       actors[i]->OnUpdate(deltaTime);
+
+      // レンダラの状態を更新
+      if (actors[i]->renderer) {
+        actors[i]->renderer->Update(*actors[i], deltaTime);
+      }

       // 速度に重力加速度を加える
       if (!actors[i]->isStatic) {
```

続いて、`PreDraw`関数を呼び出す処理を追加します。`RenderDefault`関数の定義に次のプログラムを追加してください。

```diff
 void GameEngine::RenderDefault()
 {
+  // レンダラの前処理を実行
+  for (auto& e : actors[static_cast<int>(Layer::Default)]) {
+    if (e->renderer) {
+      e->renderer->PreDraw(*e);
+    }
+  }
+
   // アニメーションバッファをGPUメモリにコピーする
   gltfFileBuffer->UploadAnimationBuffer();
```

これで「レンダラの状態を更新する機能」を追加することができました。

### 4.2 AnimatedMeshRendererクラスを定義する

続いて、アニメーションを扱うレンダラクラスを定義します。しかし、その前に先行宣言を追加しておきます。`Renderer.h`を開き、先行宣言に次のプログラムを追加してください。

```diff
 using MeshRendererPtr = std::shared_ptr<MeshRenderer>;
 using InstancedMeshRendererPtr = std::shared_ptr<InstancedMeshRenderer>;
 using ActorPtr = std::shared_ptr<Actor>;

+struct GltfAnimation;
+using GltfAnimationPtr = std::shared_ptr<GltfAnimation>;
+struct GltfScene;
 struct GltfFile;
 using GltfFilePtr = std::shared_ptr<GltfFile>;
+class GltfFileBuffer;
+using GltfFileBufferPtr = std::shared_ptr<GltfFileBuffer>;

 /**
 * 描画機能の基本クラス
```

それでは、レンダラ派生クラスを定義しましょう。クラス名は`AnimatedMeshRenderer`(アニメーテッド・メッシュ・レンダラ)とします。`StaticMeshRendererPtr`型の定義の下に、次のプログラムを追加してください。

```diff
   int meshIndex = -1;
 };
 using StaticMeshRendererPtr = std::shared_ptr<StaticMeshRenderer>;
+
+/**
+* アニメーションするメッシュ描画クラス
+*/
+class AnimatedMeshRenderer : public Renderer
+{
+public:
+  // アニメーションの再生状態
+  enum class State {
+    stop,  // 停止中
+    play,  // 再生中
+    pause, // 一時停止中
+  };
+
+  AnimatedMeshRenderer() = default;
+  virtual ~AnimatedMeshRenderer() = default;
+  virtual RendererPtr Clone() const override;
+  virtual void Update(Actor& actor, float deltaTime) override;
+  virtual void PreDraw(const Actor& actor) override;
+  virtual void Draw(const Actor& actor,
+    const ProgramPipeline& pipeline,
+    const glm::mat4& matVP) override;
+
+  void SetFileBuffer(const GltfFileBufferPtr& p) { fileBuffer = p; }
+  void SetFile(const GltfFilePtr& f, int sceneNo = 0);
+  bool SetAnimation(const GltfAnimationPtr& animation, bool isLoop = true);
+  bool SetAnimation(const std::string& name, bool isLoop = true);
+  bool SetAnimation(size_t index, bool isLoop = true);
+  bool Play();
+  bool Stop();
+  bool Pause();
+
+  const GltfFilePtr& GetFile() const { return file; }
+  State GetState() const { return state; }
+  const GltfAnimationPtr& GetAnimation() const { return animation; }
+
+  // @note 再生速度(1.0f=等速, 2.0f=2倍速, 0.5f=1/2倍速)
+  void SetAnimationSpeed(float speed) { animationSpeed = speed; }
+  float GetAnimationSpeed() const { return animationSpeed; }
+
+  // @note true=ループする, false=ループしない
+  void SetLoopFlag(bool isLoop) { this->isLoop = isLoop; }
+  bool IsLoop() const { return isLoop; }
+
+  void SetTime(float);
+  float GetTime() const;
+  bool IsFinished() const;
+
+private:
+  GltfFileBufferPtr fileBuffer;
+  GltfFilePtr file;
+  const GltfScene* scene = nullptr;
+  GltfAnimationPtr animation;
+  std::vector<int> nonAnimatedNodes; // アニメーションしないノードIDの配列
+
+  struct Range {
+    GLintptr offset;
+    GLsizeiptr size;
+  };
+  std::vector<Range> ssboRanges;
+
+  State state = State::stop;
+  float time = 0;
+  float animationSpeed = 1;
+  bool isLoop = true;
+};
+using AnimatedMeshRendererPtr = std::shared_ptr<AnimatedMeshRenderer>;

 StaticMeshRendererPtr SetStaticMeshRenderer(
   Actor& actor, const char* filename, int index = 0);
```

`AnimatedMeshRenderer`クラスにはアニメーション制御用の関数と変数が必要で、さらに姿勢行列を扱うための変数もあるため、クラス定義はある程度複雑にならざるを得ません。

### 4.3 Clone関数を定義する

続いてメンバ関数を定義していきます。最初に必要なヘッダファイルをインクルードします。`Renderer.cpp`を開き、次のヘッダファイルをインクルードしてください。

```diff
 #include "GltfMesh.h"
 #include "VertexArrayObject.h"
 #include <glm/gtc/matrix_transform.hpp>
+#include <numeric>
+#include <algorithm>

 /**
 * クローンを作成する
```

それでは、`Clone`(クローン)関数から定義しましょう。`StaticMeshRenderer::Draw`関数の定義の下に、次のプログラムを追加してください。

```diff
   }
   glBindVertexArray(0);
 }
+
+/**
+* クローンを作成する
+*/
+RendererPtr AnimatedMeshRenderer::Clone() const
+{
+  return std::make_shared<AnimatedMeshRenderer>(*this);
+}

 /**
 * アクターにスタティックメッシュレンダラを設定する
```

### 4.4 Update関数を定義する

次に`Update`(アップデート)関数を定義します。`Clone`関数の定義の下に、次のプログラムを追加してください。

```diff
 {
   return std::make_shared<AnimatedMeshRenderer>(*this);
 }
+
+/**
+* アニメーション状態を更新する
+*
+* @param deltaTime 前回の更新からの経過時間
+* @param actor     描画対象のアクター
+*/
+void AnimatedMeshRenderer::Update(Actor& actor, float deltaTime)
+{
+  // 再生中なら再生時刻を更新
+  if (animation && state == State::play) {
+    time += deltaTime * animationSpeed;
+    if (isLoop) {
+      time -= animation->totalTime * std::floor(time / animation->totalTime);
+    } else {
+      time = std::clamp(time, 0.0f, animation->totalTime);
+    } // isLoop
+  }
+
+  // 状態を更新
+  if (animation) {
+    switch (state) {
+    case State::stop:
+      break;
+    case State::play:
+      if (!isLoop && (time >= animation->totalTime)) {
+        state = State::stop;
+      }
+      break;
+    case State::pause:
+      break;
+    }
+  } // animation
+}

 /**
 * アクターにスタティックメッシュレンダラを設定する
```

この関数は、アニメーションの時刻と状態の更新を行います。

ループ再生が指定されている場合、`time`が`totalTime`以上になったら再生位置を先頭に移動し、0未満になったら再生位置を末尾に移動します。

この処理には`floor`(フロア)関数を使っています。

<pre class="tnmai_code"><strong>【書式】</strong><code>
float floor(数値);
</code></pre>

`floor`関数は、「引数で指定した数値以下の最大の整数」を返します。例えば`3.4`を指定すると`3`、`-1.5`を指定すると`-2`が返されます。

この機能によって、`time`が0未満の場合は`-1`、`totalTime`以上の場合は`1`が返されるため、これを利用して再生時刻のループを実現しています。

また、ループ再生が無効の場合、`time`が`0`以下の場合は`0`、`totalTime`以上の場合は`totalTime`にする必要がありますが、これには`clamp`(クランプ)関数を使っています。

<pre class="tnmai_code"><strong>【書式】</strong><code>
float clamp(数値, 最小値, 最大値);
</code></pre>

### 4.5 PreDraw関数を定義する

続いて`PreDraw`(プリ・ドロー)関数を定義します。`Update`関数の定義の下に、次のプログラムを追加してください。

```diff
     }
   } // animation
 }
+
+/**
+* 描画の前処理を実行
+*/
+void AnimatedMeshRenderer::PreDraw(const Actor& actor)
+{
+  // 全メッシュのアニメーション行列を更新
+  const glm::mat4 matModel = actor.GetModelMatrix();
+  ssboRanges.clear();
+  for (const auto e : scene->meshNodes) {
+    // アニメーション行列を計算
+    auto matBones = CalcAnimationMatrices(
+      file, e, animation.get(), nonAnimatedNodes, time);
+
+    // アニメーション行列にモデル行列を合成
+    for (auto& m : matBones) {
+      m = matModel * m;
+    }
+
+    // アニメーション行列をバッファに追加し、追加先のオフセットとサイズを記録
+    const GLintptr offset = fileBuffer->AddAnimationMatrices(matBones);
+    const GLsizeiptr size =
+      static_cast<GLsizeiptr>(matBones.size() * sizeof(glm::mat4));
+    ssboRanges.push_back({ offset, size });
+  }
+}

 /**
 * アクターにスタティックメッシュレンダラを設定する
```

`PreDraw`関数は、全てのメッシュのアニメーション行列を更新し、バッファに追加します。追加に使う`AddAnimationMatrices`関数は、追加したデータの先頭オフセットを返してくれます。

先頭オフセットは`Draw`関数で描画するときに必要となります。そこで、データサイズとペアにして`ssboRanges`(エスエスビーオー・レンジズ)に記録しておきます。

### 4.6 Draw関数を定義する

次に`Draw`(ドロー)関数を定義します。`PreDraw`関数で記録した先頭オフセットとデータサイズはここで使います。`PreDraw`関数の定義の下に、次のプログラムを追加してください。

```diff
     ssboRanges.push_back({ offset, size });
   }
 }
+
+/**
+* スケルタルメッシュを描画する
+*/
+void AnimatedMeshRenderer::Draw(const Actor& actor,
+  const ProgramPipeline& pipeline, const glm::mat4& matVP)
+{
+  if (!file || !scene || ssboRanges.empty()) {
+    return;
+  }
+
+  // ノードに含まれる全てのメッシュを描画
+  for (size_t i = 0; i < scene->meshNodes.size(); ++i) {
+    const glm::uint meshNo = scene->meshNodes[i]->mesh;
+    const GltfMesh& meshData = file->meshes[meshNo];
+
+    // SSBOをバインド
+    fileBuffer->BindAnimationBuffer(0, ssboRanges[i].offset, ssboRanges[i].size);
+
+    // メッシュに含まれる全てのプリミティブを描画
+    for (const auto& prim : meshData.primitives) {
+      const GltfMaterial& m = file->materials[prim.materialNo];
+      pipeline.SetUniform(locMaterialColor, m.baseColor);
+      if (m.texBaseColor) {
+        m.texBaseColor->Bind(0);
+      }
+      prim.vao->Bind();
+      glDrawElementsBaseVertex(prim.mode, prim.count, prim.type,
+        prim.indices, prim.baseVertex);
+    }
+  }
+
+  // SSBOとVAOのバインドを解除
+  fileBuffer->UnbindAnimationBuffer(0);
+  glBindVertexArray(0);
+}

 /**
 * アクターにスタティックメッシュレンダラを設定する
```

### 4.7 SetFile関数を定義する

次に`SetFile`(セット・ファイル)関数を定義します。`Draw`関数の定義の下に、次のプログラムを追加してください。

```diff
   fileBuffer->UnbindAnimationBuffer(0);
   glBindVertexArray(0);
 }
+
+/**
+* 表示するファイルを設定する
+*/
+void AnimatedMeshRenderer::SetFile(const GltfFilePtr& f, int sceneNo)
+{
+  file = f;
+  scene = &f->scenes[sceneNo];
+  animation = nullptr;
+  ssboRanges.clear();
+
+  state = State::stop;
+  time = 0;
+  animationSpeed = 1;
+  isLoop = true;
+}

 /**
 * アクターにスタティックメッシュレンダラを設定する
```

`SetFile`関数は描画するシーンを指定します。この関数でシーンを設定すると、アニメーションは解除されて初期状態に戻ります。

### 4.8 SetAnimation関数を定義する

次に、アニメーションを設定する関数を定義します。`SetFile`関数の定義の下に、次のプログラムを追加してください。

```diff
   animationSpeed = 1;
   isLoop = true;
 }
+
+/**
+* アニメーションを設定する
+*
+* @param animation 再生するアニメーション
+* @param isLoop    ループ再生の指定(true=ループする false=ループしない)
+*
+* @retval true  設定成功
+* @retval false 設定失敗
+*/
+bool AnimatedMeshRenderer::SetAnimation(const GltfAnimationPtr& animation, bool isLoop)
+{
+  // ファイルが設定されていなければ何もしない
+  if (!file) {
+    return false;
+  }
+
+  // 同じアニメーションが指定された場合は何もしない
+  if (this->animation == animation) {
+    return true;
+  }
+ 
+  // アニメーションを設定
+  this->animation = animation;
+
+  // アニメーションがnullptrの場合は再生状態をを「停止」にする
+  if (!animation) {
+    state = State::stop;
+    return false;
+  }
+
+  // アニメーションしないノードIDのリストを作る
+  {
+    const int withAnimation = -1; // 「アニメーションあり」を表す値
+
+    // 全ノード番号のリストを作成
+    const size_t size = file->nodes.size();
+    nonAnimatedNodes.resize(size);
+    std::iota(nonAnimatedNodes.begin(), nonAnimatedNodes.end(), 0);
+
+    // アニメーション対象のノード番号を「アニメーションあり」で置き換える
+    for (const auto& e : animation->scales) {
+      if (e.targetNodeId < size) {
+        nonAnimatedNodes[e.targetNodeId] = withAnimation;
+      }
+    }
+    for (const auto& e : animation->rotations) {
+      if (e.targetNodeId < size) {
+        nonAnimatedNodes[e.targetNodeId] = withAnimation;
+      }
+    }
+    for (const auto& e : animation->translations) {
+      if (e.targetNodeId < size) {
+        nonAnimatedNodes[e.targetNodeId] = withAnimation;
+      }
+    }
+
+    // 「アニメーションあり」をリストから削除する
+    const auto itr = std::remove(
+      nonAnimatedNodes.begin(), nonAnimatedNodes.end(), withAnimation);
+    nonAnimatedNodes.erase(itr, nonAnimatedNodes.end());
+  }
+
+  // 状態を「停止中」に設定
+  time = 0;
+  state = State::stop;
+  this->isLoop = isLoop;
+  return true;
+}

 /**
 * アクターにスタティックメッシュレンダラを設定する
```

この関数が行う主要な処理のひとつは、「アニメーションしないノードIDのリスト」を作ることです。このリストはアニメーションに必須ではなく、単に処理の高速化のために使われます。

「アニメーションしないノードIDのリスト」を作るプログラムは、以下の3段階の手順で作成しています。

>1. ノードIDの連番配列を作成。
>2. ノードID配列のうち、アニメーションするノードIDを特殊値`withAnimation`(ウィズ・アニメーション)に変更。
>3. ノードID配列からIDが`withAnimation`になっているノードを削除する。

連番を作成するには標準ライブラリの`iota`(イオタ)関数を使います。

<pre class="tnmai_code"><strong>【書式】</strong><code>
void iota(代入先範囲の先頭, 代入先範囲の終端, 初期値);
</code></pre>

`iota`関数は「連続した値のシーケンス(並び)」を作成します。今回は初期値として`0`を指定しているので、`0～(範囲数-1)`の数値が作成されることになります。

### 4.9 SetAnimation関数(名前指定版)を定義する

4.8節で定義した`SetAnimation`関数は、アニメーションを直接指定していました。しかし、必要になるたびに`GltfFile`からアニメーションを見つけ出す処理を書くのは面倒です。

そこで、アニメーション名でアニメーションを指定するバージョンの関数を定義しておきます。
`SetAnimation`関数の定義の下に、もうひとつの`SetAnimation`関数を追加してください。

```diff
   this->isLoop = isLoop;
   return true;
 }
+
+/**
+* アニメーションを設定する
+*
+* @param name   再生するアニメーションの名前
+* @param isLoop ループ再生の指定(true=ループする false=ループしない)
+*
+* @retval true  設定成功
+* @retval false 設定失敗
+*/
+bool AnimatedMeshRenderer::SetAnimation(const std::string& name, bool isLoop)
+{
+  if (!file) {
+    return false;
+  }
+
+  for (const auto& e : file->animations) {
+    if (e->name == name) {
+      return SetAnimation(e, isLoop);
+    }
+  }
+  return false;
+}

 /**
 * アクターにスタティックメッシュレンダラを設定する
```

この関数は、`file`変数に設定されたファイルから名前の一致するアニメーションを検索し、前に定義した`SetAnimation`関数を呼び出します。

### 4.10 SetAnimation関数(番号指定版)を定義する

アニメーションがひとつしかないglTFファイルなどの場合、いちいち名前を調べてプログラムに書くのは面倒です。そこで、番号でアニメーションを指定するバージョンの関数を定義しておきます。

名前で指定するバージョンの`SetAnimation`関数の定義の下に、次のプログラムを追加してください。

```diff
   }
   return false;
 }
+
+/**
+* アニメーションを設定する
+*
+* @param index  再生するアニメーション番号
+* @param isLoop ループ再生の指定(true=ループする false=ループしない)
+*
+* @retval true  設定成功
+* @retval false 設定失敗
+*/
+bool AnimatedMeshRenderer::SetAnimation(size_t index, bool isLoop)
+{
+  if (!file || index >= file->animations.size()) {
+    return false;
+  }
+  return SetAnimation(file->animations[index], isLoop);
+}

 /**
 * アクターにスタティックメッシュレンダラを設定する
```

番号の場合は検索の必要すらないので、非常に簡単な関数になっています。

### 4.11 Play関数を定義する

`Play`(プレイ)関数はアニメーションの再生を開始または再開します。<br>
番号指定版の`SetAnimation`関数の定義の下に、次のプログラムを追加してください。

```diff
   }
   return Play(file->animations[index], isLoop);
 }
+
+/**
+* アニメーションの再生を開始・再開する
+*
+* @retval true  成功
+* @retval false 失敗(アニメーションが設定されていない)
+*/
+bool AnimatedMeshRenderer::Play()
+{
+  if (animation) {
+    switch (state) {
+    case State::play:  return true;
+    case State::stop:  state = State::play; return true;
+    case State::pause: state = State::play; return true;
+    }
+  }
+  return false; // 再生失敗
+}

 /**
 * アクターにスタティックメッシュレンダラを設定する
```

### 4.12 Stop関数を定義する

続いて、アニメーションを停止させる関数を定義します。`Play`関数の定義の下に、次のプログラムを追加してください。

```diff
   }
   return false; // 再生失敗
 }
+
+/**
+* アニメーションの再生を停止する
+*
+* @retval true  成功
+* @retval false 失敗(アニメーションが設定されていない)
+*/
+bool AnimatedMeshRenderer::Stop()
+{
+  if (animation) {
+    switch (state) {
+    case State::play:  state = State::stop; return true;
+    case State::stop:  return true;
+    case State::pause: state = State::stop; return true;
+    }
+  }
+  return false;
+}

 /**
 * アクターにスタティックメッシュレンダラを設定する
```

「再生中」または「一時停止中」の場合に、状態を「停止」に変更します。

### 4.13 Pause関数を定義する

`Pause`(ポーズ)関数は再生を一時停止する関数です。`Stop`関数の定義の下に、次のプログラムを追加してください。

```diff
   }
   return false;
 }
+
+/**
+* アニメーションの再生を一時停止する
+*
+* @retval true  成功
+* @retval false 失敗(アニメーションが停止している、またはアニメーションが設定されていない)
+*/
+bool AnimatedMeshRenderer::Pause()
+{
+  if (animation) {
+    switch (state) {
+    case State::play:  state = State::pause; return true;
+    case State::stop:  return false;
+    case State::pause: return true;
+    }
+  }
+  return false;
+}

 /**
 * アクターにスタティックメッシュレンダラを設定する
```

この関数は、現在の状態が再生中の場合のみ、状態を「一時停止」に変更します。

### 4.14 SetTime関数を定義する

次に再生時刻を指定する`SetTime`(セット・タイム)関数を定義します。`Pause`関数の定義の下に、次のプログラムを追加してください。

```diff
   }
   return false;
 }
+
+/**
+* アニメーションの再生時刻を設定する
+*
+* @param t 再生時刻(秒)
+*/
+void AnimatedMeshRenderer::SetTime(float t)
+{
+  time = t;
+  if (animation) {
+    if (isLoop) {
+      time -= animation->totalTime * std::floor(time / animation->totalTime);
+    } else {
+      time = std::clamp(time, 0.0f, animation->totalTime);
+    }
+  } // animation
+}

 /**
 * アクターにスタティックメッシュレンダラを設定する
```

### 4.15 GetTime関数を定義する

続いて、再生時刻を取得する`GetTime`(ゲット・タイム)関数を定義します。`SetTime`関数の定義の下に、次のプログラムを追加してください。

```diff
     }
   } // animation
 }
+
+/**
+* アニメーションの再生時刻を取得する
+*
+* @return 再生時刻(秒)
+*/
+float AnimatedMeshRenderer::GetTime() const
+{
+  return time;
+}

 /**
 * アクターにスタティックメッシュレンダラを設定する
```

### 4.16 IsFinished関数を定義する

`IsFinished`(イズ・フィニッシュド)関数は、アニメーションの再生終了を調べる関数です。
`GetTime`関数の定義の下に、次のプログラムを追加してください。

```diff
 {
   return time;
 }
+
+/**
+* アニメーションの再生が終了しているか調べる
+*
+* @retval true  終了している
+* @retval false 終了していない、または一度もPlay()が実行されていない
+*
+* ループ再生中の場合、この関数は常にfalseを返すことに注意
+*/
+bool AnimatedMeshRenderer::IsFinished() const
+{
+  if (!file || !animation || isLoop) {
+    return false;
+  }
+
+  // 再生速度(方向)によって終了判定を変える
+  if (animationSpeed < 0) {
+    return time <= 0;
+  }
+  return time >= animation->totalTime;
+}

 /**
 * アクターにスタティックメッシュレンダラを設定する
```

終了とみなす条件は、再生速度の符号によって変化します。<br>
再生速度がマイナスの場合、`time`が`0`以下になっていたら再生終了とみなします。

再生速度がプラスの場合、`time`がアニメーションの`totalTime`以上になっていたら再生終了とみなします。

### 4.17 SetAnimatedMeshRenderer関数を定義する

最後に`SetAnimatedMeshRenderer`(セット・アニメーテッド・メッシュ・レンダラ)関数を定義します。`IsFinished`関数の定義の下に、次のプログラムを追加してください。

```diff
   actor.shader = Shader::StaticMesh;
   return renderer;
 }
+
+/**
+* アクターにアニメーションメッシュレンダラを設定する
+*
+* @param actor    レンダラを設定するアクター
+* @param filename glTFファイル名
+* @param sceneNo  描画するシーンの番号
+*/
+AnimatedMeshRendererPtr SetAnimatedMeshRenderer(
+  Actor& actor, const char* filename, int sceneNo)
+{
+  GameEngine& engine = GameEngine::Get();
+  auto renderer = std::make_shared<AnimatedMeshRenderer>();
+  GltfFilePtr p = engine.LoadGltfFile(filename);
+  renderer->SetFileBuffer(engine.GetGltfFileBuffer());
+  renderer->SetFile(p, sceneNo);
+  actor.renderer = renderer;
+  actor.shader = Shader::AnimatedMesh;
+  return renderer;
+}
```

これで`AnimatedMeshRenderer`クラスは完成です。

### 4.18 アニメーションするglTFファイルを表示する

それでは、アニメーションするglTFファイルを表示してみましょう。

<pre class="tnmai_assignment">
<strong>【課題01】</strong>
以下のURLでglTFの動作確認用モデルが公開されています。
<code>https://github.com/KhronosGroup/glTF-Sample-Models</code>
このサイトから<code>2.0/CesiumMan/glTF</code>にあるCesiumMan.glTF, CesiumMan_data.bin, CesiumMan_img0.pngを
ダウンロードし、プロジェクトの<code>Res</code>フォルダに保存しなさい。
画像ファイルCesiumMan_img0.pngはTGA形式に変換しておくこと。
</pre>

glTFファイルをダウンロードしたら、`GameManager.cpp`を開き、`Update`関数にある
glTFファイル表示テストプログラムの下に、次のプログラムを追加してください。

```diff
       actor->position = playerTank->position;
       engine.AddActor(actor);
     }
+    {
+      ActorPtr actor = std::make_shared<Actor>("Animated glTF Test");
+      auto renderer = SetAnimatedMeshRenderer(*actor, "Res/glTF/CesiumMan.gltf", 0);
+      renderer->SetAnimation(0);
+      renderer->Play();
+      actor->scale = glm::vec3(4);
+      actor->position = playerTank->position + glm::vec3(4, 0, -4);
+      engine.AddActor(actor);
+    }

     // 人間アクターを配置
     engine.AddActor(std::make_shared<HumanActor>(
```

プログラムが書けたらビルドして実行してください。自機の出現位置に「歩く白いヒューマノイド」が表示されていたら成功です。

<p align="center">
<img src="images/Tips_04_result_0.png" width="45%" />
</p>

>**【glTFファイルへの変換方法】**<br>
>OBjやFBXをglTFファイルに変換するにはBlender(ブレンダー)などのツールを使ってください。変換形式はglTFとbinが別々に出力される形式(Blenderの場合は`glTF Separate`)を選択してください。
>
>あるいは以下のURLで公開されているコマンドラインツールを使ってもよいでしょう。<br>
>`https://github.com/facebookincubator/FBX2glTF/releases`<br>
>`Assets`を展開すると実行ファイル一覧が表示されるので、Windows用実行ファイルをダウンロードしてください。使い方はトップページに書いてあります。

<div style="page-break-after: always"></div>

>**【4章のまとめ】**
>
>* 姿勢行列の作成回数を最小限に抑えるためには、オブジェクトを更新するときではなく、描画の直前に姿勢行列を作成するとよい。
>* 再生を制御する個々の機能は、状態を変更するだけの単純な関数として定義できる。
>* 3Dのアニメーションは再生時刻を制御することで行われる。

<pre class="tnmai_assignment">
<strong>【課題02】</strong>
お好みのスケルタルアニメーション付きのメッシュをダウンロード(または作成)し、glTF形式に変換して表示しなさい。
</pre>
