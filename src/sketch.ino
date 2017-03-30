/*

	TinLight 0.5
	Bjorn W Naucler
	mail@bnaucler.se

	http://etsy.com/shop/Tinkerlight
	http://bnaucler.se

	License: MIT
	See README.md for more information

*/

#include <Adafruit_NeoPixel.h>

// Pin definitions
#define		pixpin 2
#define		sw1pin 8
#define		sw2pin 11

// Global constants
#define		numpx 13
#define		maxbr 255
#define		minbr 0
#define		mincolorint 150
#define		mrand true

// Global variables
int			cmode, pmode = 0;
int			curbr = maxbr;
int			r, g, b;
int			wspix = 0;
int			inclen;
int			cpix;
int			finterval;
int			gdiv[numpx];

bool		incdir = true;
bool		twincr = true;
bool		twpause = true;

// Color schemes
enum		color {
				rnd, off, white,
				warmwhite, red, green,
				blue, yellow, orange
			};

// Timers
unsigned long	timer;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(numpx, pixpin, NEO_GRB + NEO_KHZ800);

int getmode() {

	int mode = 0;

	bool sw1 = digitalRead(sw1pin);
	bool sw2 = digitalRead(sw2pin);

	if(!sw1 && !sw2) mode = 1;
	else if(sw1 && !sw2) mode = 2;
	else if(!sw1 && sw2) mode = 3;
	else if(sw1 && sw2) mode = 4;

	return mode;
}

// TODO: Convert to switch
void setcol(int color) {

	if(color == rnd) {
		r = random(1,255);
		g = random(1,255);
		b = random(1,255);

		if((r + g + b) < mincolorint) setcol(rnd);
	}
	else if(color == white) { r = 255; g = 255; b = 255; }
	else if(color == warmwhite) { r = 255; g = 147; b = 41; }
	else if(color == red) { r = 255; g = 0; b = 0; }
	else if(color == green) { r = 0; g = 255; b = 0; }
	else if(color == blue) { r = 0; g = 0; b = 255; }
	else if(color == yellow) { r = 255; g = 255; b = 0; }
	else if(color == orange) { r = 255; g = 128; b = 0; }
	else { r = 0; g = 0; b = 0; }
}

void setall(int color, int brightness) {

	if(cmode != pmode) {
		setcol(color);
		for(int a = 0; a < numpx; a++) strip.setPixelColor(a, r, g, b);
		strip.setBrightness(brightness);
		strip.show();
		pmode = cmode;
	}
}

// Default: (col), 4, 300
void worm(int color, int len, int interval) {

	if(cmode != pmode) setcol(color); pmode = cmode;

	if(millis() > timer + interval) {

		for(int a = 0; a < numpx; a++) strip.setPixelColor(a, 0, 0, 0);

		for(int b = wspix; b < wspix + len; b++) {
			if(b >= numpx) strip.setPixelColor(b - numpx, r, g, b);
			else strip.setPixelColor(b, r, g, b);
		}

		strip.setBrightness(maxbr);
		strip.show();

		if(++wspix >= numpx) wspix = 0;
		timer = millis();
	}
}

// Default: (col), 60, 130
void flame(int color, int minint, int maxint) {

	if(cmode != pmode) {
		setcol(color);
		pmode = cmode;
		finterval = random(minint, maxint);
	}

	if(millis() > timer + finterval) {
		for(int a = 0; a < numpx; a++) strip.setPixelColor(a, r, g, b);
		strip.setBrightness(random(minbr, maxbr));
		strip.show();
		finterval = random(minint, maxint);
		timer = millis();
	}
}

// Default: (col), 200
void increase(int color, int interval) {

	if(cmode != pmode) {
		setcol(color);
		curbr = maxbr;
		inclen = 1;
		pmode = cmode;
	}

	if(millis() > timer + interval && incdir) {
		for(int a = 0; a < numpx; a++) strip.setPixelColor(a, 0, 0, 0);
		for(int a = 0; a < inclen; a++) strip.setPixelColor(a, r, g, b);

		if(++inclen >= numpx) {
			inclen = 1;
			incdir = !incdir;
		}

		strip.setBrightness(curbr);
		strip.show();
		timer = millis();
	}

	if(millis() > timer + interval && !incdir) {
		for(int a = 0; a < numpx; a++) strip.setPixelColor(a, r, g, b);
		for(int a = 0; a < inclen; a++) strip.setPixelColor(a, 0, 0, 0);

		if(++inclen >= numpx) {
			inclen = 1;
			incdir = !incdir;
			if(color == rnd && mrand) setcol(color);
		}

		strip.setBrightness(curbr);
		strip.show();
		timer = millis();
	}
}

// Default: (col), 10, 100
void twinkle(int color, int interval, int pinterval) {

	if(cmode != pmode) {
		setcol(color);
		cpix = random(0, numpx);
		curbr = minbr;
		for(int a = 0; a < numpx; a++) strip.setPixelColor(a, 0, 0, 0);
		strip.setBrightness(curbr);
		strip.show();
		pmode = cmode;
	}

	if(millis() > timer + interval && twincr && !twpause) {
		strip.setPixelColor(cpix, r, g, b);
		strip.setBrightness(++curbr);
		strip.show();

		if(curbr >= maxbr) twpause = true;
		timer = millis();
	}

	if(millis() > timer + pinterval && twincr && twpause) {
		twpause = false;
		twincr = false;
		timer = millis();
	}

	if(millis() > timer + interval && !twincr && !twpause) {
		strip.setPixelColor(cpix, r, g, b);
		strip.setBrightness(--curbr);
		strip.show();

		if(curbr <= minbr) twpause = true;
		timer = millis();
	}

	if(millis() > timer + pinterval && !twincr && twpause) {
		twpause = false;
		twincr = true;
		cpix = random(0, numpx);
		if(color == rnd && mrand) setcol(color);
		timer = millis();
	}
}

// Default: (col), 50, 2, 2, 50
void glow(int color, int interval, int change, int mindiv, int maxdiv) {

	if(millis() > timer + interval) {

		if(cmode != pmode) {
			setcol(color);
			pmode = cmode;
		}

		for(int a = 0; a < numpx; a++) {

			bool incr = random(2);
			bool dochng = random(2);

			if(incr && dochng) {
				gdiv[a] = gdiv[a] + change;
				if(gdiv[a] > maxdiv) gdiv[a] = maxdiv;
			}

			else if(!incr && dochng) {
				gdiv[a] = gdiv[a] - change;
				if(gdiv[a] < mindiv) gdiv[a] = mindiv;
			}

			strip.setPixelColor(a,
			(r * gdiv[a]) / 100,
			(g * gdiv[a]) / 100,
			(b * gdiv[a]) / 100);
		}

		strip.setBrightness(maxbr);
		strip.show();
		timer = millis();
	}
}

void setup() {

	randomSeed(analogRead(A0));

	strip.begin();
	strip.show();

	pinMode(pixpin, OUTPUT);
	pinMode(sw1pin, INPUT_PULLUP);
	pinMode(sw2pin, INPUT_PULLUP);
}

void loop() {

	cmode = getmode();

	if(cmode == 1) setall(warmwhite, 255);
	if(cmode == 2) setall(rnd, 150);
	if(cmode == 3) glow(red, 50, 2, 2, 50);
	if(cmode == 4) twinkle(rnd, 10, 100);
}
