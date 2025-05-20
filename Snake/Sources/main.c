/*
 * main.c
 *
 *  Created on: 7. 11. 2024
 *      Author: Jan Klanica (xklani00)
 */

#include "MK60D10.h"
#include "snake.h"

/* Mapping of LEDs and buttons to specific port pins: */
#define BTN_RIGHT_MASK 	0x400    	/* Port E, bit 10, SW2 */
#define BTN_DOWN_MASK 	0x1000    	/* Port E, bit 12, SW3 */
#define BTN_LEFT_MASK 	0x8000000 	/* Port E, bit 27, SW4 */
#define BTN_UP_MASK 	0x4000000   /* Port E, bit 26, SW5 */

/* Mapping of display to specific port pins: */
#define DISP_A0_MASK 	0x100		/* Port A, bit 8 */
#define DISP_A0_SHIFT 	8
#define DISP_A0(x)		(((uint32_t)(((uint32_t)(x))<<DISP_A0_SHIFT))&DISP_A0_MASK)
#define DISP_A1_MASK 	0x400		/* Port A, bit 10 */
#define DISP_A1_SHIFT 	10
#define DISP_A1(x)		(((uint32_t)(((uint32_t)(x))<<DISP_A1_SHIFT))&DISP_A1_MASK)
#define DISP_A2_MASK 	0x40		/* Port A, bit 6 */
#define DISP_A2_SHIFT 	6
#define DISP_A2(x)		(((uint32_t)(((uint32_t)(x))<<DISP_A2_SHIFT))&DISP_A2_MASK)
#define DISP_A3_MASK 	0x800		/* Port A, bit 11 */
#define DISP_A3_SHIFT 	11
#define DISP_A3(x)		(((uint32_t)(((uint32_t)(x))<<DISP_A3_SHIFT))&DISP_A3_MASK)
#define DISP_R0_MASK 	0x4000000	/* Port A, bit 26 */
#define DISP_R0_SHIFT 	26
#define DISP_R0(x)		(((uint32_t)(((uint32_t)(x))<<DISP_R0_SHIFT))&DISP_R0_MASK)
#define DISP_R1_MASK 	0x1000000	/* Port A, bit 24 */
#define DISP_R1_SHIFT 	24
#define DISP_R1(x)		(((uint32_t)(((uint32_t)(x))<<DISP_R1_SHIFT))&DISP_R1_MASK)
#define DISP_R2_MASK 	0x200		/* Port A, bit 9 */
#define DISP_R2_SHIFT 	9
#define DISP_R2(x)		(((uint32_t)(((uint32_t)(x))<<DISP_R2_SHIFT))&DISP_R2_MASK)
#define DISP_R3_MASK 	0x2000000	/* Port A, bit 25 */
#define DISP_R3_SHIFT 	25
#define DISP_R3(x)		(((uint32_t)(((uint32_t)(x))<<DISP_R3_SHIFT))&DISP_R3_MASK)
#define DISP_R4_MASK 	0x10000000	/* Port A, bit 28 */
#define DISP_R4_SHIFT 	28
#define DISP_R4(x)		(((uint32_t)(((uint32_t)(x))<<DISP_R4_SHIFT))&DISP_R4_MASK)
#define DISP_R5_MASK 	0x80		/* Port A, bit 7 */
#define DISP_R5_SHIFT 	7
#define DISP_R5(x)		(((uint32_t)(((uint32_t)(x))<<DISP_R5_SHIFT))&DISP_R5_MASK)
#define DISP_R6_MASK 	0x8000000	/* Port A, bit 27 */
#define DISP_R6_SHIFT 	27
#define DISP_R6(x)		(((uint32_t)(((uint32_t)(x))<<DISP_R6_SHIFT))&DISP_R6_MASK)
#define DISP_R7_MASK 	0x20000000	/* Port A, bit 29 */
#define DISP_R7_SHIFT 	29
#define DISP_R7(x)		(((uint32_t)(((uint32_t)(x))<<DISP_R7_SHIFT))&DISP_R7_MASK)
#define DISP_EN_MASK 	0x10000000 	/* Port E, bit 28 */
#define DISP_EN_SHIFT 	28
#define DISP_EN(x)		(((uint32_t)(((uint32_t)(x))<<DISP_EN_SHIFT))&DISP_EN_MASK)

