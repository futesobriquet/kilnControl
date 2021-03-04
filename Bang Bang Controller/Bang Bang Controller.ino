/*
 Name:		Bang_Bang_Controller.ino
 Created:	2/23/2021 8:14:24 PM
 Author:	jbesancon
*/
//double topSectionDutyCycle = 100;
//double bottomSectionDutyCycle = 50;
//double timebase = 10/60; //in minutes. dictates the update rate of the entire control algorithm

//includes
#include <SparkFun_MCP9600.h>

//process variables
int const segments = 5; //You MUST update segments to match how many segments will be in your profile
//double profile[segments][3] = { {250, 135, 1}, { 1000, 225, 1 }, {1100,60,1},{1570,150,1},{1820,180,1} }; //bisque
double profile[segments][3] = { {250, 1, 1}, { 1000, 2, 1 }, {1100,1,1},{1570,1,1},{1820,1,1} }; //bisque sw test
double highHysteresis = 5;
double lowHysteresis = 5;

//hardware variables
int topHeaterPin = 13;
int bottomHeaterPin = 12;
MCP9600 tempSensor;


//code variables
double startTime;
double startTemp;
int index;

//serial
double lastUpdate = 0;

void setup() {

	//Serial
	Serial.begin(115200);
	Serial.println("Serial started");

	//Thermocouple
	Wire.begin();
	Wire.setClock(100000);
	tempSensor.begin();
	if (tempSensor.isConnected()) {
		Serial.println("Thermocouple connected");
	}

	//Heaters
	pinMode(topHeaterPin, OUTPUT);
	pinMode(bottomHeaterPin, OUTPUT);

	//Process Code
	startTime = millis();
	startTemp = readTemp();
	index = 0;
}

void loop() {
	delay(20);//delay to avoid overrunning the i2c bus
	if (index <= segments - 1)
	{
		//index profile
		double setpoint;
		double endTemp = profile[index][0];
		double duration = profile[index][1];
		double rampBit = profile[index][2];
		//checktime convert to minutes
		double timeElapsed = (millis() - startTime) / 60000;

		if (timeElapsed <= duration)
		{
			if (rampBit == 1)
			{
				//if in a ramp segment calculate what setpoint we should be at our current time
				setpoint = startTemp + (timeElapsed / duration * (endTemp - startTemp));
			}
			else
			{
				//in a soak not a ramp, the setpoint is straightforward
				setpoint = endTemp;
			}
			updateHeaters(setpoint, highHysteresis, lowHysteresis);
		}
		else
		{
			index++;
			startTime = millis();
			startTemp = readTemp();
		}

		//serial update
		if ((millis() - lastUpdate) >= 10000)
		{
			Serial.print("Temperature: ");
			Serial.print(readTemp());
			Serial.print("F ");
			Serial.print("Setpoint: ");
			Serial.print(setpoint);
			Serial.print("F ");
			Serial.print("Ambient: ");
			Serial.print(readAmbient());
			Serial.print("F ");
			Serial.print("Time: ");
			Serial.print(timeElapsed);
			Serial.print("min ");
			Serial.print("Segment end time: ");
			Serial.print(duration);
			Serial.print("min ");
			Serial.print("Segment end temp: ");
			Serial.print(endTemp);
			Serial.print("F ");
			Serial.print("Segments remaining: ");
			Serial.println(segments-index-1);
			
			lastUpdate = millis();
		}
	}
	else
	{
		heatersOff();
		Serial.print("Profile Finished. Temperature: ");
		Serial.print(readTemp());
		Serial.print("F ");
		Serial.print("Ambient: ");
		Serial.print(readAmbient());
		Serial.println("F");
		delay(100000);
	}
}

void updateHeaters(double setpoint, double highHysteresis, double lowHysteresis) {
	double currentTemp = readTemp();
	if (currentTemp >= (setpoint + highHysteresis))
	{
		heatersOff();
	}
	if (currentTemp <= (setpoint - lowHysteresis))
	{
		heatersOn();
	}
}

double readTemp() {
	return tempSensor.getThermocoupleTemp(false);
}

double readAmbient() {
	return tempSensor.getAmbientTemp(false);
}

void heatersOn() {
	digitalWrite(topHeaterPin, HIGH);
	digitalWrite(bottomHeaterPin, HIGH);
}

void heatersOff() {
	digitalWrite(topHeaterPin, LOW);
	digitalWrite(bottomHeaterPin, LOW);
}