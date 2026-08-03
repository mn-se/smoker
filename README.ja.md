# Smoker

Smoker は、M5Stack Core2 を中心にした Wi-Fi 連携型の温度制御プロジェクトです。内蔵ディスプレイに現在温度とヒーター状態を表示しながら、Meross スマートプラグを制御してスモーカーの温度を目標値に近づけます。

## 概要

Core2 版は、組み込みの Web ダッシュボード、Meross スマートプラグ制御、MAX6675 熱電対読み取り、PID ベースの温度制御を統合したシステムです。内蔵画面を使えば、温度と電源状態をその場で確認できます。

## ハードウェアの特徴

- **主なプラットフォーム**: M5Stack Core2 (ESP32)
- **表示**: 内蔵ディスプレイで温度とヒーター状態を表示
- **センサー**: MAX6675 熱電対
- **電気コンロ**: 石崎電機 製作所 電気コンロ ステンレス鋼 SK-65V 
- **制御対象**: Meross スマートプラグ
- **開発基盤**: Arduino / PlatformIO

## セットアップ

1. リポジトリをクローンします。
2. PlatformIO 拡張機能を入れた VS Code でプロジェクトを開きます。
3. include/secrets.h に Wi-Fi の SSID、パスワード、Meross 認証情報を設定します。
4. include/hardware_pins.h で MAX6675 の配線を確認します。
5. M5Stack Core2 用の環境でビルドおよび書き込みを行います。

## ドキュメント

- English
  - [User Manual](doc/USER_MANUAL.en.md)
  - [Build Guide](doc/BUILD_GUIDE.en.md)
- 日本語
  - [ユーザーマニュアル](doc/USER_MANUAL.md)
  - [ビルド・保守ガイド](doc/BUILD_GUIDE.md)

## ビルドと書き込み

- M5Stack Core2: pio run -e m5stack-core2
- M5Stack Core2 への書き込み: pio run -e m5stack-core2 -t upload

## 依存ライブラリ

- ArduinoJson
- M5Unified

## ライセンス

このプロジェクトはオープンソースで、[MIT License](LICENSE) のもとで提供されています。
