#include <Wire.h>
#include <IRremote.h>

//红外编码
IRsend irsend;
unsigned int  IRopen[100] = {4400, 4400, 600, 1550, 600, 500, 600, 1550, 600, 1550, 600, 500, 600, 500, 600, 1550, 600, 500, 600, 450, 600, 1600, 600, 450, 600, 500, 600, 1550, 600, 1600, 600, 450, 600, 1600, 600, 500, 550, 1600, 600, 500, 550, 1600, 600, 1550, 600, 1600, 550, 1600, 600, 1550, 600, 1600, 550, 500, 600, 1600, 550, 500, 600, 500, 600, 500, 550, 500, 600, 500, 600, 1550, 600, 1600, 550, 500, 600, 500, 600, 500, 550, 500, 600, 500, 600, 500, 550, 500, 600, 500, 600, 1550, 600, 1600, 600, 1550, 600, 1550, 600, 1600, 600, 1550, 600, 5200}; //制冷 20 中风
unsigned int IR30[100] = {4400, 4400, 550, 1600, 600, 500, 550, 1600, 600, 1550, 600, 500, 600, 500, 600, 1550, 600, 500, 600, 450, 600, 1600, 600, 450, 600, 500, 600, 1550, 600, 1600, 600, 450, 600, 1600, 600, 450, 600, 1600, 550, 500, 600, 1600, 550, 1600, 600, 1550, 600, 1600, 550, 1600, 600, 1550, 600, 500, 600, 1550, 600, 500, 600, 500, 600, 450, 600, 500, 600, 500, 600, 1550, 600, 500, 600, 1550, 600, 1550, 600, 500, 600, 500, 600, 450, 600, 500, 600, 500, 600, 1550, 600, 500, 600, 500, 550, 1600, 600, 1550, 600, 1600, 550, 1600, 600, 5150}; //制冷 30 中风
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

//回归预测未来温度
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

void update_y(float data) {
  for (int i = 0; i < interval; ++i) {
    y[i] = y[i + 1];
  }
  y[interval - 1] = data;
}

void setup() {
  Serial.begin(9600);
  Serial.println("Start");
  Wire.begin();
  get_temp(); //清除第一次不准确的数据
  for(int i=0;i<interval;++i){ //对y的数据初始化
    y[i]=get_temp();
    delay(1000);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  float future_temp;
  int mode=0;
  get_ab(y);
  future_temp=b*20+a;

  for(int i=0;i<interval;++i){ //对y的数据初始化
    Serial.print(y[i],1);
    Serial.print(',');
  }
  Serial.print("\nTemperature:");
  Serial.println(future_temp, 1);

  if (future_temp >= 28 && mode!=1) {
    irsend.sendRaw(IRopen, 100, khz);
    irsend.sendRaw(IRopen, 100, khz);//美的空调需要发射两次才行
    Serial.println("send open");
    mode=1;
  }
  else if(mode!=2){
    irsend.sendRaw(IR30, 100, khz);
    irsend.sendRaw(IR30, 100, khz);
    Serial.println("send 30");
    mode=2;
  }
  delay(60000);
  update_y(get_temp());
}
