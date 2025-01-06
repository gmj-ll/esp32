#include <Arduino.h>
#include <WiFi.h>
#define NTP "ntp.aliyun.com"
#include <driver/rtc_io.h>

#include <ThreeWire.h>
#include <RtcDS1302.h>

#include <iconv.h>

const char* shippingName = "˳����";


const int shippingId = 1; 

const int IO = 16;    // DAT
const int SCLK = 17;  // CLK
const int CE = 18;    // RST

ThreeWire myWire(IO, SCLK, CE);
RtcDS1302<ThreeWire> Rtc(myWire);

struct tm system_time; // 系统时间

const char *ssid = "萌鸡的iPhone";
const char *password = "gmj666666";

void printDateTime(const RtcDateTime& dt) {
  char datestring[20];

  snprintf_P(datestring,
             countof(datestring),
             PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
             dt.Month(),
             dt.Day(),
             dt.Year(),
             dt.Hour(),
             dt.Minute(),
             dt.Second());
  Serial.print(datestring);
}


boolean wifi_init(){
  // 设置 ESP32 工作模式
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("正在连接 WiFi");
  int t = 0;
  while(WiFi.status() != WL_CONNECTED && t < 2){
    t++;
    delay(500);
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("wifi error");
    RtcDateTime dt = Rtc.GetDateTime();
    struct timeval tv;
    tv.tv_sec = dt.TotalSeconds() + 946656000 + 8 * 3600;
    tv.tv_usec = 0;
    struct timezone tz;
    settimeofday(&tv, NULL);
    getLocalTime(&system_time);
    // system_time 取rtc时间
    return false;
  } else {
    // system_time 取wifi时间
    Serial.println("WiFi 连接成功");
    // 设置时间
    configTime(8*3600, 0, NTP);
    getLocalTime(&system_time);
    char date_str[20];
    char time_str[20];
    strftime(date_str, sizeof(date_str), "%b %d %Y", &system_time);
    strftime(time_str, sizeof(time_str), "%H:%M:%S", &system_time);
    RtcDateTime compiled = RtcDateTime(date_str, time_str);
    Rtc.SetDateTime(compiled);
    RtcDateTime now = Rtc.GetDateTime();
    printDateTime(now);
    return true;
  }
}


void time_init() {
  // 获取本地时间
  if (!getLocalTime(&system_time)) {
    Serial.println("获取本地时间失败");
    // 连接 WiFi
    if (wifi_init()) {
      // �? NTP 服务器获取时间并设置
      configTime(8*3600, 0, NTP); 
    }
    
    return;
  }

  // 输出时间
//  Serial.println(timeinfo.tm_year)
  Serial.println(&system_time, "%F %T");
  
}

unsigned int checkBusy() {
    unsigned int isBusy = 0; // 0��ʾ��æ��1��ʾæ
    long times = millis();
    String response = "";
    while (Serial1.available()) {
        if (millis() - times > 1000) { // ��ʱ1��
            break;
        }
        char c = Serial1.read();
        response += c;
        if (c == '\n') {
            break;
        }
    }
    if (response == "OK\r\n") {
        isBusy = 0; // �յ�"OK"��ʾ��æ
    } else {
        isBusy = 1; // �����ʾ�?
    }
    return isBusy;
}

void sendCmd(String cmd) {
    String endingCode = ";\r\n";
    String realCmd = cmd + endingCode;

    Serial.print(realCmd); // ���ڵ���

    Serial1.println(realCmd);
    Serial1.flush();
    
    while (checkBusy()) {
        delay(100); // �ȴ��豸��æ
        Serial.println(" -> is busy, retrying...");
    }
    Serial.println(" -> sent successfully");
}

void changeText(int id, String content) {
    String cmd = "SET_TXT(" + String(id) + ",'" + content + "')";
    sendCmd(cmd);
}

void setup()
{
    Serial.begin(115200); // ��ʼ���������ڵ���
    while (!Serial) {}; // �ȴ���������
    Serial1.begin(115200); // ��ʼ������1��������Ļͨ��
    while (!Serial1) {}; // �ȴ�����1����
    wifi_init();
    Serial.println("init");
}

void loop()
{
    time_init();
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &system_time);

    String cmd = "SET_TXT(0,'" + String(time_str) +  "');" + 
    "SET_TXT(1,'" + String(shippingName) +  "');"
    "QBAR(0," + shippingId + "/" + String(time_str) + ")";
    sendCmd(cmd);
    delay(60000);
}
#define countof(a) (sizeof(a) / sizeof(a[0]))

