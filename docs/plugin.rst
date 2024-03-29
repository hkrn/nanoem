==========================================
プラグイン
==========================================

.. important::
   プラグインを使用するには 29.2.0 以上が必要です。またプラグインの提供及び導入方法が今後変更される可能性があります。

導入方法 (macOS)
******************************************

* nanoem アプリを Finder から「パッケージの内容を表示」で開く
* ``Contents`` > ``Plugins`` に移動し ``Plugins`` フォルダにプラグインをコピー

  * 例えば「準標準ボーンプラグイン」の場合は ``plugin_ssb.dylib`` を ``Plugins`` フォルダにコピーする

* nanoem を再起動
* メニューから「編集」>「モデルプラグイン」に導入したいプラグインの名前が出ることを確認する

  * 例えば「準標準ボーンプラグイン」の場合「準標準ボーンプラグイン」が名前に出る

.. _CCDE11E1-3416-425D-80DF-A347F79E7BDD:

準標準ボーンプラグイン
******************************************

`「準標準ボーンプラグイン」(macOS 版) をダウンロード <_static/plugins/plugin_ssb-1.3.0.zip>`_

.. important::
   「準標準ボーンプラグイン」は配布モーションによって必須指定するほどの重要性の高いプラグインのため特例として移植して配布していますが、
   原則として PMDEditor/PMXEditor 系のプラグインの移植はいたしません。予めご了承ください。

   Windows 版の配布予定はありません（そもそも PMDEditor/PMXEditor が使える環境でありわざわざ nanoem 側のプラグインを使う理由がないため）。

   ``Semi-Standard Bone Plugin`` として英語表記に対応しています（移植元のオリジナルの方は英語表記に対応していません）。

.. warning::
   「準標準ボーンプラグイン」の利用はモデルの改造行為にあたるため、プラグイン利用前にモデルの利用規約を確認してください。
   ただしモデル配布の段階で準標準ボーンプラグインで作成されるボーンが最初から入ってることがありその場合は実行不要です。

.. note::
   「準標準ボーンプラグイン」含むモデルプラグインを実行すると元のモデルに対して上書き保存します。そのため、実行前のモデルデータのバックアップが作成されて対象モデルと同じフォルダに ``[モデルのファイル名]_backup_at_[年月日]_[時刻].pmx`` として書き出されます
   （例えばモデルのファイル名が「初音ミク」で実行時刻が「2020年7月7日12時39分51秒」の場合作られるファイル名が ``初音ミク_backup_at_20200707_123951.pmx`` になります）。

   実行元が PMD の場合は **必ずバックアップを取ってください**。内部の実装仕様上モデル読み込み時に PMX に変換するためです。

   モデルをプラグイン実行前のデータに元に戻したい場合はバックアップを元のモデルファイルにコピーしてください。なお、バックアップ作成を無効にすることはできません。

「準標準ボーンプラグイン」はそぼろさんが開発する PMDEditor/PMXEditor 用のプラグインである「準標準ボーンプラグイン」とほぼ同じ機能を実現する :ref:`7F24495C-52C6-4659-A309-0E75CAB72D3B` です。

`そぼろさんによる準標準ボーンプラグインの解説動画はこちら <https://www.nicovideo.jp/watch/sm14956092>`_

これは標準モデルである ``初音ミク.pmd`` に存在しないものの以下の理由からに広く使われるボーンを自動的に作成するものです（作成されるボーンがすでにある場合は処理をスキップします）。

- モーションを作りやすくするため
- モーションの動きの自由度を上げるため
- 配布されてるモーションに直接手を加えることなく補正できるようにするため

  - 回転付与のボーンは主にこの用途です

配布されるモーションによっては準標準ボーンプラグインで作られるボーンを必須とすることがあります（なくても実際には読み込めるもののモーション作成者が意図しない動きが発生する可能性がある）。

