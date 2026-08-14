# Tools

このフォルダには、Smoker 本体の設定を補助するユーティリティを置いています。

## meross_mss305.py

Meross クラウドへログインし、対象デバイスの UUID / KEY / IP を取得して、Smoker 本体へ送信するスクリプトです。

### できること

- Meross クラウドへのログイン
- 利用可能デバイスの探索
- UUID / KEY / IP の取得
- Smoker 本体の /config/device へ自動送信

### 前提条件

- Python 3.9 以降
- PC と Smoker 本体が同じネットワークに接続されていること
- Smoker 本体が起動しており、Web API にアクセスできること

### 依存パッケージ

PowerShell 例:

python -m pip install meross_iot requests

### 使い方

PowerShell 例:

python tools/meross_mss305.py <Merossメールアドレス> <Merossパスワード> --controller <SmokerのIPアドレス>

`smoker.local` を解決できない Windows 環境では、コントローラの IP アドレスを指定してください。
例: `python tools/meross_mss305.py <Merossメールアドレス> <Merossパスワード> --controller 192.168.1.50`

実行後、スクリプトは次を順に行います。

1. Meross クラウドにログイン
2. デバイスを探索
3. UUID / KEY / IP を取得
4. http://smoker.local/config/device へ POST

### うまくいかない場合

- Smoker 本体へ送信できない
  - smoker.local が解決できない場合は、PC の mDNS 解決状況を確認してください。
  - ルーターで Smoker 本体の IP を確認し、ブラウザで http://<SmokerのIP>/status にアクセスできるか確認してください。

- Meross デバイスが見つからない
  - Meross アカウント情報が正しいか確認してください。
  - デバイスが同じ Meross アカウントに登録済みか確認してください。

### セキュリティ注意

このスクリプトはメールアドレスとパスワードをコマンドライン引数で受け取ります。
シェル履歴に残る可能性があるため、共有端末では注意してください。
