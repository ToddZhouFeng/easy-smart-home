#include <Wire.h>
#include <IRremote.h>

int target_temp = 24; //目标温度，必须是二十几，人体最舒适的温度为18~24
//红外编码部分开始//
IRsend irsend;
unsigned int IR20high[100] = {4450, 4350, 600, 1550, 650, 450, 600, 1550, 650, 1550, 600, 450, 650, 450, 600, 1550, 600, 500, 600, 500, 600, 1550, 600, 500, 550, 500, 650, 1550, 600, 1550, 600, 500, 600, 1550, 600, 500, 600, 450, 600, 1600, 550, 1600, 600, 1550, 600, 1600, 600, 1550, 600, 1550, 600, 1600, 600, 1550, 600, 500, 600, 450, 600, 500, 600, 500, 600, 500, 550, 500, 600, 500, 600, 500, 550, 1600, 600, 500, 550, 500, 600, 500, 600, 500, 550, 500, 600, 1600, 550, 1600, 600, 500, 600, 1550, 600, 1550, 600, 1600, 550, 1600, 600, 1550, 600, 5200}; //制冷 20 高风
unsigned int  IR25mid[100] = {4400, 4400, 600, 1550, 600, 500, 600, 1550, 600, 1550, 600, 500, 600, 500, 600, 1550, 600, 500, 600, 450, 600, 1600, 600, 450, 600, 500, 600, 1550, 600, 1600, 600, 450, 600, 1600, 600, 500, 550, 1600, 600, 500, 550, 1600, 600, 1550, 600, 1600, 550, 1600, 600, 1550, 600, 1600, 550, 500, 600, 1600, 550, 500, 600, 500, 600, 500, 550, 500, 600, 500, 600, 1550, 600, 1600, 550, 500, 600, 500, 600, 500, 550, 500, 600, 500, 600, 500, 550, 500, 600, 500, 600, 1550, 600, 1600, 600, 1550, 600, 1550, 600, 1600, 600, 1550, 600, 5200}; //制冷 25 中风
//unsigned int IR30mid[100] = {4400, 4400, 550, 1600, 600, 500, 550, 1600, 600, 1550, 600, 500, 600, 500, 600, 1550, 600, 500, 600, 450, 600, 1600, 600, 450, 600, 500, 600, 1550, 600, 1600, 600, 450, 600, 1600, 600, 450, 600, 1600, 550, 500, 600, 1600, 550, 1600, 600, 1550, 600, 1600, 550, 1600, 600, 1550, 600, 500, 600, 1550, 600, 500, 600, 500, 600, 450, 600, 500, 600, 500, 600, 1550, 600, 500, 600, 1550, 600, 1550, 600, 500, 600, 500, 600, 450, 600, 500, 600, 500, 600, 1550, 600, 500, 600, 500, 550, 1600, 600, 1550, 600, 1600, 550, 1600, 600, 5150}; //制冷 30 中风
unsigned int IR30low[100] = {4450, 4350, 600, 1550, 600, 550, 550, 1550, 650, 1550, 600, 500, 550, 550, 550, 1600, 550, 550, 550, 500, 600, 1550, 600, 500, 600, 500, 550, 1600, 600, 1550, 600, 500, 600, 1550, 600, 1600, 550, 550, 550, 500, 600, 1550, 600, 1600, 550, 1600, 600, 1550, 600, 1600, 600, 500, 550, 1600, 600, 1550, 600, 500, 550, 550, 550, 550, 550, 500, 550, 500, 600, 1600, 550, 550, 550, 1600, 600, 1550, 600, 500, 600, 500, 550, 550, 550, 500, 600, 500, 550, 1600, 600, 500, 550, 550, 550, 1600, 550, 1600, 600, 1550, 600, 1600, 550, 5200}; //制冷 30 低风
int khz = 38; // 38kHz carrier frequency

//DHT12获取温度
float get_temp() {
  float temperature = 30;
  uint8_t bits[5];
  uint8_t checksum;
  do {
    Wire.beginTransmission(0x5C); //DHT12 的地址
    Wire.write(0); //向 DHT12 请求数据
    Wire.endTransmission();
    int bytes = Wire.requestFrom(0x5C, 5);

    for (int i = 0; i < bytes; ++i) {
      bits[i] = Wire.read();
    }
    //humidity = bits[0] + bits[1]*0.1; //获取湿度（此程序不需要）
    temperature = bits[2] + (bits[3] & 0x7F) * 0.1; //获取温度
    if (bits[3] & 0x80) {
      temperature = -temperature;
    }
    checksum = bits[0] + bits[1] + bits[2] + bits[3]; //校验
  } while (bits[4] != checksum);
  return temperature;
}
//红外编码部分结束//

//回归预测未来温度开始//
int interval = 10; //y的数量
float aver_x = 5.5; //从1到interval的平均值，节省时间
float a, b; //回归方程的参数
float y[10] = {};//数量要等于interval

int get_ab(float y[]) {
  float aver_y = 0;
  for (int i = interval - 1; i >= 0; --i) {
    aver_y += y[i];
  }
  aver_y /= interval;

  float b_frac_up = 0;
  for (int i = interval; i; --i) {
    b_frac_up += (i * y[i - 1]);
  }
  b_frac_up -= (interval * aver_x * aver_y);

  float b_frac_down = 0;
  for (int i = interval; i; --i) {
    b_frac_down += (i * i);
  }
  b_frac_down -= (interval * aver_x * aver_x);

  if (b_frac_down) {
    b = b_frac_up / b_frac_down;
    a = aver_y - b * aver_x;
  }
  else return 0;
  return 1;
}

void update_y(float data) { //更新已有数据
  for (int i = 0; i < interval; ++i) {
    y[i] = y[i + 1];
  }
  y[interval - 1] = data;
}

//回归预测未来温度结束//

//setup程序开始//
void setup() {
  Serial.begin(9600);
  Serial.println("Start");
  Wire.begin();
  get_temp(); //清除第一次不准确的数据
  for (int i = 0; i < interval; ++i) { //对y的数据初始化
    y[i] = get_temp();
    delay(1000);
  }
}
//setup程序结束//

//loop程序开始//
int mode = 0;
void loop() {
  // put your main code here, to run repeatedly:
  float future_temp;
  get_ab(y);
  future_temp = b * 20 + a;

  for (int i = 0; i < interval; ++i) { //对y的数据初始化
    Serial.print(y[i], 1);
    Serial.print(',');
  }
  Serial.print("\nTemperature:");
  Serial.println(future_temp, 1);

  if (future_temp <= target_temp+5) {
    if (future_temp >= target_temp-2 && future_temp <= target_temp+2) { //在舒适温度范围内，则不高不低
      if (mode != 25) {
        irsend.sendRaw(IR25mid, 100, khz);
        irsend.sendRaw(IR25mid, 100, khz);//美的空调需要发射两次才行
        Serial.println("send 25");
        mode = 25;
      }
    }
    else if (mode != 30) { //低于舒适温度，则制冷最弱
      irsend.sendRaw(IR30low, 100, khz);
      irsend.sendRaw(IR30low, 100, khz);
      Serial.println("send 30");
      mode = 30;
    }
  }
  else if (mode != 20) { //过高于舒适温度，则尽快降温
    irsend.sendRaw(IR20high, 100, khz); //17℃的风太猛了
    irsend.sendRaw(IR20high, 100, khz);
    Serial.println("send 20");
    mode = 20;
  }
  delay(60000);
  update_y(get_temp());
}
//loop程序结束//
