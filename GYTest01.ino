/*
	Name:       GYTest01.ino
	Created:	05-Oct-19 6:11:44 PM
	Author:		Gabriela Yontcheva
*/

#include <Wire.h> //common, used by sd
#include <SD.h> //sd card
#include <Adafruit_SSD1306.h> //display
#include <Adafruit_BME280.h> //thermo
#include <SoftwareSerial.h> //gps communication
#include <TinyGPS++.h> //gps library

#if (SSD1306_LCDHEIGHT != 64) //ensure the fix of Adafruit library, do not remove!
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

#define VERSION_TAG "v1.0"
#define DEVICE_NAME "The Device"
#define AUTHOR_NAME "2019, Yontcheva, 11a"
#define GMT_OFFSET 2 /*GMT+2*/
#define FILE_NAME "track.gpx"

//libraries
Adafruit_SSD1306 display; //display
Adafruit_BME280 bme; //thermo
SoftwareSerial softSerial(D4, D0); //gps communication
TinyGPSPlus gps; //gps related functions

//global variables
int prevState; //UI related
unsigned long thermoLastMeasureTime;
bool doDisplayRefresh;
bool recordMode;

//gps related variables
bool gpsReady;
double gpsSpeed;
double gpsDistance;
double gpsLat, lat0;
double gpsLng, lng0;
int32_t gpsSatellitesInUse;
String gpsTimeString;

//image, PROGMEM is to save some space, put in flash
const unsigned char tues[] PROGMEM = {
	// 'tuesmh', 128x48px
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x18, 0x7f, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x18, 0x7f, 0x86, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x18, 0x01, 0x87, 0xc0, 0x01, 0xc6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x18, 0x01, 0x87, 0x00, 0x00, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x18, 0x01, 0x87, 0x00, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x18, 0x01, 0x86, 0x00, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x18, 0x7f, 0x86, 0x1f, 0xe1, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x18, 0x7f, 0x86, 0x1f, 0xf1, 0x86, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x18, 0x01, 0x86, 0x00, 0x31, 0x87, 0xc0, 0x30, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x18, 0x01, 0x86, 0x00, 0x31, 0x87, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x18, 0x03, 0x86, 0x00, 0x31, 0x87, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x18, 0x07, 0x86, 0x00, 0x31, 0x86, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0x87, 0xf8, 0x31, 0x86, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x1f, 0xf9, 0x87, 0xfc, 0x30, 0x86, 0x1f, 0xf0, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x30, 0x06, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x70, 0x06, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x78, 0x06, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x01, 0xcf, 0x86, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0x83, 0x87, 0xfc, 0x30, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xfe, 0x00, 0x87, 0xfc, 0x30, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x01, 0xc0, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

//once
void setup() {
	Serial.begin(115200); //for debug print
	Serial.println("\nSetup ...");

	if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) //initialize i2c
	{
		Serial.println("display.begin error ...");
	}

	if (!bme.begin(0x76)) //initialize i2c
	{
		Serial.println("bme.begin error ...");
	}

	if (!SD.begin(D8)) //cs pin
	{
		Serial.println("SD.begin error ...");
	}

	softSerial.begin(9600); //initialize serial for gps
	softSerial.listen();

	displayLogo(4000); //for about 4s
	doDisplayRefresh = true;

	//initialize defaults
	display.setTextSize(2);
	display.setTextColor(WHITE);
	pinMode(D3, INPUT); //the button pin

	lat0 = 0; //initial values for first point
	lng0 = 0;
}

//measures the time passed since ts
unsigned long millisSince(unsigned long ts)
{
	return (unsigned long)(millis() - ts);
}

//displays the intro page for mills milliseconds
void displayLogo(int mills)
{
	display.clearDisplay();
	yield();
	
	display.drawBitmap(0, 16, tues, 128, 48, WHITE);
	display.setTextSize(1);
	display.setTextColor(WHITE);
	yield();
	
	display.setCursor(0, 0); //first line
	display.print(DEVICE_NAME);
	yield();
	
	display.setCursor(0, 8); //second line
	display.print(AUTHOR_NAME);
	yield();
	
	display.setCursor(104, 56);
	display.println(VERSION_TAG); //low right
	yield();
	
	display.display();
	yield();

	//wait for mills, avoid using of delay!
	unsigned long startWait = millis();
	while (millisSince(startWait) < mills)
	{
		yield();
	}
}

