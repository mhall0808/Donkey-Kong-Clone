/**
 * MARK HALL
 *
 * DONKEY KONG CLONE
 *
 * ECEN 260
 *
 * This is a Donkey Kong clone game for my ECEN 260 final project.
 */

#include "drawLibrary.h"

void main(void) {
	WDTCTL = WDTPW + WDTHOLD;           // Stop watchdog timer

	waitMS(300);                        // Wait a bit.


	initClk();							// Init clock to 16 MHz
	initPins();							// Init Pin Functionality
	initUSCI();							// Init USCI (SPI)
	initLCD();							// Init LCD Controller
	fillScreen(WHITE);

	TACCTL0 = 0x0010;                   // Turn on the interrupt enable, I believe.
	TACCR0 = 0x5000;                    // Timer that counts down.  When timer is complete, interrupt.
	TACTL = 0x02B0;



	// Beginning sequence.  Simple, yet works well.
	while (adcZ < 600) {

		drawString(120, 300, BLK, "Hello! Press the         ");
		drawString(120, 172, BLK, " screen to begin!          ");
		channelCheck();
	}

	//clear the screen, then go into the opening load, bricks, characters, etc.
	fillScreen(WHITE);
	openingLoad();

	_BIS_SR(GIE);               // Interrupt Enabled

	/**
	 * Main loop:
	 *
	 * These functions all need to be updated regularly.
	 */
	while (1) {
		collisionDetection();
		drawInteger(8, 150, BLK, score);
		if (joystickSpeed == 0) {  // This check is linked to the timer so everything syncs together.
			joystickCheck();
			fallCheck();
			enemyMovementCheck();
			enemyFallCheck();
		}
		badGuyHeadCheck();
		buttonCheck();
		victoryConditionCheck();
	}
}

/**
 * This is to be created mainly for the while loop.  However, it will be used multiple times,
 * so moving it out of the while loop and into another function is necessary.
 */
void channelCheck() {
	/**
	 * Read the Z Channel
	 */
	ADC10CTL0 = 0x1010;
	ADC10CTL1 = INCH_1;

	P1DIR = TS_YM + TS_XP;
	P1OUT = TS_YM;
	ADC10AE0 = TS_YP;

	ADC10CTL0 |= 0x0003;	// Turn on the reading enable, until it is ready.
	while (ADC10CTL1 & 0x0001)
		;             // Wait if ADC10 core is active
	int zY = ADC10MEM;

	ADC10CTL0 = 0x1010;
	ADC10CTL1 = INCH_0;

	ADC10AE0 = TS_XM;
	ADC10CTL0 |= 0x0003;	// Turn on the reading enable, until it is ready.
	while (ADC10CTL1 & 0x0001)
		;             // Wait if ADC10 core is active
	int zX = ADC10MEM;

	adcZ = 1023 - (zY - zX);

}

/**
 * I've decided to go simple on this.
 * 		IF the enemy is on lines 1, 3, 10, move right.
 * 		ELSE move left.
 *
 * 	Simple, right?
 */
void enemyMovementCheck() {

	int i = 0;
	// Go through all enemies.
	for (i; i < 8; i++) {
		// since this is an array and not a vector, I set a home position where nothing is drawn
		if (badHead[i].xCoords != 0) {
			// Next, we need to check if the enemy is on a brick or ladder.  If not, don't move.
			if (ladderCheck(badHead[i].yCoords, badHead[i].xCoords + 16)
					|| brickCheck(badHead[i].xCoords + 16,
							badHead[i].yCoords)) {
				// If the bad guy is on these lines, move right
				if ((badHead[i].xCoords == 1 * 16)
						|| (badHead[i].xCoords == 3 * 16)
						|| (badHead[i].xCoords == 10 * 16)) {
					clear16(badHead[i].yCoords, badHead[i].xCoords);

					if (ladderCheck(badHead[i].yCoords, badHead[i].xCoords)) {
						draw16(badHead[i].yCoords, badHead[i].xCoords, ladder,
								'l');
					}

					badHead[i].yCoords += -16;
					draw16(badHead[i].yCoords, badHead[i].xCoords, badGuyHead,
							'e');

					// else, move left
				} else {
					clear16(badHead[i].yCoords, badHead[i].xCoords);

					if (ladderCheck(badHead[i].yCoords, badHead[i].xCoords)) {
						draw16(badHead[i].yCoords, badHead[i].xCoords, ladder,
								'l');
					}

					if ((badHead[i].yCoords + 16) > 320) {
						badHead[i].yCoords = 0;
						badHead[i].xCoords = 0;
					} else {
						badHead[i].yCoords += 16;
					}
					draw16(badHead[i].yCoords, badHead[i].xCoords, badGuyHead,
							'e');
				}
			}
		}
	}
}

