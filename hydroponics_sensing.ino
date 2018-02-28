/*
	코드명 : 수경재배 센싱코드
	작성일 :2018.02.26
	작성자 : 이제호, 백인걸
*/
#include <OneWire.h>	//DS18 센서 사용
#include <DHT_U.h>
#include <DHT.h>		//DH11/22 센서 사용

#define fan1 11
#define fan2 12
#define fan3 13		//냉각팬 핀번호
#define DS18_pin 38
#define DHT_pin 39
#define WATER_LEVEL_pin 5	//수위센서(0, 1)
#define POT1_pin 0
#define POT2_pin 1
#define POT3_pin 2
#define POT4_pin 3		//조도센서 1,2,3,4(아날로그핀)
#define RELAY_IN1 28
#define RELAY_IN2 29
#define RELAY_IN3 30
#define RELAY_IN4 31

DHT dht22(DHT_pin, DHT22);
OneWire DS18(DS18_pin);		//수위센서
byte DS18_addr[8] = { 0 };	//수위센서 주소값

int POT_val[5] = { 0, };	//val1, val2, val3, val4, avg
boolean fan_state = 0;		//냉각팬 상태. 0 : OFF, 1 : ON
boolean ds18_state = 0;		//연결상태. 0 : 연결X, 1 : 연결O

long sensor_previousTime = 0;
long sensor_interval = 5000;		//센서 측정 시간.

void setup()
{
	pinMode(fan1, OUTPUT);
	pinMode(fan2, OUTPUT);
	pinMode(fan3, OUTPUT);		//냉각팬 핀모드 : 출력
	pinMode(WATER_LEVEL_pin, INPUT);	//수위 센서 핀모드 : 출력
	pinMode(RELAY_IN1, OUTPUT);
	pinMode(RELAY_IN2, OUTPUT);
	pinMode(RELAY_IN3, OUTPUT);
	pinMode(RELAY_IN4, OUTPUT);			//LED제어용 릴레이 핀모드.
	for (int i = RELAY_IN1; i <= RELAY_IN4; i++) {
		digitalWrite(i, HIGH);
	}

	Serial.begin(9600);
	dht22.begin();
	Connect_DS18();
}

void loop()
{
	delay(2000);	//delay : 2초로 millis가 2초 밀림.
	float DHT_temp = dht22.readTemperature();
	float DHT_humi = dht22.readHumidity();
	float Water_temp = getTemp();		//수온측정
	fan_control(Water_temp);			//수온에 따른 냉각팬 제어
	read_POT();							//조도센서값 배열에 저장.
	Relay_Control();

	//일정 시간마다 센싱한 값 출력(시리얼)
	if (millis() - sensor_previousTime > sensor_interval) {
		Serial.print(String("Water Temperature : ") + Water_temp + "\n");
		Serial.print(String("DHT Temperature : ") + DHT_temp + "\n");
		Serial.print(String("DHT Humidity : ") + DHT_humi + "\n");
		Serial.print(String("Fan State : ") + fan_state + "\n");
		Serial.print(String("POT Value : ") + POT_val[0] + " " + POT_val[1] + " " + POT_val[2] + " " + POT_val[3] + " " + POT_val[4] + "\n");
		Serial.println();
		sensor_previousTime = millis();
	}

  /* add main program code here */

}
//--------------------프로그램 함수----------------------
//조도센서 값 읽는 함수.
void read_POT() {
	int temp = 0;		//4개 값의 합 저장.

	for (int i = 0; i < 4; i++) {
		POT_val[i] = analogRead(i);		//조도센서 각각의 값을 저장.
		temp += POT_val[i];
	}
	POT_val[4] = temp / 4;	//4개 값 평균
}

//조도센서 값에 따라 LED 제어용 릴레이 ON/OFF
void Relay_Control() {
	if (POT_val[4] <= 90) 
		for (int i = RELAY_IN1; i <= RELAY_IN4; i++)
			digitalWrite(i, LOW);
	else if(POT_val[4] > 90)
		for (int i = RELAY_IN1; i <= RELAY_IN4; i++)
			digitalWrite(i, HIGH);
}

//DS18 연결 함수
void Connect_DS18() {
	if (DS18.search(DS18_addr)) {
		Serial.println("DS18 Connected.");
		ds18_state = 1;
	}
	else {
		Serial.println("DS18 Can not find!");
		ds18_state = 0;
	}
}

//DS18 수온 측정 함수
float getTemp(void) {
	byte data[9] = { 0 };

	if (ds18_state == 0) {
		Serial.println("DS18_not_find!!!");
		return 0.0;
	}		//수온센서 연결이 안될경우,

	if (OneWire::crc8(DS18_addr, 7) != DS18_addr[7]) {
		Serial.println("CRC is not valid!");
		return;
	}
	DS18.reset();
	DS18.select(DS18_addr);
	DS18.write(0x44, 1);        //???
	DS18.reset();
	delay(1000);
	DS18.select(DS18_addr);
	DS18.write(0xBE);		//Read Scratchpad.

	for (int i = 0; i < 9; i++) {           // we need 9 bytes
		data[i] = DS18.read();
	}

	DS18.reset_search();

	byte MSB = data[1];
	byte LSB = data[0];
	float tempRead = ((MSB << 8) | LSB);
	float TemperatureSum = tempRead / 16;		//측정된 값을 섭씨온도로 변환.

	return TemperatureSum;
}

//냉각팬 제어 함수.
void fan_control(float temp) {
	if (temp > 22.00) {
		if (fan_state == 0) {
			fan_state = 1;
			digitalWrite(fan1, HIGH);
			digitalWrite(fan2, HIGH);
			digitalWrite(fan3, HIGH);
		}
	}
	else if (temp < 17.00) {
		if (fan_state == 1) {
			fan_state = 0;
			digitalWrite(fan1, LOW);
			digitalWrite(fan2, LOW);
			digitalWrite(fan3, LOW);
		}
	}
	else {
		if (fan_state == 1) {
			fan_state = 0;
			digitalWrite(fan1, LOW);
			digitalWrite(fan2, LOW);
			digitalWrite(fan3, LOW);
		}
	}
}