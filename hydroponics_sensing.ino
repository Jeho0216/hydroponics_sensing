/*
	�ڵ�� : ������� �����ڵ�
	�ۼ��� :2018.02.26
	�ۼ��� : ����ȣ, ���ΰ�
*/
#include <OneWire.h>	//DS18 ���� ���
#include <DHT_U.h>
#include <DHT.h>		//DH11/22 ���� ���

#define fan1 11
#define fan2 12
#define fan3 13		//�ð��� �ɹ�ȣ
#define DS18_pin 38
#define DHT_pin 39
#define WATER_LEVEL_pin 5	//��������(0, 1)
#define POT1_pin 0
#define POT2_pin 1
#define POT3_pin 2
#define POT4_pin 3		//�������� 1,2,3,4(�Ƴ��α���)
#define RELAY_IN1 28
#define RELAY_IN2 29
#define RELAY_IN3 30
#define RELAY_IN4 31

DHT dht22(DHT_pin, DHT22);
OneWire DS18(DS18_pin);		//��������
byte DS18_addr[8] = { 0 };	//�������� �ּҰ�

int POT_val[5] = { 0, };	//val1, val2, val3, val4, avg
boolean fan_state = 0;		//�ð��� ����. 0 : OFF, 1 : ON
boolean ds18_state = 0;		//�������. 0 : ����X, 1 : ����O

long sensor_previousTime = 0;
long sensor_interval = 5000;		//���� ���� �ð�.

void setup()
{
	pinMode(fan1, OUTPUT);
	pinMode(fan2, OUTPUT);
	pinMode(fan3, OUTPUT);		//�ð��� �ɸ�� : ���
	pinMode(WATER_LEVEL_pin, INPUT);	//���� ���� �ɸ�� : ���
	pinMode(RELAY_IN1, OUTPUT);
	pinMode(RELAY_IN2, OUTPUT);
	pinMode(RELAY_IN3, OUTPUT);
	pinMode(RELAY_IN4, OUTPUT);			//LED����� ������ �ɸ��.
	for (int i = RELAY_IN1; i <= RELAY_IN4; i++) {
		digitalWrite(i, HIGH);
	}

	Serial.begin(9600);
	dht22.begin();
	Connect_DS18();
}

void loop()
{
	delay(2000);	//delay : 2�ʷ� millis�� 2�� �и�.
	float DHT_temp = dht22.readTemperature();
	float DHT_humi = dht22.readHumidity();
	float Water_temp = getTemp();		//��������
	fan_control(Water_temp);			//���¿� ���� �ð��� ����
	read_POT();							//���������� �迭�� ����.
	Relay_Control();

	//���� �ð����� ������ �� ���(�ø���)
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
//--------------------���α׷� �Լ�----------------------
//�������� �� �д� �Լ�.
void read_POT() {
	int temp = 0;		//4�� ���� �� ����.

	for (int i = 0; i < 4; i++) {
		POT_val[i] = analogRead(i);		//�������� ������ ���� ����.
		temp += POT_val[i];
	}
	POT_val[4] = temp / 4;	//4�� �� ���
}

//�������� ���� ���� LED ����� ������ ON/OFF
void Relay_Control() {
	if (POT_val[4] <= 90) 
		for (int i = RELAY_IN1; i <= RELAY_IN4; i++)
			digitalWrite(i, LOW);
	else if(POT_val[4] > 90)
		for (int i = RELAY_IN1; i <= RELAY_IN4; i++)
			digitalWrite(i, HIGH);
}

//DS18 ���� �Լ�
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

//DS18 ���� ���� �Լ�
float getTemp(void) {
	byte data[9] = { 0 };

	if (ds18_state == 0) {
		Serial.println("DS18_not_find!!!");
		return 0.0;
	}		//���¼��� ������ �ȵɰ��,

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
	float TemperatureSum = tempRead / 16;		//������ ���� �����µ��� ��ȯ.

	return TemperatureSum;
}

//�ð��� ���� �Լ�.
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