/**
 * FALL CHECK
 *
 * This function is simple:  If a player is on an air space, then you fall.  Keep doing
 * this until you hit the ground.
 */
void fallCheck() {
	if (ladderCheck(playerY, playerX + 16) || brickCheck(playerX + 16, playerY))
		;
	else if (jumpSpeed == 0) {
		clear16(playerY, playerX);
		if (ladderCheck(playerY, playerX)) {
			draw16(playerY, playerX, ladder, 'l');
		}
		playerX += 16;
		draw16(playerY, playerX, player, 'a');
	}
}

/**
 * ENEMY FALL CHECK
 * 	Same as fall check, but with a for loop.  There was a problem with the enemies falling funny,
 * 	so I just added a second function to fix this.
 */
void enemyFallCheck() {
	int i = 0;
	for (i; i < 8; i++) {
		if (badHead[i].xCoords != 0) {
			if (ladderCheck(badHead[i].yCoords, badHead[i].xCoords + 16)
					|| brickCheck(badHead[i].xCoords + 16, badHead[i].yCoords))
				;
			else {
				{
					clear16(badHead[i].yCoords, badHead[i].xCoords);
					badHead[i].xCoords += 16;
					draw16(badHead[i].yCoords, badHead[i].xCoords, badGuyHead,
							'e');
				}
			}
		}
	}
}

/**
 * JOYSTICK CHECK
 * 		For this function, it runs the adc to see if the voltage has changed.
 * 		if it has, then move either right, left, up or down.
 */
void joystickCheck() {

	ADC10CTL0 = 0x1010;
	ADC10CTL1 = INCH_4;
	ADC10AE0 = 0x10;
	ADC10CTL0 |= 0x0003; // Turn on the reading enable, until it is ready.
	while (ADC10CTL1 & 0x0001)
		;             // Wait if ADC10 core is active
	joystickValue = ADC10MEM;

	// Move right
	if ((joystickValue > 800) && (brickCheck(playerX, playerY - 16) == 0)) {
		clear16(playerY, playerX);
		// if there is a ladder, redraw
		if (ladderCheck(playerY, playerX)) {
			draw16(playerY, playerX, ladder, 'l');
		}
		// wrap function
		if ((playerY - 16) < 0) {
			playerY = 340;
		} else {
			playerY += -16;
		}
		draw16(playerY, playerX, player, 'a');

		// Move left
	} else if (joystickValue < 250
			&& (brickCheck(playerX, playerY + 16) == 0)) {
		clear16(playerY, playerX);
		// ladder check
		if (ladderCheck(playerY, playerX)) {
			draw16(playerY, playerX, ladder, 'l');
		}
		// wrap function
		if ((playerY + 16) > 340) {
			playerY = 0;
		} else {
			playerY += 16;
		}
		draw16(playerY, playerX, player, 'a');
	}


	// up/down axis check
	ADC10CTL0 = 0x1010;
	ADC10CTL1 = INCH_6;
	ADC10AE0 = 0x10;
	ADC10CTL0 |= 0x0003; // Turn on the reading enable, until it is ready.
	while (ADC10CTL1 & 0x0001)
		;             // Wait if ADC10 core is active
	joystickValue = ADC10MEM;

	// move up and replace the ladder
	if ((joystickValue > 600) && (ladderCheck(playerY, playerX))) {
		clear16(playerY, playerX);
		draw16(playerY, playerX, ladder, 'l');
		playerX += -16;
		draw16(playerY, playerX, player, 'a');

		// move down and replace the ladder
	} else if ((joystickValue < 250) && ladderCheck(playerY, playerX + 16)) {
		clear16(playerY, playerX);
		if (ladderCheck(playerY, playerX) == 1)
			draw16(playerY, playerX, ladder, 'l');
		playerX += 16;
		draw16(playerY, playerX, player, 'a');
	}
}

