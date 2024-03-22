/*
 * LCD_GFX.c
 *
 * Created: 9/20/2021 6:54:25 PM
 *  Author: You
 */ 

#include "LCD_GFX.h"
#include "ST7735.h"

/******************************************************************************
* Local Functions
******************************************************************************/



/******************************************************************************
* Global Functions
******************************************************************************/

/**************************************************************************//**
* @fn			uint16_t rgb565(uint8_t red, uint8_t green, uint8_t blue)
* @brief		Convert RGB888 value to RGB565 16-bit color data
* @note
*****************************************************************************/
uint16_t rgb565(uint8_t red, uint8_t green, uint8_t blue)
{
	return ((((31*(red+4))/255)<<11) | (((63*(green+2))/255)<<5) | ((31*(blue+4))/255));
}

/**************************************************************************//**
* @fn			void LCD_drawPixel(uint8_t x, uint8_t y, uint16_t color)
* @brief		Draw a single pixel of 16-bit rgb565 color to the x & y coordinate
* @note
*****************************************************************************/
void LCD_drawPixel(uint8_t x, uint8_t y, uint16_t color) {
	LCD_setAddr(x,y,x,y);
	SPI_ControllerTx_16bit(color);
}

/**************************************************************************//**
* @fn			void LCD_drawChar(uint8_t x, uint8_t y, uint16_t character, uint16_t fColor, uint16_t bColor)
* @brief		Draw a character starting at the point with foreground and background colors
* @note
*****************************************************************************/
void LCD_drawChar(uint8_t x, uint8_t y, uint16_t character, uint16_t fColor, uint16_t bColor){
	uint16_t row = character - 0x20;		//Determine row of ASCII table starting at space
	int i, j;
	if ((LCD_WIDTH-x>=5)&&(LCD_HEIGHT-y>=7)){
		for(i=0;i<5;i++){
			uint8_t pixels = ASCII[row][i]; //Go through the list of pixels
			for(j=0;j<8;j++){
				if (((pixels>>j)&1)==1){
					LCD_drawPixel(x+i,y+j,fColor);
				}
				else {
					LCD_drawPixel(x+i,y+j,bColor);
				}
			}
		}
	}
}


/******************************************************************************
* LAB 4 TO DO. COMPLETE THE FUNCTIONS BELOW.
* You are free to create and add any additional files, libraries, and/or
*  helper function. All code must be authentically yours.
******************************************************************************/

void swap(uint8_t* a, uint8_t* b) {
	int temp = *a;
	*a = *b;
	*b = temp;
}

void swap_s(int16_t* a, int16_t* b) {
	int temp = *a;
	*a = *b;
	*b = temp;
}

/**************************************************************************//**
* @fn			void LCD_drawCircle(uint8_t x0, uint8_t y0, uint8_t radius,uint16_t color)
* @brief		Draw a colored circle of set radius at coordinates
* @note
*********************************************************/

void LCD_drawCircle(uint8_t x0, uint8_t y0, uint8_t radius,uint16_t color)
{
	int radSq = radius * radius;
	int edgeLength = (radius + radius + 1);
	int rectSize = edgeLength * edgeLength;
	int x, y;
	int lasty;
	for(int x = 0; x <= radius; x++) {
		for (int y = 0; y <= radius; y ++) {
			if(x*x+y*y <= radSq && y > lasty) {
				lasty = y;
			}
		}
		LCD_drawBlock(x0+x, y0-lasty, x0+x+1, y0+lasty+1, color);
		LCD_drawBlock(x0-x, y0-lasty, x0-x+1, y0+lasty+1, color);
		lasty = -1;
	}
}

/**************************************************************************//**
* @fn			void LCD_drawLine(short x0,short y0,short x1,short y1,uint16_t c)
* @brief		Draw a line from and to a point with a color
* @note
*****************************************************************************/
void LCD_drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color)
{
	if (x0 > x1) {
		swap(&x0, &x1);
	}
	if (y0 > y1) {
		swap(&y0, &y1);
	}
	int dx = x1 - x0;
	int dy = y1 - y0;
	int D = 2 * dy - dx;
	int y = y0;
	for (int x = x0; x < x1; x++) {
		LCD_drawPixel(x, y, color);
		if (D > 0) {
			y++;
			D -= 2 * dx;
		}
		D += 2 * dy;
	}
}

/**************************************************************************//**
* @fn			void LCD_drawBlock(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1,uint16_t color)
* @brief		Draw a colored block at coordinates
* @note
*****************************************************************************/
void LCD_drawBlock(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
	// now circles and blocks can be partially offscreen
	x0 = CLAMP(x0, LCD_WIDTH, 0);
	x1 = CLAMP(x1, LCD_WIDTH, 0);
	y0 = CLAMP(y0, LCD_HEIGHT, 0);
	y1 = CLAMP(y1, LCD_HEIGHT, 0);
	if (x0 == x1 || y0 == y1) {
		return;
	}
	if (x0 > x1) {
		swap_s(&x0, &x1);
	}
	if (y0 > y1) {
		swap_s(&y0, &y1);
	}
	LCD_setAddr(x0,y0,x1-1,y1-1);
	int numPixels = (x1-x0)*(y1-y0);
	clear(LCD_PORT, LCD_TFT_CS);
	for (int i = 0; i < numPixels; i++) {
		SPI_ControllerTx_16bit_stream(color);
	}
	set(LCD_PORT, LCD_TFT_CS);
}

/**************************************************************************//**
* @fn			void LCD_setScreen(uint16_t color)
* @brief		Draw the entire screen to a color
* @note
*****************************************************************************/
void LCD_setScreen(uint16_t color) 
{
	LCD_setAddr(0,0,LCD_WIDTH-1,LCD_HEIGHT-1);
	int numPixels = LCD_SIZE;
	clear(LCD_PORT, LCD_TFT_CS);
	for (int i = 0; i < numPixels; i++) {
		SPI_ControllerTx_16bit_stream(color);
	}
	set(LCD_PORT, LCD_TFT_CS);
}

/**************************************************************************//**
* @fn			void LCD_drawString(uint8_t x, uint8_t y, char* str, uint16_t fg, uint16_t bg)
* @brief		Draw a string starting at the point with foreground and background colors
* @note
*****************************************************************************/
void LCD_drawString(uint8_t x, uint8_t y, char* str, uint16_t fg, uint16_t bg)
{
	for (int i = 0; str[i] != 0; i++) {
		LCD_drawChar(x + i * 6, y, str[i], fg, bg);
		LCD_drawBlock(x + i * 6 + 5, y, x + i * 6 + 6, y + 8, bg);
	}
}