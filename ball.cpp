#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <curl/curl.h>
#include <sys/time.h>  
const int target_hour = 22;
const int target_min = 1;
const int target_sec = 0;
typedef struct {
	char* memory;
	size_t size;
}
MemoryStruct;
typedef struct {
	char* UM_distinctid;
	char* PHPSESSID;
	char* vjuid;
	char* vjvd;
	char* vt;
	char* cas_ticket;
}
CookieValues;
static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp) {
	size_t realsize = size * nmemb;
	MemoryStruct* mem = (MemoryStruct*)userp;
	char* ptr = (char*) realloc(mem->memory, mem->size + realsize + 1);
	if (!ptr) return 0;
	mem->memory = ptr;
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;
	return realsize;
}
CookieValues parse_cookie_values(const char* text) {
	CookieValues cookies = {
		0
	}
	;
	const char* keys[] = {
		"UM_distinctid", "PHPSESSID", "vjuid", "vjvd", "vt", "cas_ticket"
	}
	;
	char** values[] = {
		&cookies.UM_distinctid, &cookies.PHPSESSID, &cookies.vjuid,
		                       &cookies.vjvd, &cookies.vt, &cookies.cas_ticket
	}
	;
	for (int i = 0; i < 6; ++i) {
		const char* start = strstr(text, keys[i]);
		if (start) {
			start += strlen(keys[i]) + 1;
			const char* end = strpbrk(start, ";'");
			size_t len = end ? (size_t)(end - start) : strlen(start);
			*values[i] = (char*) malloc(len + 1);
			strncpy(*values[i], start, len);
			(*values[i])[len] = '\0';
		}
	}
	return cookies;
}
void get_tomorrow_date(char* buffer, size_t size) {
	time_t now = time(NULL);
	struct tm* tm = localtime(&now);
	tm->tm_mday += 1;
	mktime(tm);
	// Normalize
	strftime(buffer, size, "%Y-%m-%d", tm);
}
char* get_time(int start_time, int select, int num, int times, CookieValues cookies) {
	CURL* curl = curl_easy_init();
	if (!curl) return NULL;
	curl_global_init(CURL_GLOBAL_DEFAULT);
	int base_head = select == 0 ? 57 : 85;
	int base_time = select == 0 ? 3885 : 4075;
	int base_id   = select == 0 ? 16339 : 18917;
	char date_str[16];
	get_tomorrow_date(date_str, sizeof(date_str));
	char json[512];
	if (times == 2) {
		snprintf(json, sizeof(json),
		                 "[{\"date\":\"%s\",\"period\":%d,\"sub_resource_id\":%d},"
		                 "{\"date\":\"%s\",\"period\":%d,\"sub_resource_id\":%d}]",
		                 date_str, start_time + base_time, 14 * num + base_id - start_time,
		                 date_str, start_time + base_time + 1, 14 * num + base_id - start_time - 1);
	} else {
		snprintf(json, sizeof(json),
		                 "[{\"date\":\"%s\",\"period\":%d,\"sub_resource_id\":%d}]",
		                 date_str, start_time + base_time, 14 * num + base_id - start_time);
	}
	char* encoded_json = curl_easy_escape(curl, json, 0);
	char postfields[1024];
	snprintf(postfields, sizeof(postfields),
	             "resource_id=%d&code=&remarks=&deduct_num=&data=%s",
	             base_head, encoded_json);
	curl_free(encoded_json);
	// 构造 Cookie 头
	char cookie_header[512];
	snprintf(cookie_header, sizeof(cookie_header),
	             "UM_distinctid=%s; PHPSESSID=%s; vjuid=%s; vjvd=%s; vt=%s; cas_ticket=%s",
	             cookies.UM_distinctid, cookies.PHPSESSID, cookies.vjuid,
	             cookies.vjvd, cookies.vt, cookies.cas_ticket);
	struct curl_slist* headers = NULL;
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
	// 响应数据结构体
	MemoryStruct chunk = {
		.memory = (char*) malloc(1), .size = 0
	}
	;
	curl_easy_setopt(curl, CURLOPT_URL, "https://eportal.hnu.edu.cn/site/reservation/launch");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_COOKIE, cookie_header);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfields);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	// curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	CURLcode res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
	}
	// 清理
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);
	curl_global_cleanup();
	return chunk.memory;
}
#include <pthread.h>
// 携带参数的结构体
typedef struct {
	CookieValues cookies;
	int start_time;
	int select;
	int num;
	int times;
	int thread_id;
}
ThreadArgs;
void wait_until(int target_hour, int target_min, int target_sec) {
	while (1) {
		time_t now = time(NULL);
		struct tm* tm = localtime(&now);
		if (tm->tm_hour == target_hour && tm->tm_min == target_min && tm->tm_sec >= target_sec) {
			break;
		} else {
			printf("当前时间：%02d:%02d:%02d\n", tm->tm_hour, tm->tm_min, tm->tm_sec);
		}
		struct timespec ts = {
			0, 10000000
		}
		;
		nanosleep(&ts, NULL);
	}
}
void* thread_func(void* arg) {
	ThreadArgs* args = (ThreadArgs*)arg;
	wait_until(target_hour, target_min, target_sec);
	while (1) {
		struct timeval tv;
		gettimeofday(&tv, NULL);
		struct tm* tm_info = localtime(&tv.tv_sec);
		char time_buffer[32];
		strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", tm_info);
		char* response = get_time(args->start_time, args->select, args->num, args->times, args->cookies);
		printf("[%s.%03d] 线程 %2d : %s \n", time_buffer, tv.tv_usec / 1000, args->thread_id, response);
		if (strstr(response, "成功")) {
			printf("✅ 线程 %d 抢单成功！\n", args->thread_id);
			free(response);
			exit(0);
			// 所有线程直接退出
		}
		free(response);
	}
	return NULL;
}
int main() {
	const char* raw_cookie = "UM_distinctid=19456bcf51973-02b317ffc187a-dfd-1bcab9-191bcsdg1b2103f; PHPSESSID=ST-3634698-efZoJ7ReP0dfhbYd3vKH-zfsoftcom; vjuid=283900; vjvd=3e159fssdfdfd1876d931c1fa6da01; vt=264sddsf332; cas_ticket=ST-3647798-efZoJ723sdfd3vKH-zfsoft.com";
	CookieValues cookies = parse_cookie_values(raw_cookie);
	const int num_threads = 12;
	pthread_t threads[num_threads];
	ThreadArgs args[num_threads];
	for (int i = 0; i < num_threads; ++i) {
		args[i].cookies = cookies;
		args[i].start_time = 12;
		//几点开始
		args[i].select = 1;
		//0综合馆 1体育馆
		args[i].num = 1;
		//第几个场地
		args[i].times = 2;
		//连续1小时 连续2小时
		args[i].thread_id = i + 1;
		pthread_create(&threads[i], NULL, thread_func, &args[i]);
	}
	for (int i = 0; i < num_threads; ++i) {
		pthread_join(threads[i], NULL);
	}
	free(cookies.UM_distinctid);
	free(cookies.PHPSESSID);
	free(cookies.vjuid);
	free(cookies.vjvd);
	free(cookies.vt);
	free(cookies.cas_ticket);
	return 0;
}