void initClk(void) {
	BCSCTL1 = CALBC1_16MHZ;				// 16 MHz Operation
	DCOCTL = CALDCO_16MHZ;
}

void initPins(void) {
	P1DIR = LCD_MOSI | LCD_SCK;
	P2DIR = LCD_RST | LCD_CS | LCD_DC;

	P1SEL |= LCD_MOSI + LCD_SCK;
	P1SEL2 |= LCD_MOSI + LCD_SCK;

	P2OUT = LCD_RST | LCD_CS | LCD_DC;
}

void fillScreen(int color) {
	unsigned int i;

	writeLCDControl(0x2A);				// Select Column Address
	writeLCDData(0);				// Setup beginning column address
	writeLCDData(0);				// Setup beginning column address
	writeLCDData(0);					// Setup ending column address
	writeLCDData(239);					// Setup ending column address

	writeLCDControl(0x2B);				// Select Row Address
	writeLCDData(0);					// Setup beginning row address
	writeLCDData(0);					// Setup beginning row address
	writeLCDData(0x01);					// Setup ending row address
	writeLCDData(0x3F);					// Setup ending row address

	writeLCDControl(0x2C);				// Select Memory Write
	for (i = 240 * 161; i > 0; i--)	// Loop through all memory locations 16 bit color
			{
		writeLCDData(color >> 8);		// Write data to LCD memory
		writeLCDData(color & 0xff);		// Write data to LCD memory
		writeLCDData(color >> 8);		// Write data to LCD memory
		writeLCDData(color & 0xff);		// Write data to LCD memory
	}
}

void drawPixel(int x, int y, int color) {
	writeLCDControl(0x2A);				// Select Column Address
	writeLCDData(x >> 8);	// Starting x Address (Most Sig 8 bits of address)
	writeLCDData(x & 0xff);	// Starting x Address (Least Sig 8 bits of address)
	writeLCDData(x >> 8);	// Ending x Address (Most Sig 8 bits of address)
	writeLCDData(x & 0xff);	// Ending x Address (Least Sig 8 bits of address)

	writeLCDControl(0x2B);				// Select Row Address
	writeLCDData(y >> 8);	// Starting y Address (Most Sig 8 bits of address)
	writeLCDData(y & 0xff);	// Starting y Address (Least Sig 8 bits of address)
	writeLCDData(y >> 8);	// Ending y Address (Most Sig 8 bits of address)
	writeLCDData(y & 0xff);	// Ending y Address (Least Sig 8 bits of address)

	writeLCDControl(0x2C);		// Select Color to write to the pixel
	writeLCDData(color >> 8);	// Send Most Significant 8 bits of 16 bit color
	writeLCDData(color & 0xff);	// Send Least Significant 8 bits of 16 bit color
}

void writeLCDControl(char data) {
	P2OUT &= ~LCD_DC;					// DC low
	P2OUT &= ~LCD_CS;					// CS Low

	while (IFG2 & UCB0TXIFG)
		;				// TX buffer ready?
	UCB0TXBUF = data;						// start transmission

	return;
}

void writeLCDData(char data) {
	P2OUT |= LCD_DC;					// DC high
	P2OUT &= ~LCD_CS;					// CS Low

	while (IFG2 & UCB0TXIFG)
		;				// TX buffer ready?
	UCB0TXBUF = data;						// start transmission

	return;
}

