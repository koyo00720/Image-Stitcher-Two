# Image Stitcher Two

複数画像の結合をアシストするツールです。  
Microsoft 製の Image Composite Editor の部分的代替となります。  
本ソフトウェアでは入力画像と出力画像のピクセルサイズが一致するため、
光学顕微鏡や電子顕微鏡画像といったピクセルサイズが重要な用途に最適です。

## 特徴
- 画像の変形は一切無し
- 回転移動無し、平行移動のみ許可
- 非重複部のRGB値は入力画像と出力画像で一致
- 2枚の画像重複部の出力画像RGB値は、重複-非重複境界線からのユークリッド距離に応じて入力画像から重みづけ
- 入力画像はAlphaありに対応。しかし、微妙なAlpha値は想定せず、0.5を閾値に2値化される

## 使用法
1. 繋げたい画像2枚を開く。
2. マウスで画像を操作し、画像同士を大体位置合わせする。
3. どちらかのCalc.を押す。  
   位相相関法の場合、2回以上押して画像が動かないことが望ましい。  
   SSIMの場合、厳密な位置合わせに適するが、探索範囲が広いほど計算負荷が高い。
5. 結合を押す。画像が1枚にまとめられる。
6. さらに画像を追加することができる。追加しない場合はPNGでExportする。

## 対応画像解像度
40000 x 60000 まで確認済み。これ以上も可能と思われる。

## ビルド
- Qt 6.10.2 (MinGW 64-bit)
- OpenCV 4.12.0
- CMake + Ninja (optional)

## Third-party libraries
This project uses:
- Qt — LGPL v3
- OpenCV — Apache License 2.0  
Each library is distributed under its own license.

## Contributing
This repository is not accepting pull requests.
Please do not submit contributions.
