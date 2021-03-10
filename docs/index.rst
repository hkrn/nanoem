=======================================================
nanoem のマニュアル
=======================================================

.. important::
   nanoem itself has English-ready UI however it's manual is not translated and there is no plan to translate it due to high maintenance cost.
   If you want to read English manual, translate this manual with `Google Translate <https://translate.google.com/#ja/en/nanoem.readthedocs.io>`_ or `use translate webpages feature in Google Chrome <https://support.google.com/chrome/answer/173424>`_ (At least the minimum quality to understand this manual is gualantieed).

nanoem は `MikuMikuDance <https://sites.google.com/view/vpvp/>`_ (以下 MMD) で使われるモデルやモーションの読み込み、及び編集と保存が可能な MMD 互換実装のアプリケーションです。

nanoem では MMD との互換性をもつことと同時に以下を目標として開発しています。

- 移植性

  - 主目標である macOS 以外に Windows で動作
  - 未公開であるものの Linux / Raspberry Pi / Windows10 on ARM64 上でも動作 [#f1]_

- 立ち上がりが軽量であること

  - 1秒程度で立ち上がる [#f2]_

- サイズが小さいこと

  - 10MB 程度 [#f3]_

.. toctree::
   :maxdepth: 2
   :caption: 目次

   install.rst
   guide.rst
   application.rst
   menu.rst
   preference.rst
   effect.rst
   plugin.rst
   faq.rst
   faq_effect.rst
   trouble_shooting.rst
   privacy.rst
   change_log.rst
   known_bugs.rst
   alternative.rst
   license.rst
   architecture.rst

.. [#f1] もし必要な場合は :ref:`37420267-8E5A-41EA-A159-FFF490DF1D8D` でお問い合わせください
.. [#f2] MMD と同じようにというのはありますがエディタ感覚で使えることを目標としています
.. [#f3] macOS における圧縮時のサイズ。31.0 以降はユニバーサルバイナリ化のためおよそ倍のサイズになっています