void waitMS(unsigned int m_sec) {
	while (m_sec--)
		__delay_cycles(16000);			// 1000 for 1MHz
}

void initLCD(void) {
	P1OUT = 0x00;			// Set all outputs low for initialization
	P2OUT = LCD_RST | LCD_CS | LCD_DC;

	P2OUT |= LCD_RST;					// Reset LCD
	waitMS(10);
	P2OUT &= ~LCD_RST;
	waitMS(10);
	P2OUT |= LCD_RST;
	waitMS(100);

	writeLCDControl(0xEF);				//
	writeLCDData(0x03);
	writeLCDData(0x80);
	writeLCDData(0x02);

	writeLCDControl(0xCB);				// Power Control A
	writeLCDData(0x39);
	writeLCDData(0x2C);
	writeLCDData(0x00);
	writeLCDData(0x34);
	writeLCDData(0x02);

	writeLCDControl(0xCF);				// Power Control B
	writeLCDData(0x00);
	writeLCDData(0xC1);
	writeLCDData(0x30);

	writeLCDControl(0xED);				// Power On Sequence Control
	writeLCDData(0x64);
	writeLCDData(0x03);
	writeLCDData(0x12);
	writeLCDData(0x81);

	writeLCDControl(0xE8);				// Driver Timing Control A
	writeLCDData(0x85);
	writeLCDData(0x00);
	writeLCDData(0x78);

	writeLCDControl(0xF7);				// Pump Ration Control
	writeLCDData(0x20);

	writeLCDControl(0xEA);				// Driver Timing Control A
	writeLCDData(0x00);
	writeLCDData(0x00);

	writeLCDControl(0xC0); 				// Power Control 1
	writeLCDData(0x23);

	writeLCDControl(0xC1);    			// Power Control 2
	writeLCDData(0x10);

	writeLCDControl(0xC5);    			// VCOM Control
	writeLCDData(0x3e);
	writeLCDData(0x28);

	writeLCDControl(0xC7);    			// VCOM Control2
	writeLCDData(0x86);

	writeLCDControl(0x36);    			// Memory Access Control
	writeLCDData(0x40 | 0x08);

	writeLCDControl(0x3A);				// COLMOD Pixel Format Set
	writeLCDData(0x55);

	writeLCDControl(0xB1);				// Frame Rate Control
	writeLCDData(0x00);
	writeLCDData(0x18);

	writeLCDControl(0xB6);    			// Display Function Control
	writeLCDData(0x08);
	writeLCDData(0x82);
	writeLCDData(0x27);

	writeLCDControl(0xF2);    			// 3Gamma Control
	writeLCDData(0x00);

	writeLCDControl(0x26);    			// Gamma curve selected
	writeLCDData(0x01);

	writeLCDControl(0xE0);   			// Positive Gamma Correction
	writeLCDData(0x0F);
	writeLCDData(0x31);
	writeLCDData(0x2B);
	writeLCDData(0x0C);
	writeLCDData(0x0E);
	writeLCDData(0x08);
	writeLCDData(0x4E);
	writeLCDData(0xF1);
	writeLCDData(0x37);
	writeLCDData(0x07);
	writeLCDData(0x10);
	writeLCDData(0x03);
	writeLCDData(0x0E);
	writeLCDData(0x09);
	writeLCDData(0x00);

	writeLCDControl(0xE1);    			// Negative Gamma Correction
	writeLCDData(0x00);
	writeLCDData(0x0E);
	writeLCDData(0x14);
	writeLCDData(0x03);
	writeLCDData(0x11);
	writeLCDData(0x07);
	writeLCDData(0x31);
	writeLCDData(0xC1);
	writeLCDData(0x48);
	writeLCDData(0x08);
	writeLCDData(0x0F);
	writeLCDData(0x0C);
	writeLCDData(0x31);
	writeLCDData(0x36);
	writeLCDData(0x0F);

	writeLCDControl(0x11);    			// Exit Sleep
	writeLCDControl(0x29);    			// Display on
}

