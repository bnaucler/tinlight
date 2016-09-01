/*

	TinLight 0.5
	Bjorn W Naucler
	mail@bnaucler.se

	http://etsy.com/shop/Tinkerlight
	http://bnaucler.se

	Absolutely no rights reserved.
	Feel free to copy and modify the code to your liking.
	I'm curious to see what you come up with!

	TODO:
	* More modes!
		Disco?
	* Enum for brightness levels?
	* Improve flame mode by individual pixel movement?
		Or merge with glow?
	* Reduce code footprint for 'twinkle' and 'increase'

*/

#include <Adafruit_NeoPixel.h>

// Pin definitions
const int		pixelPin = 7;
const int		sw1Pin = 2;
const int		sw2Pin = 8;

// Global constants
const int		modes = 4;					// Number of modes - static
const int		numPixels = 13;				// Number of pixels - static
const int		maxBrightness = 255;
const int		minBrightness = 0;
const int		minColorIntensity = 150;	// When randomly selecting color 

const boolean	moreRandom = true;			// Set to true for more color change

// 'Worm' mode constants
const int		wormInterval = 300;			// Worm speed
const int		wormLength = 4;				// Worm length in pixels

// 'Flame' mode constants
const int		flameIntervalMin = 60;		// Maximal flame speed
const int		flameIntervalMax = 130;		// Minimal flame speed

// 'Glow' mode constants
const int		glowInterval = 50;			// Glow speed
const int		glowDivMin = 2;				// Lowest glow level
const int		glowDivMax = 50;			// Highest glow level
const int		glowDivDiff = 2;			// Difference per pixel and cycle
const int		glowMultiplier = 100;		// Avoid floating point calculations

// 'Increase' mode constants
const int		increaseInterval = 200;		// Increase / decrease speed

// 'Twinkle' mode constants
const int		twinkleInterval = 10;		// Twinkle speed
const int		twinklePauseInterval = 100;	// Time to pause at max and min

// Switch states
boolean			sw1Read, sw2Read;

// Variable declarations
int				currentMode = 1, previousMode = 2;
int				currentBrightness = maxBrightness;
int				color1, color2, color3;
int				wormStartPixel = 0;
int				ledNumber;
int				increaseLength;
int				twinkleCurrentPixel;
int				flameInterval;
int				glowDiv[numPixels];

boolean			glowIncrease, glowDoChange;
boolean			increaseDirection = true;
boolean			twinkleIncrease = true;
boolean			twinklePause = true;

// Color schemes
enum			color { 
					rnd, off, white, 
					warmwhite, red, green, 
					blue, yellow, orange
				};

// Timers
unsigned long	timer;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(numPixels, pixelPin, NEO_GRB + NEO_KHZ800);

void readSensors()
{
	sw1Read = digitalRead(sw1Pin);
	sw2Read = digitalRead(sw2Pin);

	if( !sw1Read && !sw2Read )	{ currentMode = 1; }
	if( sw1Read && !sw2Read )	{ currentMode = 2; }
	if( !sw1Read && sw2Read )	{ currentMode = 3; }
	if( sw1Read && sw2Read )	{ currentMode = 4; }
}

int setColor(int color)
{
	if(color == rnd) { 
		color1 = random(1,255);
		color2 = random(1,255);
		color3 = random(1,255);

		if((color1 + color2 + color3) < minColorIntensity) { setColor(rnd); }
	}

	if(color == off) { color1 = 0; color2 = 0; color3 = 0; }
	if(color == white) { color1 = 255; color2 = 255; color3 = 255; }
	if(color == warmwhite) { color1 = 255; color2 = 147; color3 = 41; }
	if(color == red) { color1 = 255; color2 = 0; color3 = 0; }
	if(color == green) { color1 = 0; color2 = 255; color3 = 0; }
	if(color == blue) { color1 = 0; color2 = 0; color3 = 255; }
	if(color == yellow) { color1 = 255; color2 = 255; color3 = 0; }
	if(color == orange) { color1 = 255; color2 = 128; color3 = 0; }
}

int setAll(int color, int brightness)
{
	if(currentMode != previousMode) {
		setColor(color);
		for(int a = 0; a <= numPixels; a++) {
			strip.setPixelColor(a, color1, color2, color3);	
		}
		strip.setBrightness(brightness);
		strip.show();
		previousMode = currentMode;
	}
}

int worm(int color)
{
	if(currentMode != previousMode) { setColor(color); previousMode = currentMode; }

	if(millis() > timer + wormInterval)
	{
		for(int a=0; a <= numPixels; a++) { strip.setPixelColor(a,0,0,0); }
		for(int b=wormStartPixel; b < wormStartPixel + wormLength; b++) {
			if(b >= numPixels) { ledNumber = b - numPixels; } else { ledNumber = b; }
			strip.setPixelColor(ledNumber, color1, color2, color3);
		}

		strip.setBrightness(maxBrightness);
		strip.show();

		wormStartPixel++;
		if(wormStartPixel >= numPixels) { wormStartPixel = 0; }
		timer = millis();
	}
}

