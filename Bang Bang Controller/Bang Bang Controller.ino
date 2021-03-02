/*
 Name:		Bang_Bang_Controller.ino
 Created:	2/23/2021 8:14:24 PM
 Author:	jbesancon
*/
//double topSectionDutyCycle = 100;
//double bottomSectionDutyCycle = 50;
//double timebase = 10/60; //in minutes. dictates the update rate of the entire control algorithm

//process variables
int const segments = 5; //You MUST update segments to match how many segments will be in your profile
double profile[segments][3] = { {250, 135, 1}, { 1000, 225, 1 }, {1100,60,1},{1570,150,1},{1820,180,1} };
double highHysteresis = 10;
double lowHysteresis = 10;

//hardware variables
//topHeaterPin
//bottomHeterPin 


//code variables
double startTime;
double startTemp;
int index;

//serial
double lastUpdate = 0;

void setup() {
	// put your setup code here, to run once:
	startTime = millis();
	startTemp = readTemp();
	index = 0;
	//Serial
	Serial.begin(9600);
}

void loop() {
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
				setpoint = startTemp + (timeElapsed / duration * (endTemp-startTemp));
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
		
		//serial
		if ((millis() - lastUpdate) >= 10000)
		{
			updateSerial(readTemp(), setpoint, timeElapsed, duration);
			lastUpdate = millis();
		}
	}
	else 
	{
		Serial.println(readTemp());
		delay(100000);
	}
}

void updateHeaters(double setpoint, double highHysteresis, double lowHysteresis) {
	double currentTemp = readTemp();
	if(currentTemp >= (setpoint + highHysteresis))
	{
		heatersOff();
	}
	if (currentTemp <= (setpoint - lowHysteresis))
	{
		heatersOn();
	}
}

double readTemp() {

}

void heatersOn() {

}

void heatersOff() {
}

void updateSerial(double temperature, double setpoint, double time, double duration) {
	Serial.print("Temperature: ");
	Serial.print(temperature);
	Serial.print("F ");
	Serial.print("Setpoint: ");
	Serial.print(setpoint);
	Serial.print("F ");
	Serial.print("Time: ");
	Serial.print(time);
	Serial.print("min ");
	Serial.print("End time: ");
	Serial.print(duration);
	Serial.println("min ");
}