/* Timer periods */
#define DISP_COL_CLK_PERIOD 0xCB72	/* 	   52 083 cycles */
#define SNAKE_STEP_PERIOD 0x98967F	/* 10 000 000 cycles */

SnakeGame sgame;
enum Direction next_direction = NONE;
int column_i = 0;

/* Initialize the MCU - basic clock settings, turning the watchdog off */
void MCUInit(void)  {
    MCG_C4 |= ( MCG_C4_DMX32_MASK | MCG_C4_DRST_DRS(0x01) );
    SIM_CLKDIV1 |= SIM_CLKDIV1_OUTDIV1(0x00);
    WDOG_STCTRLH &= ~WDOG_STCTRLH_WDOGEN_MASK;
}

/* Initialize buttons - port clock, GPIO pins, interrupt */
void ButtonsInit(void)
{
    SIM->SCGC5 = SIM_SCGC5_PORTE_MASK;  /* Turn on port clock */

    /* Set pins for GPIO functionality and set interrupt */
    PORTE->PCR[10] = PORT_PCR_MUX(0x01) | PORT_PCR_ISF(0x01) | PORT_PCR_IRQC(0xA); // SW2
    PORTE->PCR[12] = PORT_PCR_MUX(0x01) | PORT_PCR_ISF(0x01) | PORT_PCR_IRQC(0xA); // SW3
    PORTE->PCR[27] = PORT_PCR_MUX(0x01) | PORT_PCR_ISF(0x01) | PORT_PCR_IRQC(0xA); // SW4
    PORTE->PCR[26] = PORT_PCR_MUX(0x01) | PORT_PCR_ISF(0x01) | PORT_PCR_IRQC(0xA); // SW5

    NVIC_ClearPendingIRQ(PORTE_IRQn); /* Clear interrupt flag for port E */
    NVIC_EnableIRQ(PORTE_IRQn);       /* Enable interrupt from port E */
}

/* Initialize display - port clock, GPIO pins */
void DisplayInit(void) {
	SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK;  /* Turn on port A clock */
	SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;  /* Turn on port E clock */

	/* Set pins for GPIO functionality */
	PORTA_PCR(DISP_A0_SHIFT) = PORT_PCR_MUX(0x01);
	PORTA_PCR(DISP_A1_SHIFT) = PORT_PCR_MUX(0x01);
	PORTA_PCR(DISP_A2_SHIFT) = PORT_PCR_MUX(0x01);
	PORTA_PCR(DISP_A3_SHIFT) = PORT_PCR_MUX(0x01);
	PORTA_PCR(DISP_R0_SHIFT) = PORT_PCR_MUX(0x01);
	PORTA_PCR(DISP_R1_SHIFT) = PORT_PCR_MUX(0x01);
	PORTA_PCR(DISP_R2_SHIFT) = PORT_PCR_MUX(0x01);
	PORTA_PCR(DISP_R3_SHIFT) = PORT_PCR_MUX(0x01);
	PORTA_PCR(DISP_R4_SHIFT) = PORT_PCR_MUX(0x01);
	PORTA_PCR(DISP_R5_SHIFT) = PORT_PCR_MUX(0x01);
	PORTA_PCR(DISP_R6_SHIFT) = PORT_PCR_MUX(0x01);
	PORTA_PCR(DISP_R7_SHIFT) = PORT_PCR_MUX(0x01);
	PORTE_PCR(DISP_EN_SHIFT) = PORT_PCR_MUX(0x01);

	/* Set GPIO pins as output */
	GPIOA_PDDR |= (DISP_A0(0x1)
			| DISP_A1(0x1)
			| DISP_A2(0x1)
			| DISP_A3(0x1)
			| DISP_R0(0x1)
			| DISP_R1(0x1)
			| DISP_R2(0x1)
			| DISP_R3(0x1)
			| DISP_R4(0x1)
			| DISP_R5(0x1)
			| DISP_R6(0x1)
			| DISP_R7(0x1));
	GPIOE_PDDR |= DISP_EN(0x1);
}