int flame(int color)
{
	if(currentMode != previousMode) {
		setColor(color);
		previousMode = currentMode;
		flameInterval = random(flameIntervalMin, flameIntervalMax);
	}

	if(millis() > timer + flameInterval)
	{
		for(int a=0; a <= numPixels; a++) { strip.setPixelColor(a, color1, color2, color3); }
		strip.setBrightness(random(minBrightness,maxBrightness));
		strip.show();
		flameInterval = random(flameIntervalMin, flameIntervalMax);
		timer = millis();
	}
}

int increase(int color)
{
	if(currentMode != previousMode) { 
		setColor(color);
		currentBrightness = maxBrightness;
		increaseLength = 1;
		previousMode = currentMode; 
	}

	if(millis() > timer + increaseInterval && increaseDirection)
	{
		for(int a=0; a <= numPixels; a++) { strip.setPixelColor(a, 0, 0, 0); }
		for(int a=0; a < increaseLength; a++) { 
			strip.setPixelColor(a, color1, color2, color3); 
		}

		increaseLength++;
		if(increaseLength >= numPixels) { 
			increaseLength = 1; 
			increaseDirection = !increaseDirection;
		}

		strip.setBrightness(currentBrightness);
		strip.show();
		timer = millis();
	}

	if(millis() > timer + increaseInterval && !increaseDirection)
	{
		for(int a=0; a <= numPixels; a++) { 
			strip.setPixelColor(a, color1, color2, color3); 
		}
		for(int a=0; a < increaseLength; a++) { strip.setPixelColor(a, 0, 0, 0); }

		increaseLength++;

		if(increaseLength >= numPixels) { 
			increaseLength = 1; 
			increaseDirection = !increaseDirection;
			if(color == rnd && moreRandom) { setColor(color); } 
		}
		
		strip.setBrightness(currentBrightness);
		strip.show();
		timer = millis();
	}
}

int twinkle(int color)
{
	if(currentMode != previousMode) { 
		setColor(color);
		twinkleCurrentPixel = random(0,numPixels);
		currentBrightness = minBrightness;
		for(int a=0; a <= numPixels; a++) { strip.setPixelColor(a, 0, 0, 0); }
		strip.setBrightness(currentBrightness);
		strip.show();
		previousMode = currentMode; 
	}

	if(millis() > timer + twinkleInterval && twinkleIncrease && !twinklePause)
	{
		currentBrightness++;

		strip.setPixelColor(twinkleCurrentPixel, color1, color2, color3); 
		strip.setBrightness(currentBrightness);
		strip.show();

		if(currentBrightness >= maxBrightness) { twinklePause = true; }
		timer = millis();
	}

	if(millis() > timer + twinklePauseInterval && twinkleIncrease && twinklePause)
	{ 
		twinklePause = false;
		twinkleIncrease = false;
		timer = millis();
	}

	if(millis() > timer + twinkleInterval && !twinkleIncrease && !twinklePause)
	{
		currentBrightness--;

		strip.setPixelColor(twinkleCurrentPixel, color1, color2, color3); 
		strip.setBrightness(currentBrightness);
		strip.show();

		if(currentBrightness <= minBrightness) { twinklePause = true; }
		timer = millis();
	}

	if(millis() > timer + twinklePauseInterval && !twinkleIncrease && twinklePause)
	{
		twinklePause = false;
		twinkleIncrease = true;
		twinkleCurrentPixel = random(0,numPixels);
		if(color == rnd && moreRandom) { setColor(color); } 
		timer = millis();
	}
}

int glow(int color)
{
	if(currentMode != previousMode) { setColor(color); previousMode = currentMode; }

	if(millis() > timer + glowInterval)
	{
		for(int a=0; a <= numPixels; a++)
		{
			glowIncrease = random(2);
			glowDoChange = random(2);

			if(glowIncrease && glowDoChange) {
				glowDiv[a] = glowDiv[a] + glowDivDiff; 
				if(glowDiv[a] > glowDivMax) { glowDiv[a] = glowDivMax; }
			}

			else if (!glowIncrease && glowDoChange) {
				glowDiv[a] = glowDiv[a] - glowDivDiff;
				if(glowDiv[a] < glowDivMin) { glowDiv[a] = glowDivMin; }
			}
			
			strip.setPixelColor(a,
			(color1 * glowDiv[a]) / glowMultiplier,
			(color2 * glowDiv[a]) / glowMultiplier,
			(color3 * glowDiv[a]) / glowMultiplier
			); 
		}

		strip.setBrightness(maxBrightness);
		strip.show();
		timer = millis(); 
	}
}

void setup()
{
	randomSeed(analogRead(A0));

	strip.begin();
	strip.show();

	pinMode(pixelPin, OUTPUT);
	pinMode(sw1Pin, INPUT);
	pinMode(sw2Pin, INPUT);
}

void loop()
{ 
	readSensors();

	if(currentMode == 1) { setAll(warmwhite, 255); }
	if(currentMode == 2) { setAll(yellow, 150); }
	if(currentMode == 3) { glow(rnd); }
	if(currentMode == 4) { twinkle(rnd); }
}
