#include "lcd.h"

#ifdef LCD_R154101_ST7796S


//�ӿڶ���
//extern xdata unsigned char buffer[512];
//------------------------------------------------------
#define COL 320			//��
#define ROW 320			//��


//LCD�Ļ�����ɫ�ͱ���ɫ	   
extern uint16_t  POINT_COLOR;//Ĭ�Ϻ�ɫ    
extern uint16_t  BACK_COLOR; //������ɫ.Ĭ��Ϊ��ɫ

//LCM
#define LCD_PORT	"GPIO_0"

#define	CS 				11	
#define	RST				28
#define	RS				12
#define	WR				13
#define	RD				20

//DB0~7
#define DB0				26
#define DB1				27
#define DB2				25
#define DB3				31
#define DB4				24
#define DB5				23
#define DB6				30
#define DB7				22

#define DBS_LIST { DB0, DB1, DB2, DB3, DB4, DB5, DB6, DB7 }

//TP 
#define TP_CS			10

//LEDK(LED����)
#define LEDK_1          0
#define LEDK_2          1

#define X_min 0x0043		 //TP���Է�Χ��������
#define X_max 0x07AE
#define Y_min 0x00A1
#define Y_max 0x0759
//------------------------------------------------------

extern void BlockWrite(unsigned int x,unsigned int y,unsigned int w,unsigned int h);
extern void WriteOneDot(unsigned int color);
extern void LCD_Init(void);
extern void Write_Data(uint8_t i);
extern void LCD_Clear(uint16_t color);

#endif/*LCD_R154101_ST7796S*/