* ``*`` が後ろについてるものは頂点のボーン再配置及びウェイト塗り（以降自動再配置）を実行します
* ⚠️ が先頭についてる場合「必要なボーン」が足りてないため実行するとエラーが発生することを示しています

  * 上記によるエラーが発生した場合はモデルが変更されずそのままになります
  * 「必要なボーン」は「日本語」のボーン名で判定します
  * ボーン名の判定において全角英数字と半角英数字を区別します

腕捩れボーン *
==========================================

「右腕捩」と「左腕捩」を作成します。それぞれ「右腕」と「右ひじ」、「左腕」と「左ひじ」の間に作成されます。

また、捩ボーンを親とする回転付与率が 0.25/0.5/0.75 の３つの回転付与ボーンが作成されます。

ちなみに「捩れ」は「ねじれ」と読みます。

必要なボーン
------------------------------------------

* 右腕
* 右ひじ
* 左腕
* 左ひじ

回転自動補正
==========================================

「腕捩れボーン」作成時に腕にマッピングされている頂点を元に位置補正を行うかを指定します。

手捩れボーン *
==========================================

「右手捩」と「左手捩」を作成します。それぞれ「右ひじ」と「右手首」、「左ひじ」と「左手首」の間に作成されます。

また、捩ボーンを親とする回転付与率が 0.25/0.5/0.75 の３つの回転付与ボーンが作成されます。

ちなみに「捩れ」は「ねじれ」と読みます。

必要なボーン
------------------------------------------

* 右ひじ
* 右手首
* 左ひじ
* 左手首

上半身2ボーン *
==========================================

「上半身2ボーン」を作成します。これは「上半身」と「首」の間に作成されます。

上半身と首の間、すなわち胸部に位置するということもあって特に高い頻度で利用されるボーンであり、体の傾きを調整しやすくします。

必要なボーン
------------------------------------------

* 上半身
* 首

腰ボーン
==========================================

「腰ボーン」を作成します。これは「下半身」と「足」の間に作成されます。

また「右足」と「腰」の間に「腰キャンセル右」が、「左足」と「腰」の間に「腰キャンセル左」が作成されます。
名前が示す通り回転補正が -1.0 で行われる回転付与のボーンで、震えを制御するために使います。

必要なボーン
------------------------------------------

* 下半身
* 右足
* 左足

足IK親
==========================================

.. note::
   作成されるボーン「足IK親」の ``IK`` が全角ではなく半角英数字な点にご注意ください

「右足IK親」と「左足IK親」を作成します。これは「右足ＩＫ」と「左足ＩＫ」の親として作成されます。

必要なボーン
------------------------------------------

.. caution::
   全角英数字の方の「ＩＫ」です。半角英数字の「IK」ではないのでご注意ください

* 右足ＩＫ
* 左足ＩＫ

足先EX *
==========================================

「右足先EX」と「左足先EX」を作成します。加えて以下の回転付与のボーンが作成されます。

* 右足D
* 右ひざD
* 右足首D
* 左足D
* 左ひざD
* 左足首D

これらの回転付与のボーンは元のモーションに直接手を加えることなく追加の回転補正を行えるようにするために使われます。

必要なボーン
------------------------------------------

.. caution::
   全角英数字の方の「ＩＫ」です。半角英数字の「IK」ではないのでご注意ください

* 右足
* 右ひざ
* 右足首
* 右つま先ＩＫ
* 左足
* 左ひざ
* 左足首
* 左つま先ＩＫ

足Dボーンを操作可能に
==========================================

.. caution::
   「ボーン表示枠に自動登録」に✅を入れる必要があります

「足先EX」で作られる末尾がDのボーンをタイムラインの表示枠に追加にするかどうかを指定します。

手持ちアクセサリ用ダミー
==========================================

「右ダミー」と「左ダミー」を作成します。それぞれ「右手首」と「右中指１」、「左手首」と「左中指１」の間に作成されます。

手に持たせる形のアクセサリを外部親経由で持たせるために使います。

必要なボーン
------------------------------------------

