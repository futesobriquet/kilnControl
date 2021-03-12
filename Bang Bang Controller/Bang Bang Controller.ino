/*
 Name:		Bang_Bang_Controller.ino
 Created:	2/23/2021 8:14:24 PM
 Author:	jbesancon
*/

//includes
#include <Wire.h>
#include <SparkFun_MCP9600.h>
#include <SparkFun_Alphanumeric_Display.h> //Click here to get the library: http://librarymanager/All#Alphanumeric_Display by SparkFun
#include <MsTimer2.h>

//process variables
//BISQUE SETTINGS
//int const segments = 5; //You MUST update segments to match how many segments will be in your profile
//double profile[segments][3] = { {250, 135, 1}, { 1000, 225, 1 }, {1100,60,1},{1570,150,1},{1820,180,1} }; //Cone 06 bisque

//GLAZE SETTINGS
int const segments = 3; //You MUST update segments to match how many segments will be in your profile
double profile[segments][3] = { {250, 120, 1}, { 2167, 360, 1 }, {2167, 30, 0 } }; //Cone 6 glaze

double highHysteresis = 5;
double lowHysteresis = 5;

//hardware variables
int topHeaterPin = 8;
int bottomHeaterPin = 13; //12
MCP9600 tempSensor;
HT16K33 display;

//code variables
double startTime;
double startTemp;
int index;
boolean heatGlobal = LOW;

//serial variables
double lastUpdate = 0;

void setup() {

	//Serial
	Serial.begin(115200);
	Serial.println("Serial started");

	//Thermocouple
	Wire.begin();
	Wire.setClock(100000);
	tempSensor.begin();
	Thermocouple_Type tcType = TYPE_K;
	tempSensor.setThermocoupleType(tcType);
	if (tempSensor.isConnected()) {
		Serial.print("Thermocouple connected. Type:");
		if (tempSensor.getThermocoupleType() == tcType)
		{
			Serial.println("K");
		}
		else {
			Serial.println("UNKNOWN");
		}
	}

	//Displays
	if (display.begin() == true) {
		Serial.println("Display connected");
	}


	//Heaters
	pinMode(topHeaterPin, OUTPUT);
	pinMode(bottomHeaterPin, OUTPUT);
	//start PWMing bottom heater
	MsTimer2::set(1000, toggleBottomHeater);
	MsTimer2::start();

	//Process Code
	startTime = millis();
	startTemp = readTemp();
	index = 0;
}

void loop() {
	
	
	delay(20);//delay to avoid overrunning the i2c bus
	//display temp
	int temp = (int) readTemp();
	display.print(temp);

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
		//stop pwming
		MsTimer2::stop();

		Serial.print("Profile Finished. Temperature: ");
		Serial.print(readTemp());
		Serial.print("F ");
		Serial.print("Ambient: ");
		Serial.print(readAmbient());
		Serial.println("F");
		delay(10000);
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
	heatGlobal = HIGH; //This sets the bottom heater PWM to high
	digitalWrite(topHeaterPin, HIGH);
}

void heatersOff() {
	heatGlobal = LOW; //This sets the bottom heater PWM to low
	digitalWrite(topHeaterPin, LOW);
}

//low levels, do not call directly from main, only from mid level functions
void toggleBottomHeater(){
	static boolean toggle = HIGH;
	digitalWrite(bottomHeaterPin, (toggle&&heatGlobal));
	toggle = !toggle;
}