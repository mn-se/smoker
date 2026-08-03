# Smoker

ESP8266 または ESP32 (M5Stack Core2) を使用して、ネットワーク経由で対象デバイス（スマートプラグなど）のON/OFFを制御するプロジェクトです。

## 概要

このプロジェクトはPlatformIOで構築されています。指定されたWi-Fiネットワークに接続し、ターゲットとなるIPアドレス（例: `192.168.0.2`）のポート80に対してHTTP POSTリクエスト（JSON形式のペイロード）を送信することで、デバイスのトグル操作を行います。

## 環境構築

- **Platform**: Espressif 8266 / Espressif 32
- **Board**: ESP-WROOM-02 / M5Stack Core2
- **Framework**: Arduino
- **Development Environment**: PlatformIO

## セットアップ手順

1. リポジトリをクローンします。
2. PlatformIOがインストールされたVSCode等のエディタでプロジェクトを開きます。
3. include/secrets.h などの設定ファイル（Wi-FiのSSID、パスワード、ターゲットIPなど）を環境に合わせて変更してください。
4. MAX6675 の配線ピンを include/hardware_pins.h で確認・調整してください。
5. ボードに応じて環境を指定してビルド/書き込みを実行します。

## ドキュメント

- 日本語
  - [ユーザーマニュアル](doc/USER_MANUAL.md)
  - [ビルド・保守ガイド](doc/BUILD_GUIDE.md)
- English
  - [User Manual](doc/USER_MANUAL.en.md)
  - [Build Guide](doc/BUILD_GUIDE.en.md)

### ビルド例

- ESP8266: pio run -e esp_wroom_02
- M5Stack Core2: pio run -e m5stack-core2

### 書き込み例

- ESP8266: pio run -e esp_wroom_02 -t upload
- M5Stack Core2: pio run -e m5stack-core2 -t upload

## 依存ライブラリ
- ArduinoJson

## ライセンス

This project is open source and available under the [MIT License](LICENSE).