void initUSCI(void) {
	UCB0CTL1 |= UCSWRST;					// USCI in reset state
	UCB0CTL0 |= UCMST + UCSYNC + UCCKPH + UCMSB;// SPI Master, 8bit, MSB first, synchronous mode
	UCB0CTL1 |= UCSSEL_2;					// USCI CLK-SRC=SMCLK=~8MHz
	UCB0CTL1 &= ~UCSWRST;				// USCI released for operation
	IE2 |= UCB0TXIE;						// enable TX interrupt
	IFG2 &= ~UCB0TXIFG;

	_EINT();							// enable interrupts
}

/**
 * DRAW LETTER
 * 	I think I am going to do it like this:  Each letter that
 * comes in, will have a name, location and color.  I will create
 * an 8x8 array, and for each letter, I will make it true or false
 * to create a "letter."
 */

/**
 * Outputs integers until integering is integered out.
 */
void drawInteger(int y, int x, int color, int number) {
	int temp = number;
	int numIntegers = 0;
	while (temp > 0) {
		temp = temp / 10;
		numIntegers++;
	}
	for (numIntegers; numIntegers > 0; numIntegers--) {
		drawNumber(y, x - (8 * numIntegers), color, number % 10);
		number = number / 10;
	}
}

/**
 * DRAW 16
 * 		This is a function that is small but took a while.  It takes in an array, and
 * 		based on the itemtype, it draws it a certain color.
 */
void draw16(int x, int y, const char charArray[32], char itemType) {

	unsigned int i = 0;
	unsigned int j = 0;
	int color = BLK;

	// This will choose the color to draw
	switch (itemType) {
	case 'e':
		color = RED;
		break;
	case 'l':
		color = BROWN;
		break;
	case 'p':
		color = BLU;
		break;
	default:
		break;
	}
	char theChar;

	// Drawing for loop
	for (i; i < 32; i++) {
		theChar = charArray[i];
		for (j; j < 8; j++) {
			if ((theChar & 0x80) == 0x80) {
				drawPixel((i / 2) + y, (i % 2) * 8 + j + x - 16, color);
			}
			theChar = theChar << 1;
		}
		j = 0;
	}
}

/**
 * CLEAR 16
 * 		Clears a 16x16 section
 */
void clear16(int x, int y) {
	int i = 0;
	int j = 0;
	for (i; i < 32; i++) {
		for (j; j < 8; j++) {
			drawPixel((i / 2) + y, (i % 2) * 8 + j + x - 16, WHITE);

		}
		j = 0;
	}
}

/**
 * This function has been created because I ran out of space in main() to create arrays.
 * 	This is my way of circumventing.  Unfortunately, this means I can't compare against it for my physics
 * 	but that's ok, ill figure it out.
 */
void createBricks(char brick[32]) {
	int brickArray = 0;
	for (brickArray; brickArray < 21; brickArray++) {
		draw16(brickArray * 16, 14 * 16, brick, 'b');
	}
	brickArray = 5;
	for (brickArray; brickArray < 17; brickArray++) {
		draw16(brickArray * 16, 11 * 16, brick, 'b');
	}
	brickArray = 2;
	for (brickArray; brickArray < 15; brickArray++) {
		if (brickArray != 9)
			draw16(brickArray * 16, 8 * 16, brick, 'b');
	}
	brickArray = 4;
	for (brickArray; brickArray < 17; brickArray++) {
		draw16(brickArray * 16, 4 * 16, brick, 'b');
	}
	draw16(17 * 16, 2 * 16, brick, 'b');
	draw16(18 * 16, 2 * 16, brick, 'b');
	draw16(19 * 16, 1 * 16, brick, 'b');
	draw16(20 * 16, 1 * 16, brick, 'b');
}

/**
 * BRICK CHECK
 * 		takes in X and Y.  If there is a brick in that spot, return 1.  Else, return 0.
 */
