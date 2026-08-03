# Smoker Pro 開発・保守ガイド

このガイドでは、M5Stack Core2 上で動作する Smoker Pro のビルド手順、書き込み手順、保守方法を説明します。

---

## 1. 開発環境

本プロジェクトは PlatformIO を使って開発されています。

- IDE: Visual Studio Code + PlatformIO IDE 拡張機能
- Framework: Arduino
- Platform: Espressif 32
- Board: M5Stack Core2

---

## 2. セットアップと機密情報の管理

セキュリティのため、Wi-Fi の認証情報などの機密値は include/secrets.h に保存し、Git では管理しません。

### include/secrets.h の作成
ビルド前に、以下の内容で include/secrets.h を作成してください。

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

このファイルがない場合は空値が使われ、デバイスはセットアップモードで起動します。

### Meross 認証情報を後から更新する
後から認証情報を更新したい場合は、デバイスをネットワークに接続した状態で、付属スクリプトを実行して保存内容を書き換えます。

```bash
python tools/meross_mss305.py <Merossの登録メールアドレス> <Merossのパスワード>
```

---

## 3. 書き込み手順

ファームウェアと Web UI のファイルシステム画像の両方をデバイスに書き込む必要があります。

### 3.1 ファームウェアをビルドする
Core2 用の環境を明示して実行します。

```bash
pio run -e m5stack-core2
```

### 3.2 プログラム書き込み（シリアル接続）
初回書き込みやソースコード変更時に使用します。

- VS Code: PlatformIO の Upload ボタンを押します。
- CLI: pio run -e m5stack-core2 -t upload

### 3.3 無線書き込み（OTA）
一度シリアルで書き込んだ後は、Wi-Fi 経由で OTA 更新が可能です。Core2 用 env で次の設定を有効にします。

```ini
upload_protocol = espota
upload_port = smoker.local    ; または IP アドレス
```

その状態で Upload を実行すると無線更新できます。

### 3.4 Web UI のファイルシステムイメージをアップロードする
data/ 配下のファイルを更新した場合は、通常のファームウェア書き込みでは反映されないため、ファイルシステムイメージを別途アップロードします。

- VS Code:
  1. PlatformIO のタスクを開きます。
  2. m5stack-core2 環境を選びます。
  3. Upload Filesystem Image を実行します。
- CLI: pio run -e m5stack-core2 -t uploadfs

---

## 4. パーティション構成

Core2 環境では ESP32 ボード定義の標準パーティション構成を使用します。Web UI とログ保存に LittleFS 領域を使うため、容量を意識して管理してください。

---

## 5. ボード別ピン定義

MAX6675 のピン定義は include/hardware_pins.h で管理しています。実際の配線に合わせて Core2 側の定数を調整してください。

---

## 6. デバッグとシリアルモニタ

動作確認や不具合解析にはシリアルモニタを使います。

- 通信速度: 115200 bps
- 確認項目:
  - 起動時の Wi-Fi 接続状態
  - Meross プラグとの通信状態
  - MAX6675 センサーの読み取り値
  - PID 制御の計算結果とヒーターの ON/OFF タイミング

---

## 7. Web UI のキャッシュ制御

ブラウザ側に古い Web UI が残ることがあるため、更新後はキャッシュを確認してください。

1. Upload Filesystem Image を完了させます。
2. ブラウザで http://smoker.local を開き、リロードします。
3. 反映されない場合は、smoker.local のサイトデータやキャッシュを削除してください。
