/*
* Copyright (c) 2019 Nordic Semiconductor ASA
*
* SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
*/

#include <nrf9160.h>
#include <zephyr.h>
#include <device.h>
#include <stdio.h>
#include <sys/printk.h>
#include <drivers/spi.h>
#include <drivers/gpio.h>
#include <dk_buttons_and_leds.h>
#include "lcd.h"
#include "font.h"
#include "img.h"
#include "inner_flash.h"
#include "external_flash.h"
#include "uart_ble.h"

bool lcd_sleep_in = false;
bool lcd_sleep_out = false;


void test_show_image(void)
{
	u8_t i=0;
	u16_t x,y,w=0,h=0;

	printk("test_show_image\n");
	
	LCD_Clear(BLACK);
	
	//LCD_get_pic_size(peppa_pig_160X160, &w, &h);
	LCD_get_pic_size_from_flash(IMG_PEPPA_240X240_ADDR, &w, &h);
	LCD_dis_pic_from_flash(0, 0, IMG_PEPPA_240X240_ADDR);
	while(0)
	{
		switch(i)
		{
			case 0:
				LCD_dis_pic_from_flash(w*0, h*0, IMG_PEPPA_240X240_ADDR);
				//LCD_dis_pic_rotate(w*0,h*0,peppa_pig_160X160,0);
				break;
			case 1:
				LCD_dis_pic_from_flash(w*1, h*0, IMG_PEPPA_240X240_ADDR);
				//LCD_dis_pic_rotate(w*1,h*0,peppa_pig_160X160,90);
				break;
			case 2:
				LCD_dis_pic_from_flash(w*1, h*1, IMG_PEPPA_240X240_ADDR);
				//LCD_dis_pic_rotate(w*1,h*1,peppa_pig_160X160,180);
				break;
			case 3:
				LCD_dis_pic_from_flash(w*0, h*1, IMG_PEPPA_240X240_ADDR);
				//LCD_dis_pic_rotate(w*0,h*1,peppa_pig_160X160,270);
				break;
			case 4:
				LCD_Fill(w*0,h*0,w,h,BLACK);
				break;
			case 5:
				LCD_Fill(w*1,h*0,w,h,BLACK);
				break;
			case 6:
				LCD_Fill(w*1,h*1,w,h,BLACK);
				break;
			case 7:
				LCD_Fill(w*0,h*1,w,h,BLACK);
				break;
		}
		
		i++;
		if(i>=8)
			i=0;
		
		k_sleep(K_MSEC(1000));								//�����ʱ1000ms
	}
}

void test_show_color(void)
{
	u8_t i=0;

	printk("test_show_image\n");
	
	while(1)
	{
		switch(i)
		{
			case 0:
				LCD_Clear(WHITE);
				break;
			case 1:
				LCD_Clear(BLACK);
				break;
			case 2:
				LCD_Clear(BLUE);
				break;
			case 3:
				LCD_Clear(BRED);
				break;
			case 4:
				LCD_Clear(GRED);
				break;
			case 5:
				LCD_Clear(GBLUE);
				break;
			case 6:
				LCD_Clear(RED);
				break;
			case 7:
				LCD_Clear(MAGENTA);
				break;
			case 8:
				LCD_Clear(GREEN);
				break;
			case 9:
				LCD_Clear(CYAN);
				break;
			case 10:
				LCD_Clear(YELLOW);
				break;
			case 11:
				LCD_Clear(BROWN);
				break;
			case 12:
				LCD_Clear(BRRED);
				break;
			case 13:
				LCD_Clear(GRAY);
				break;					
		}
		
		i++;
		if(i>=14)
			i=0;
		
		k_sleep(K_MSEC(1000));								//�����ʱ1000ms
	}
}

void test_show_string(void)
{
	u16_t x,y,w,h;
	
	//LCD_Clear(BLACK);								//����
	
	POINT_COLOR=WHITE;								//������ɫ
	BACK_COLOR=BLACK;  								//����ɫ 

#ifdef FONT_16
	LCD_SetFontSize(FONT_SIZE_16);					//���������С
#endif
	LCD_MeasureString("�����а¿�˹�������޹�˾",&w,&h);
	x = (w > LCD_WIDTH)? 0 : (LCD_WIDTH-w)/2;
	y = 40;	
	LCD_ShowString(x,y,"�����а¿�˹�������޹�˾");
	
	LCD_MeasureString("2015-2020 August International Ltd. All Rights Reserved.",&w,&h);
	x = (w > LCD_WIDTH)? 0 : (LCD_WIDTH-w)/2;
	y = y + h + 10;	
	LCD_ShowString(x,y,"2015-2020 August International Ltd. All Rights Reserved.");

#ifdef FONT_24
	LCD_SetFontSize(FONT_SIZE_24);					//���������С
#endif
	LCD_MeasureString("Rawmec Business Park, Plumpton Road, Hoddesdon",&w,&h);
	x = (w > LCD_WIDTH)? 0 : (LCD_WIDTH-w)/2;
	y = y + h + 10;	
	LCD_ShowString(x,y,"Rawmec Business Park, Plumpton Road, Hoddesdon");
	
	LCD_MeasureString("��������������������·�����㳡A��",&w,&h);
	x = (w > LCD_WIDTH)? 0 : (LCD_WIDTH-w)/2;
	y = y + h + 10;	
	LCD_ShowString(x,y,"��������������������·�����㳡A��");

#ifdef FONT_32
	LCD_SetFontSize(FONT_SIZE_32);					//���������С
#endif
	LCD_MeasureString("2020-01-03 16:30:45",&w,&h);
	x = (w > LCD_WIDTH)? 0 : (LCD_WIDTH-w)/2;
	y = y + h + 10;	
	LCD_ShowString(x,y,"2020-01-03 16:30:45");
}

/**@brief Initializes buttons and LEDs, using the DK buttons and LEDs
 * library.
 */
static void buttons_leds_init(void)
{
	int err;

	err = dk_leds_init();
	if (err)
	{
		printk("Could not initialize leds, err code: %d\n", err);
	}

	err = dk_set_leds_state(0x00, DK_ALL_LEDS_MSK);
	if (err)
	{
		printk("Could not set leds state, err code: %d\n", err);
	}
}

void system_init(void)
{
	buttons_leds_init();
	key_init();
	LCD_Init();
	flash_init();
}

/***************************************************************************
* ��  �� : main���� 
* ��  �� : �� 
* ����ֵ : int ����
**************************************************************************/
int main(void)
{
	printk("main\n");

	system_init();
	dk_set_led(DK_LED1,1);

//	test_show_string();
//	test_show_image();
	test_nvs();
//	test_flash();
//	test_uart_ble();

	while(true)
	{
		k_cpu_idle();
	}
}