int brickCheck(int x, int y) {
	if ((x) == (14 * 16))
		return 1;
	if (((x) == (8 * 16)) && (y >= 2 * 16) && (y < 15 * 16)) {
		if (y != 9)
			return 1;
	}
	if (((x) == (11 * 16)) && (y >= 5 * 16) && (y < 17 * 16)) {
		return 1;
	}
	if (((x) == (4 * 16)) && (y >= 4 * 16) && (y < 17 * 16)) {
		return 1;
	}
	if (((x) == (2 * 16)) && ((y == 17) || (y == 18)))
		return 1;

	return 0;
}

/**
 * Here we will check if there is a ladder under a player or bomb.
 * 	If there is, return true.  This will also serve to let me decide
 * 	if I can go up or not.
 */
int ladderCheck(int y, int x) {
	int i = 0;
	for (i; i < 24; i++) {
		if ((ladders[i].yCoords == x) && (ladders[i].xCoords == y)) {
			return 1;
		}
		if ((ladders[i].yCoords == x) && (ladders[i].xCoords == y)) {
			return 1;
		}
	}
	return 0;
}

/**
 * BUTTON CHECK
 * 		If the button is pushed, the character jumps into the air for a second.
 */
void buttonCheck() {
	if (((P2IN & 0x10) != 0x10)
			&& (ladderCheck(playerY, playerX + 16)
					|| brickCheck(playerX + 16, playerY))) {
		clear16(playerY, playerX);
		if (ladderCheck(playerY, playerX)) {
			draw16(playerY, playerX, ladder, 'l');
		}
		playerX += -16;
		draw16(playerY, playerX, player, 'a');
		jumpSpeed = 1;
		jumpCount = 1;
	}
	if ((P2IN & 0x20) != 0x20) {

	}
}

/**
 * BAD GUY HEAD CHECK
 * 		This controls the spawning of bad guy heads.  If the time has elapsed,
 * 		then create a head and it starts walking by itself.
 */
void badGuyHeadCheck() {
	if (spawnCounter == 0) {
		spawnCounter = 1;
		unsigned int i = 0;
		for (i; i < 8; i++) {
			if (badHead[i].xCoords == 0) {
				badHead[i].xCoords = 1 * 16;
				badHead[i].yCoords = 15 * 16;
				draw16(16, 15 * 16, badGuyHead, 'e');
				break;
			}
		}
	}
}

/**
 * If the player arrives to the top, you win the level.
 */
void victoryConditionCheck() {
	if (playerX == 1 * 16) {
		int i = 0;
		// clear bad guys from the screen
		for (i; i < 8; i++) {
			clear16(badHead[i].yCoords, badHead[i].xCoords);
			if (ladderCheck(badHead[i].yCoords, badHead[i].xCoords)) {
				draw16(badHead[i].yCoords, badHead[i].xCoords, ladder, 'l');
			}
			badHead[i].xCoords = 0;
			badHead[i].yCoords = 0;
		}
		clear16(playerY, playerX);
		if (ladderCheck(playerY, playerX)) {
			draw16(playerY, playerX, ladder, 'l');
		}
		// restart player location and draw him back in
		playerY = 20 * 16;
		playerX = 13 * 16;
		draw16(playerY, playerX, player, 'a');
		spawnSpeed = spawnSpeed / 2;  // double the spawn speed by halving the clock
		spawnCounter = 1;
		score += 500;  // increment score

		// check to see if you win the game.
		if (spawnSpeed < 10) {
			victorySequence();
		}
	}
}

/**
 * IF you win, clear the screen until the player hits the button
 */
void victorySequence() {

	fillScreen(WHITE);

	while (adcZ < 400) {

		drawString(120, 240, BLK, "YOU WIN!!!");
		drawString(132, 240, GRN, "Play Again?");
		channelCheck();
	}
	fillScreen(WHITE);
	openingLoad();

}

