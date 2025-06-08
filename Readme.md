# 湖南大学体育馆预约脚本
* 注意，脚本虽快，但使用该脚本需要有一定计算机基础
* 直接抓请求的包，直接curl
* 只抓了两个羽毛球馆的包，用了快一年了还没失手过

# ball.cpp——C语言多线程同时请求预约，最快的请求方式
# ball.py——python单线程，预约没有C快
# 使用说明
* 用浏览器登录一网通办，然后F12抓包
![alt text](image.png)
![alt text](image-1.png)
* 例如抓到的数据为
```
curl 'https://eportal.hnu.edu.cn/v2/static/version.js?v=1749397086013' \
  -H 'accept: application/json, text/javascript, */*; q=0.01' \
  -H 'accept-language: zh-CN,zh;q=0.9,en;q=0.8' \
  -H 'cache-control: no-cache' \
  -b 'UM_distinctid=191bcfd51b1970-1111111111-4c6511158-1bcab9-191bcf1151b2103f; PHPSESSID=ST-41911-4scez111c7wNouIS-zfsoftcom; vjuid=2811126; vjvd=3e159fb11111331876d931c1fa6da01; vt=26511194; cas_ticket=ST-41911161-4scezq111111uIS-zfsoft.com' \
  -H 'pragma: no-cache' \
  -H 'priority: u=0, i' \
  -H 'referer: https://eportal.hnu.edu.cn/v2/reserve/reserveDetail?id=57' \
  -H 'sec-ch-ua: "Google Chrome";v="137", "Chromium";v="137", "Not/A)Brand";v="24"' \
  -H 'sec-ch-ua-mobile: ?0' \
  -H 'sec-ch-ua-platform: "macOS"' \
  -H 'sec-fetch-dest: empty' \
  -H 'sec-fetch-mode: cors' \
  -H 'sec-fetch-site: same-origin' \
  -H 'user-agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/137.0.0.0 Safari/537.36' \
  -H 'x-requested-with: XMLHttpRequest'
```
* header如果你登录不换浏览器不换设备只要改一次就行，每次运行只需要改cookie
# 修改.cpp文件
* 根据你复制的curl来修改标头：
```
	headers = curl_slist_append(headers, "Accept: application/json, text/plain, */*");
	headers = curl_slist_append(headers, "Accept-Language: zh-CN,zh;q=0.9,en;q=0.8");
	headers = curl_slist_append(headers, "Cache-Control: no-cache");
	headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
	headers = curl_slist_append(headers, "Origin: https://eportal.hnu.edu.cn");
	headers = curl_slist_append(headers, "Pragma: no-cache");
	headers = curl_slist_append(headers, "Priority: u=1, i");
	headers = curl_slist_append(headers, "Referer: https://eportal.hnu.edu.cn/v2/reserve/reserveDetail?id=83");
	headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0");
	headers = curl_slist_append(headers, "X-Requested-With: XMLHttpRequest");
```
* 然后在raw_cookie中填写你的cookie例：
```
	const char* raw_cookie = "UM_distinctid=19456bcf51973-02b317ffc187a-dfd-1bcab9-191bcsdg1b2103f; PHPSESSID=ST-3634698-efZoJ7ReP0dfhbYd3vKH-zfsoftcom; vjuid=283900; vjvd=3e159fssdfdfd1876d931c1fa6da01; vt=264sddsf332; cas_ticket=ST-3647798-efZoJ723sdfd3vKH-zfsoft.com";

```
* 最后在main函数中修改预约信息
```
		args[i].start_time = 12;
		//几点开始
		args[i].select = 1;
		//0综合馆 1体育馆
		args[i].num = 1;
		//第几个场地
		args[i].times = 2;
		//连续1小时 连续2小时
```
* num_threads可以修改线程数
* 开始预约时间默认为22:01:00(可修改)
```
const int target_hour = 22;
const int target_min = 1;
const int target_sec = 0;
```
# 编译运行
```
gcc ball.cpp -o ball -lcurl -lpthread && ./ball
```
* 缺gcc缺库自己百度装
# 修改.py文件
* 同理修改headers和raw
```
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
```
* 调用方式如下，参数填写和C++一样      
```
get_time(13,1,1,1)
```