void loop() {
	int state = digitalRead(D3); //reads the button state
	if (state == 0 && prevState == 1) //detects front edge
	{
		recordMode = !recordMode;

		Serial.println("Pressed ...");
		lat0 = gpsLat; //stores current gps point
		lng0 = gpsLng;

		File dataFile = SD.open(FILE_NAME, FILE_WRITE);
		if (dataFile)
		{
			if (recordMode)
			{
				Serial.println("SD.open begin write ...");
				dataFile.truncate(0);
				dataFile.println("<?xml version=\"1.0\" encoding=\"UTF - 8\"?><gpx>");
			}
			else
			{
				Serial.println("SD.open end write ...");
				dataFile.println("</gpx>");
			}
			dataFile.close();
		}
		else {
			Serial.println("SD.open write error ...");
		}

	}
	prevState = state;

	//debug
	//if (...)
	//{
	//	bme.takeForcedMeasurement();

	//	File dataFile = SD.open(FILE_NAME, FILE_READ);
	//	if (dataFile)
	//	{
	//		Serial.println("SD.open read OK ...");

	//		int x = dataFile.parseInt();

	//		/*while (dataFile.available()) {
	//			Serial.write(dataFile.read());
	//		}*/

	//		Serial.println(x);
	//		dataFile.close();
	//	}
	//	else {
	//		Serial.println("SD.open read error ...");
	//	}
	//}

	if (doDisplayRefresh)
	{
		doDisplayRefresh = false; //got it

		display.clearDisplay();
		yield();

		//first row
		display.setCursor(0, 0);
		String distance;
		if (lat0 == 0 && lng0 == 0) //still not initialized
		{
			distance = "0";
		}
		else
		{
			distance = String((int)gpsDistance);
		}

		distance = distance + " m";

		while (distance.length() < 7)
		{
			distance = distance + " ";
		}

		distance = distance + gpsSatellitesInUse;
		if (recordMode)
		{
			distance = distance + "+";
		}

		display.println(distance);
		yield();

		//second row
		display.setCursor(0, 16);
		display.println(String(bme.readTemperature()) + " " + (char)247 + "C"); //celsius degrees
		yield();

		//third row
		display.setCursor(0, 32);
		display.println(gpsTimeString);
		yield();

		//fourth row
		display.setCursor(0, 48);
		display.println(String(gpsSpeed) + " km/h");
		yield();

		display.display();
		yield();

		if (recordMode && gpsReady)
		{
			File dataFile = SD.open(FILE_NAME, FILE_WRITE);
			if (dataFile)
			{
				Serial.println("SD.open log write ...");
				
				dataFile.print("<wpt lat=\"");
				
				dataFile.print(gps.location.rawLat().negative ? "-" : "+");
				dataFile.print(gps.location.rawLat().deg);
				dataFile.print(".");
				dataFile.print(gps.location.rawLat().billionths);
				
				dataFile.print("\" lon=\"");

				dataFile.print(gps.location.rawLng().negative ? "-" : "+");
				dataFile.print(gps.location.rawLng().deg);
				dataFile.print(".");
				dataFile.print(gps.location.rawLng().billionths);
				
				dataFile.println("\"/>");

				gpsReady = false; //got it
				dataFile.close();
			}
			else {
				Serial.println("SD.open write error ...");
			}
		}
	}

	//read gps data
	while (softSerial.available() > 0) //have something to read
	{
		yield();
		byte gpsData = softSerial.read();
		yield();
		gps.encode(gpsData);
		yield();
		if (gps.location.isUpdated()) //have complete data
		{
			gpsLat = gps.location.lat();
			gpsLng = gps.location.lng();
			gpsSpeed = gps.speed.kmph();
			gpsSatellitesInUse = gps.satellites.value();

			char str[12];
			sprintf(str, "%02d:%02d:%02d", (gps.time.hour() + GMT_OFFSET) % 24, gps.time.minute(), gps.time.second());
			gpsTimeString = String(str);
			gpsDistance = gps.distanceBetween(lat0, lng0, gpsLat, gpsLng);
			gpsReady = true;
			doDisplayRefresh = true;
		}
	}

	if (millisSince(thermoLastMeasureTime) > 2000) //every 2 seconds
	{
		bme.takeForcedMeasurement();
		thermoLastMeasureTime = millis();
		doDisplayRefresh = true;
		yield();
	}
}