/* Initialize PIT - module clock, interrupt period */
void PITInit() {
	SIM_SCGC6 |= SIM_SCGC6_PIT_MASK; // Enable clock to PIT

	// turn on PIT
	PIT_MCR = 0;

	// Timer 0
	PIT_TCTRL0 &= ~PIT_TCTRL_TEN_MASK;	// Turn OFF PIT to perform setup
	PIT_LDVAL0 = DISP_COL_CLK_PERIOD;	// Setup timer period
	PIT_TCTRL0 = PIT_TCTRL_TIE_MASK; 	// Enable timer interrupts
	PIT_TFLG0 = PIT_TFLG0; 				// Clear any pending interrupt (now)
	NVIC_EnableIRQ(PIT0_IRQn);			// Enable interrupt
	PIT_TCTRL0 |= PIT_TCTRL_TEN_MASK; 	// Start timer

	// Timer 1
	PIT_TCTRL1 &= ~PIT_TCTRL_TEN_MASK;  // Turn OFF PIT to perform setup
	PIT_LDVAL1 = SNAKE_STEP_PERIOD; 	// Setup timer period
	PIT_TCTRL1 = PIT_TCTRL_TIE_MASK; 	// Enable timer interrupts
	PIT_TFLG1 = PIT_TFLG1;				// Clear any pending interrupt (now)
	NVIC_EnableIRQ(PIT1_IRQn);			// Enable interrupt
	PIT_TCTRL1 |= PIT_TCTRL_TEN_MASK; 	// Start timer
}

/* A delay function */
void delay(uint64_t bound) {
	for (uint64_t i=0; i < bound; i++) { __NOP(); }
}

/* Send next column data to display */
void PIT0_IRQHandler() {
	GPIOE_PDOR |= DISP_EN(0x1);			// Disable display

	// get column display data
	int col[8];
	snake_game_get_col_as_nulls_and_zeros(&sgame, column_i, col);

 	// clean all screen data
	GPIOA_PDOR &= ~(DISP_A0_MASK | DISP_A1_MASK | DISP_A2_MASK | DISP_A3_MASK | DISP_R0_MASK
				   | DISP_R1_MASK | DISP_R2_MASK | DISP_R3_MASK | DISP_R4_MASK | DISP_R5_MASK
				   | DISP_R6_MASK | DISP_R7_MASK);

	// set screen data
	GPIOA_PDOR |= (DISP_A0((column_i & 0x1)) | DISP_A1((column_i & 0x2) >> 1)
				  | DISP_A2((column_i & 0x4) >> 2) | DISP_A3((column_i & 0x8) >> 3)
				  | DISP_R0(col[0]) | DISP_R1(col[1]) | DISP_R2(col[2]) | DISP_R3(col[3])
				  | DISP_R4(col[4]) | DISP_R5(col[5]) | DISP_R6(col[6]) | DISP_R7(col[7]));

	GPIOE_PDOR &= ~DISP_EN(0x1);		// Enable display
	column_i = (column_i + 1) % 16;  	// Move to next column
	PIT_TFLG0 = PIT_TFLG0;				// Clear interrupt
}

/* Make next move in a snake game */
void PIT1_IRQHandler() {
	snake_game_move(&sgame, next_direction);
	next_direction = NONE;
	PIT_TFLG1 = PIT_TFLG1;
}

/* Register buttons as inputs to snake game */
void PORTE_IRQHandler(void)
{
	delay(20000);

	// next direction is rotated because the display is attached in rotated state
	if ((PORTE_ISFR & BTN_RIGHT_MASK) && !(GPIOE->PDIR & BTN_RIGHT_MASK)) {
		next_direction = UP;
	}
	if ((PORTE_ISFR & BTN_LEFT_MASK) && !(GPIOE->PDIR & BTN_LEFT_MASK)) {
		next_direction = DOWN;
	}
	if ((PORTE_ISFR & BTN_UP_MASK) && !(GPIOE->PDIR & BTN_UP_MASK)) {
		next_direction = LEFT;
	}
	if ((PORTE_ISFR & BTN_DOWN_MASK) && !(GPIOE->PDIR & BTN_DOWN_MASK)) {
		next_direction = RIGHT;
	}
	PORTE_ISFR = PORTE_ISFR;
}

int main(void)
{
    MCUInit();
    snake_game_init(&sgame);
    ButtonsInit();
    DisplayInit();
    PITInit();

    while (1) {}

    return 0;
}
