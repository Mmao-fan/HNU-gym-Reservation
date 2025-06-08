import datetime
import requests
from datetime import date, timedelta
import re

import time

url = "https://eportal.hnu.edu.cn/site/reservation/launch"

def parse_cookie_values(text):
    """
    从给定的 curl 注释文本中提取 UM_distinctid、PHPSESSID、vjuid、vjvd、vt、cas_ticket 的值。
    返回一个 dict，key 为字段名，value 为对应的字符串（未找到时为 None）。
    """
    keys = ["UM_distinctid", "PHPSESSID", "vjuid", "vjvd", "vt", "cas_ticket"]
    result = {}
    for key in keys:
        # 匹配 key=后面直到第一个分号或单引号为止的内容
        m = re.search(rf"{re.escape(key)}=([^;']+)", text)
        result[key] = m.group(1) if m else None
    return result
headers = {
        'accept': 'application/json, text/plain, */*',
        'accept-language': 'zh-CN,zh;q=0.9,en;q=0.8',
        'cache-control': 'no-cache',
        'content-type': 'application/x-www-form-urlencoded',
        'origin': 'https://eportal.hnu.edu.cn',
        'pragma': 'no-cache',
        'priority': 'u=1, i',
        'referer': 'https://eportal.hnu.edu.cn/v2/reserve/reserveDetail?id=83',
        'sec-ch-ua': '"Google Chrome";v="135", "Not-A.Brand";v="8", "Chromium";v="135"',
        'sec-ch-ua-mobile': '?0',
        'sec-ch-ua-platform': '"macOS"',
        'sec-fetch-dest': 'empty',
        'sec-fetch-mode': 'cors',
        'sec-fetch-site': 'same-origin',
        'user-agent': 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/135.0.0.0 Safari/537.36',
        'x-requested-with': 'XMLHttpRequest',
}
raw = """
//   -b 'UM_distinctid=191bcfd51b1970-022323237a-4c657b58-1bcab9-191bcfd51b2103f; PHPSESSID=ST-36472323-efZoJ7ReP0N232H-zfsoftcom; vjuid=2323; vjvd=3e159fbcc098c33132fa6da01; vt=264323832; cas_ticket=ST-3647798-efZoJ7R3233vKH-zfsoft.com' \
"""
cookies = parse_cookie_values(raw)
# 计算明天的日期
tomorrow = date.today() + timedelta(days=1)

# 按 YYYY-MM-DD 格式生成字符串并赋值
datas = tomorrow.strftime("%Y-%m-%d")
def get_time(start_time,select,num,times=1):
    while True:
        if select==0:
            base_head=57
            base_time=3885
            base_id=16339
        elif select==1:
            base_head=85
            base_time=4075
            base_id=18917
        data = {
            "resource_id": f"{base_head}",
            "code": "",
            "remarks": "",
            "deduct_num": "",
            "data" : f'[{{"date":"{datas}","period":{start_time+base_time},"sub_resource_id":{14*num+base_id-start_time}}}]'
        }
        if times==2:
            data = {
                "resource_id": f"{base_head}",
                "code": "",
                "remarks": "",
                "deduct_num": "",
                "data":f'[{{"date":"{datas}","period":{start_time+base_time},"sub_resource_id":{14*num+base_id-start_time}}},{{"date":"{datas}","period":{start_time+base_time+1},"sub_resource_id":{14*num+base_id-start_time-1}}}]',
            }
        response = requests.post(url, headers=headers, cookies=cookies, data=data)
        print(response.text)
        if "成功" in response.text:
            print("成功")
    return response.text
def get_current_hour():
    return datetime.datetime.now().strftime("%H")
def get_current_minute():
    return datetime.datetime.now().strftime("%M")
def get_current_second():
    return datetime.datetime.now().strftime("%S")
if __name__ == "__main__":
    hour,min,sec=22,1,0
    while True:
        h=get_current_hour()
        m=get_current_minute()
        if int(h)==hour and int(m)==min and int(get_current_second())>=sec:
            print("开始抢单")
            break
        else:
            print(f"当前时间：{h}:{m}:{get_current_second()}")
            time.sleep(0.01)
    while True:
        get_time(13,1,1,1)