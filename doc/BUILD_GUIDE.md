# Smoker Pro 開発・保守ガイド

このドキュメントでは、Smoker Pro のビルド環境の設定、プログラムの書き込み、およびメンテナンス方法について説明します。

---

## 1. 開発環境

本プロジェクトは **PlatformIO** を使用して開発されています。

*   **IDE**: Visual Studio Code + PlatformIO IDE 拡張機能
*   **Framework**: Arduino
*   **Platform**: espressif8266 / espressif32
*   **Board**:
    *   ESP-WROOM-02 (ESP8266)
    *   M5Stack Core2 (ESP32)

---

## 2. セットアップと機密情報の管理

セキュリティのため、Wi-Fiパスワードなどの機密情報は `include/secrets.h` に分離されています。このファイルは Git 管理から除外されています。

### `include/secrets.h` の作成
ビルド前に、`include/secrets.h` を以下の内容で作成してください：

```cpp
#ifndef SECRETS_H
#define SECRETS_H

#define SECRET_WIFI_SSID "あなたのSSID"
#define SECRET_WIFI_PASS "あなたのパスワード"
#define SECRET_MEROSS_UUID "MerossのUUID"
#define SECRET_MEROSS_KEY "MerossのKEY"
#define SECRET_MEROSS_IP "MerossのIP"

#endif
```

※ このファイルが存在しない場合、空文字としてコンパイルされ、デバイスは自動的に「セットアップモード（APモード）」で起動します。

### Merossプラグ情報の自動取得・上書き (オプション)
`secrets.h` に記載したデフォルト値を使わず、後から認証情報を更新したい場合は、Wi-Fi接続後に以下の付属スクリプトを実行してESP8266内の設定（EEPROM）を上書きできます。

```bash
python tools/meross_mss305.py <Merossの登録メールアドレス> <Merossのパスワード>
```

---

## 3. 書き込み手順

ESP8266 / ESP32 には「プログラム（バイナリ）」と「WebUIデータ（ファイルシステム）」の **2種類** を書き込む必要があります。

### 3.0 ビルド環境の選択

PlatformIO の env を明示して実行してください。

*   ESP8266: `esp_wroom_02`
*   M5Stack Core2: `m5stack-core2`

例:

```bash
pio run -e esp_wroom_02
pio run -e m5stack-core2
```

### 3.1 プログラムの書き込み (Firmware - シリアル接続)
ソースコード（`src/`）を変更した際に行います。（初回書き込み時）

*   **VSCode**: 下部ツールバーの `→`（PlatformIO: Upload）をクリック。
*   **CLI**:
    *   ESP8266: `pio run -e esp_wroom_02 -t upload`
    *   M5Stack Core2: `pio run -e m5stack-core2 -t upload`

### 3.2 無線での書き込み (OTA - Over-The-Air)
一度シリアルでファームウェアを書き込んだ後は、Wi-Fi経由での無線書き込み（OTA）が可能です。
`platformio.ini` の対象 env で以下の行を有効化します：

```ini
; OTAアップロード時は下の2行を有効化してupload_speedをコメントアウト
upload_protocol = espota
upload_port = smoker.local    ; または IPアドレスを指定
```
この状態で、対象 env を指定して Upload を実行すると無線で書き込まれます。

### 3.3 WebUIデータの書き込み (Filesystem Image)
`data/` フォルダ内のファイル（HTML, CSS, JS, manifest.json 等）を更新した際に行います。**通常のアップロードでは更新されないため注意してください。**

*   **VSCode**:
    1.  PlatformIO アイコンをクリック。
    2.  `PROJECT TASKS` > 対象env (`esp_wroom_02` または `m5stack-core2`) > `Platform` を開く。
    3.  **`Upload Filesystem Image`** を実行。
*   **CLI**:
    *   ESP8266: `pio run -e esp_wroom_02 -t uploadfs`
    *   M5Stack Core2: `pio run -e m5stack-core2 -t uploadfs`


---

## 4. パーティション構成

`platformio.ini` で以下の通り設定されています：
*   **ESP8266 (esp_wroom_02)**:
    *   Flash容量: 2MB
    *   ファイルシステム: LittleFS (約256KB)
    *   LD Script: `eagle.flash.2m256.ld`
*   **ESP32 (m5stack-core2)**:
    *   ボード定義の標準パーティションを利用

WebUIのファイルサイズやログの保存上限を変更する際は、LittleFSの残り容量に注意してください。

---

## 7. ボード別ピン定義

MAX6675 のピン定義は [include/hardware_pins.h](../include/hardware_pins.h) で管理します。

*   ESP8266: 既存配線 (D0/D5/D6)
*   M5Stack Core2: 外付け配線用の GPIO を使用

実機配線に合わせて、Core2 側の定数を編集してください。

---

## 5. デバッグとシリアルモニタ

動作確認やエラー診断には、シリアルモニタを使用します。

*   **通信速度**: 115200 bps
*   **主な確認項目**:
    *   起動時のWi-Fi接続状況
    *   Merossプラグへの通信成否
    *   MAX6675 センサーの読み取り値
    *   PID制御の計算値（ヒーターのON/OFFタイミング）

---

## 6. WebUIのキャッシュ制御について

本プロジェクトは PWA (Service Worker) を導入しているため、WebUIを更新してもブラウザに古い画面が残る場合があります。

ファイルを更新した後は、ブラウザ側で以下の操作を行ってください：
1. `Upload Filesystem Image` を完了させる。
2. スマホブラウザで `http://smoker.local` を開き、リフレッシュ。
3. 反映されない場合は、ブラウザ設定からサイトデータ（smoker.local）の削除・キャッシュのクリアを行ってください。
