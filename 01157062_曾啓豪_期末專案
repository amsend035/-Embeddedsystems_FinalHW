from machine import UART
import utime
import ntptime
import time
from xtools import connect_wifi_led

# 1. Wi-Fi
connect_wifi_led()

# 2. UART
com = UART(2, 9600, tx=17, rx=16)
com.init(9600)

print("ESP32 ready")

# 3. 城市與時區偏移（相對於 UTC）
city_offset = {
    'TPE': 8,
    'TYO': 9,
    'PAR': 2,
    'NYC': -4,
    'SYD': 10,
    'LON': 1,
    'BER': 2,
    'SFO': -7
}

# 4. 同步 NTP（只做一次）
try:
    ntptime.settime()  # 將時間同步到 UTC
except:
    print("NTP sync failed")

def get_local_time(offset):
    t = time.localtime(time.time() + offset * 3600)
    hhmm = '{:02}{:02}'.format(t[3], t[4])  # HHMM
    return hhmm

while True:
    if com.any():
        try:
            msg = com.read().decode().strip().upper()
            print("Received:", msg)
            if msg in city_offset:
                offset = city_offset[msg]
                hhmm = get_local_time(offset)
                print("Send:", hhmm)
                com.write(hhmm.encode())
        except Exception as e:
            print("Error:", e)
    utime.sleep(0.1)