.. caution::
   全角英数字の方の「１」です。半角英数字の「1」ではないのでご注意ください

* 右手首
* 右中指１
* 左手首
* 左中指１

肩キャンセルボーン
==========================================

以下のボーンを作成します。P は右肩または左肩の親として、C は右肩または左肩と右腕または左腕の間に入ります。

* 右肩P
* 右肩C
* 左肩P
* 左肩C

P は親を、C はキャンセルを意味し、C の方は付与率が -1.0 の回転付与ボーンとして作成されます。

必要なボーン
------------------------------------------

* 右肩
* 右腕
* 左肩
* 左腕

親指０ボーン *
==========================================

「右親指０」と「左親指０」ボーンを作成します。それぞれ「右手首」と「右親指１」、「左手首」と「左親指１」の間に作成されます。

必要なボーン
------------------------------------------

.. caution::
   全角英数字の方の「１」です。半角英数字の「1」ではないのでご注意ください

* 右手首
* 左手首
* 右親指１
* 左親指１

親指ローカル軸設定
==========================================

親指０ボーン作成時に各親指ボーンのローカル軸を追加で設定します。

グルーブボーン
==========================================

「グルーブ」ボーンを作成します。「センター」ボーンの直後に配置されます。

いわゆるボーンの多段化の一種であり、実質的に全ての動きを司るセンターボーンから上下の動きを分離するために使われます。

必要なボーン
------------------------------------------

* センター

全ての親
==========================================

「全ての親」ボーンを作成します。最上位に設定されます。

名前が示す通り全てのボーンの親として設定され、個々のボーンを直接編集することなくモデル自体の位置を補正するために使われます。
ただし、配布用に作る場合にかぎり先の理由で干渉を起こす可能性があることから利用を避けるべきです。

必要なボーンはありません。

操作中心
==========================================

「操作中心」ボーンを作成します。最上位に設定（優先度的には「全ての親」よりも上）されます。

特殊な扱いを受けるボーンで :ref:`6BECA538-F9C4-4628-88EB-7E99C046115F` でカメラの視点の中心に設定するために使われます
（モデルを選択したあとに「ビュー」パネルの「モデル」にチェックボックスを入れることで機能します）。
そのため、操作中心ボーンを動かしてもモデル自身は一切動きません。

必要なボーンはありません。

ボーン表示枠に自動登録
==========================================

ボーン作成時にタイムラインの表示枠に登録するかどうかを設定します。

以下のボーンは作成時に「依存するボーン」に表示枠がない場合は「センター」枠が自動的に作成され、その枠に移動します

.. csv-table::

   ボーン名,依存するボーン名
   腰,下半身
   グルーブ,センター
   全ての親,(最初のボーン)
   操作中心,(最初のボーン)

(材質選択)
==========================================

ウィンドウ右側では材質一覧が並んでおり、自動再配置の対象とする材質を選択することが可能です。
例えばモデルと一体化してるアクセサリなど自動再配置の対象にしたくない材質がある場合はチェックボックスから外してください。

.. note::
   頂点が複数の材質に紐づいている場合は自動再配置の除外設定を優先します

何も設定しない場合は全ての材質が自動再配置の対象となります。

変更履歴
==========================================

  * 1.3.0 (2021/2/14)

    * 実行後にクラッシュしやすくなる不具合を修正
    * 英語表記の文言を一部修正

  * 1.2.0 (2020/12/27)

    * Apple Silicon 対応のためユニバーサルバイナリ化

  * 1.1.0 (2020/8/1)

    * 必要なボーンが足りてない場合は先頭に⚠を出すようにした
    * 実行時に必要なボーンが足りてない場合はエラーを表示して処理を中止させるようにした

      * これらの機能はオリジナルには存在せず独自に追加した機能となります
      * 29.2 においてエラーメッセージが途中で途切れる問題があります

  * 1.0.0 (2020/7/20)

    * 29.2 と同時にリリース