/**
 * Hmm.  I don't think I actually plugged this in.  I must have been tired.
 *
 * Pauses game if you touch the screen.
 */
void pauseGame() {
	if (adcZ > 400) {
		adcZ = 0;
		while (adcZ < 400)
			channelCheck();
	}
}

/**
 * COLLISON DETECTION
 * 		Checks the enemy location against the player location.  If the player is in the same spot, kill
 * 		the player.
 */
void collisionDetection() {
	int i = 0;
	// run the enemies against the player.  If it matches, then clear the screen.
	for (i; i < 8; i++) {
		if ((badHead[i].yCoords == playerY)
				&& (badHead[i].xCoords == playerX)) {
			int j = 0;
			for (j; j < 8; j++) {
				clear16(badHead[j].yCoords, badHead[j].xCoords);
				if (ladderCheck(badHead[i].yCoords, badHead[i].xCoords)) {
					draw16(badHead[i].yCoords, badHead[i].xCoords, ladder, 'l');
				}
				badHead[j].xCoords = 0;
				badHead[j].yCoords = 0;
			}
			clear16(playerY, playerX);
			playerY = 20 * 16;
			playerX = 13 * 16;
			draw16(playerY, playerX, player, 'a');
			lives--;
			drawInteger(8, 32, BLK, lives);
			if (lives < 0) {
				gameOver();  // game over sequence if out of lives.
			}
		}
	}
}

/**
 * GAME OVER
 * 		If the player dies, the game over sequence appears.  If you click the
 * 		screen, then the game starts over.
 */
void gameOver() {
	fillScreen(WHITE);

	while (adcZ < 400) {

		drawString(120, 240, BLK, "GAME OVER!                   ");
		drawString(132, 240, GRN, "Play Again?                  ");
		channelCheck();
	}
	fillScreen(WHITE);
	openingLoad();

}

/**
 * 	OPENING LOAD
 * 		Starts the game up.  Moved from MAIN so I could run it multiple times
 *
 * 		Deletes everything, resets the lives and score, and redraws everything.  Win win.
 */
void openingLoad() {
	lives = 5;
	score = 0;
	/**
	 * 		This portion is created with the notion that our screen can be itemized as a
	 *	15*15 pixels per block.  This will allow me to control the location
	 *	better so there are no weird floaty things.
	 */
	createBricks(brick);

	int itemsList = 24;
	for (itemsList; itemsList >= 0; itemsList--) {
		if (ladders[itemsList].xCoords != 0
				&& ladders[itemsList].yCoords != 0) {
			draw16(ladders[itemsList].xCoords, ladders[itemsList].yCoords,
					ladder, 'l');
		}
	}

	drawString(8, 190, BLK, "Score                  ");
	drawString(8, 72, BLK, "Lives                   ");
	drawInteger(8, 32, BLK, lives);
	draw16(penguinLoc.xCoords, penguinLoc.yCoords, penguin, 'p');
	draw16(badGuyLoc.xCoords, badGuyLoc.yCoords, badGuy, 'e');
	draw16(playerY, playerX, player, 'a');

}
#pragma INTERRUPT (USCI)
#pragma vector=USCIAB0TX_VECTOR
void USCI(void) {
	P2OUT |= LCD_CS;						// transmission done
	IFG2 &= ~UCB0TXIFG;					// clear TXIFG
}

/**
 * For this, we have a few countdown timers we compare against:  jumping, and joystick speed
 * and spawn speed.
 */
#pragma vector=TIMER0_A0_VECTOR
__interrupt
void TIMER_A(void) {
	count++;
	joystickSpeed++;
	jumpCount++;
	spawnCounter++;
	enemySpeed++;
	// 1 second
	if (count == 15) {
		count = 0;
		score += 1;
	}
	if (joystickSpeed > 5) {
		joystickSpeed = 0;
		enemySpeed = 0;
	}
	if (jumpCount > 30) {
		jumpSpeed = 0;
	}

	if (spawnCounter > spawnSpeed) {
		spawnCounter = 0;
	}
}

