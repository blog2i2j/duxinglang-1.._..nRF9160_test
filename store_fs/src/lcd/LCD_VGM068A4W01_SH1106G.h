/****************************************Copyright (c)************************************************
** File Name:			    LCD_VGM068A4W01_SH1106G.h
** Descriptions:			The VGM068A4W01_SH1106G screen drive head file
** Created By:				xie biao
** Created Date:			2021-01-07
** Modified Date:      		2021-01-07
** Version:			    	V1.0
******************************************************************************************************/
#include <zephyr\types.h>
#include "lcd.h"

#ifdef LCD_VGM068A4W01_SH1106G

//#include "boards.h"

//�ӿڶ���
//extern xdata unsigned char buffer[512];
//------------------------------------------------------
#define COL 		96			//��
#define ROW 		32			//��
#define PAGE_H		8			//ÿ��page���ص��Ϊ8
#define PAGE_MAX	4			//�ܹ�4��page

//LCD�Ļ�����ɫ�ͱ���ɫ	   
extern uint16_t  POINT_COLOR;//Ĭ�Ϻ�ɫ    
extern uint16_t  BACK_COLOR; //������ɫ.Ĭ��Ϊ��ɫ

//LCM
#define LCD_PORT	"GPIO_0"
#define LCD_DEV 	"SPI_3"

#define CS		23
#define	RST		24
#define	RS		21
#define	SCL		22
#define	SDA		20
#define VDD		18

//TP 
#define TP_0			12
#define TP_1			12

//CTP
#define CTP_EINT	25
#define	CTP_SDA		0
#define	CTP_SCL		1
#define	CTP_RSET	16

//LEDK(LED����)
#define LEDK	31
#define LEDA	14

#define X_min 0x0043		 //TP���Է�Χ��������
#define X_max 0x07AE
#define Y_min 0x00A1
#define Y_max 0x0759

#define LCD_DATA_LEN ((COL*ROW)/8)

//------------------------------------------------------

extern u8_t lcd_data_buffer[2*LCD_DATA_LEN];

extern void BlockWrite(unsigned int x,unsigned int y,unsigned int w,unsigned int h);
extern void WriteOneDot(u16_t color);
extern void Write_Data(u8_t i);
extern void LCD_Clear(u16_t color);
extern void LCD_Init(void);
extern void LCD_SleepIn(void);
extern void LCD_SleepOut(void);

#endif/*LCD_R108101_GC9307*/
