import argparse
import asyncio
import os
import re
import requests
from datetime import datetime
from meross_iot.http_api import MerossHttpClient, MerossCloudCreds
from meross_iot.manager import MerossManager


def _is_ipv4(text):
    if not isinstance(text, str):
        return False
    if not re.match(r"^\d{1,3}(?:\.\d{1,3}){3}$", text):
        return False
    parts = text.split(".")
    return all(0 <= int(p) <= 255 for p in parts)


def _collect_ip_candidates(dev):
    candidates = []

    for attr in ("lan_ip", "host", "inner_ip", "local_ip", "ip"):
        v = getattr(dev, attr, None)
        if isinstance(v, str) and _is_ipv4(v):
            candidates.append(v)

    descriptor = getattr(dev, "descriptor", None)
    if descriptor is not None:
        for attr in ("innerIp", "inner_ip", "host", "ip"):
            v = getattr(descriptor, attr, None)
            if isinstance(v, str) and _is_ipv4(v):
                candidates.append(v)

    # Keep order and remove duplicates.
    uniq = []
    for ip in candidates:
        if ip not in uniq:
            uniq.append(ip)
    return uniq

async def main(email, password):
    print("Merossクラウドにログイン中...")
    api_base_url = "https://iot.meross.com"
    dummy_creds = MerossCloudCreds(user_id="", user_email=email, issued_on=datetime.now().isoformat(), domain=api_base_url, mqtt_domain="", token="", key="")
    http_client = MerossHttpClient(cloud_credentials=dummy_creds)

    try:
        creds = await http_client.async_login(email=email, password=password, api_base_url=api_base_url)
        if not creds:
            creds = http_client.cloud_credentials
    except Exception as e:
        print(f"❌ ログイン失敗: {e}")
        return

    print("デバイスを検索中（数秒かかります）...")
    manager = None
    ip_val = None
    key_val = creds.key
    uuid_val = None

    try:
        manager = MerossManager(http_client=MerossHttpClient(cloud_credentials=creds))
        await manager.async_init()
        devices = await manager.async_device_discovery()

        # MQTT経由の状態同期が到着するまで待機
        await asyncio.sleep(15)

        if not devices:
            devices = manager.find_devices()

        if not devices:
            print("❌ デバイスが見つかりませんでした。")
            return

        dev = devices[0]
        uuid_val = dev.uuid
        print(f"\n✅ デバイス「{dev.name}」を発見！")

        candidates = _collect_ip_candidates(dev)
        if candidates:
            ip_val = candidates[0]

        if hasattr(dev, "async_update") and asyncio.iscoroutinefunction(dev.async_update):
            try:
                await dev.async_update()
                after_update = _collect_ip_candidates(dev)
                if after_update and not ip_val:
                    ip_val = after_update[0]
                candidates = after_update or candidates
            except Exception as e:
                print(f"⚠ async_update 失敗: {e}")

        print(f"  UUID : {uuid_val}")
        print(f"  Key  : ******** (取得成功)")
        print(f"  IP候補: {', '.join(candidates) if candidates else '(なし)'}")
        print(f"  使用IP: {ip_val if ip_val else '自動取得失敗'}")
    finally:
        if manager is not None:
            if hasattr(manager, "async_close") and asyncio.iscoroutinefunction(manager.async_close):
                await manager.async_close()
            else:
                manager.close()
            await asyncio.sleep(2)

    if not uuid_val:
        print("❌ UUID を取得できませんでした。")
        return

    # 自動取得できなかった場合のフォールバック
    if not ip_val:
        ip_val = input("\nプラグのIPアドレスを手動で入力してください (例: 192.168.0.2): ")
        
    # コントローラへの無線送信
    esp_url = "http://smoker.local/config/device"
    payload = {
        "uuid": uuid_val,
        "key": key_val,
        "ip": ip_val
    }
    
    print(f"\nコントローラ ({esp_url}) に設定データを送信中...")
    try:
        response = requests.post(esp_url, data=payload, timeout=10)
        if response.status_code == 200:
            print("✨ 完了！シリアル接続なしでコントローラにキーが記憶されました！")
        else:
            print(f"❌ 失敗: コントローラがエラーを返しました ({response.status_code})\n詳細: {response.text}")
    except Exception as e:
        print(f"❌ 通信エラー: コントローラが見つかりません。")
        print("  ・コントローラの電源が入っているか\n  ・PCと同じWi-Fiネットワークに繋がっているかを確認してください。")

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description="Meross設定取得＆自動転送スクリプト",
        formatter_class=argparse.RawTextHelpFormatter,
        epilog=(
            "実行例:\n"
            "  python tools/meross_mss305.py your_mail@example.com your_password\n"
            "\n"
            "引数:\n"
            "  email     Merossアカウントのメールアドレス\n"
            "  password  Merossアカウントのパスワード\n"
        ),
    )
    parser.add_argument("email", metavar="email", help="Merossアカウントのメールアドレス")
    parser.add_argument("password", metavar="password", help="Merossアカウントのパスワード")
    args = parser.parse_args()

    if os.name == 'nt':
        asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())
    asyncio.run(main(args.email, args.password))