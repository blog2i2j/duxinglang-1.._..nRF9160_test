/*
* Copyright (c) 2019 Nordic Semiconductor ASA
*
* SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
*/

#include <zephyr.h>
#include <stdio.h>
#include <zephyr/types.h>
#include <string.h>
#include <drivers/uart.h>
#include <dk_buttons_and_leds.h>
#include "datetime.h"
#include "Settings.h"

#define UART_DEV	"UART_1"
#define BUF_MAXSIZE	1024

#define PACKET_HEAD	0xAB
#define PACKET_END	0x88

#define HEART_RATE_ID			0xFF31			//����
#define BLOOD_OXYGEN_ID			0xFF32			//Ѫ��
#define BLOOD_PRESSURE_ID		0xFF33			//Ѫѹ
#define	ONE_KEY_MEASURE_ID		0xFF34			//һ������
#define	PULL_REFRESH_ID			0xFF35			//����ˢ��
#define	SLEEP_DETAILS_ID		0xFF36			//˯������
#define	FIND_DEVICE_ID			0xFF37			//�����ֻ�
#define SMART_NOTIFY_ID			0xFF38			//��������
#define	ALARM_SETTING_ID		0xFF39			//��������
#define USER_INFOR_ID			0xFF40			//�û���Ϣ
#define	SEDENTARY_ID			0xFF41			//��������
#define	SHAKE_SCREEN_ID			0xFF42			//̧������
#define	MEASURE_HOURLY_ID		0xFF43			//�����������
#define	SHAKE_PHOTO_ID			0xFF44			//ҡһҡ����
#define	LANGUAGE_SETTING_ID		0xFF45			//��Ӣ�����л�
#define	TIME_24_SETTING_ID		0xFF46			//12/24Сʱ����
#define	FIND_PHONE_ID			0xFF47			//�����ֻ��ظ�
#define	WEATHER_INFOR_ID		0xFF48			//������Ϣ�·�
#define	TIME_SYNC_ID			0xFF49			//ʱ��ͬ��
#define	TARGET_STEPS_ID			0xFF50			//Ŀ�경��
#define	BATTERY_LEVEL_ID		0xFF51			//��ص���
#define	FIRMWARE_INFOR_ID		0xFF52			//�̼��汾��
#define	FACTORY_RESET_ID		0xFF53			//����ֻ�����
#define	ECG_ID					0xFF54			//�ĵ�
#define	LOCATION_ID				0xFF55			//��ȡ��λ��Ϣ
#define	DATE_FORMAT_ID			0xFF56			//�����ո�ʽ����

#define	BLE_CONNECT_ID			0xFFB0			//BLE��������


static u32_t rece_len=0;

static u8_t rx_buf[BUF_MAXSIZE]={0};
static u8_t tx_buf[BUF_MAXSIZE]={0};

static struct device *uart_dev;

static K_FIFO_DEFINE(fifo_uart_tx_data);
static K_FIFO_DEFINE(fifo_uart_rx_data);

struct uart_data_t {
	void  *fifo_reserved;
	u8_t    data[BUF_MAXSIZE];
	u16_t   len;
};

bool BLE_is_connected = false;

extern bool update_time;
extern bool update_date;
extern bool update_date_time;

extern bool show_date_time_first;
extern u8_t date_time_changed;

void ble_connect_or_disconnect_handle(u8_t *buf, u32_t len)
{
	printk("BLE status:%x\n", buf[6]);
	
	if(buf[6] == 0x01)				//�鿴controlֵ
		BLE_is_connected = true;
	else if(buf[6] == 0x00)
		BLE_is_connected = false;
	else
		BLE_is_connected = false;
}

void APP_set_time_24_format(u8_t *buf, u32_t len)
{
	u8_t reply[256] = {0};
	u32_t i,reply_len = 0;
	
	if(buf[7] == 0x00)
		global_settings.time_format = TIME_FORMAT_24;//24 format
	else if(buf[7] == 0x01)
		global_settings.time_format = TIME_FORMAT_12;//12 format
	else
		global_settings.time_format = TIME_FORMAT_24;//24 format

	update_time = true;

	//packet head
	reply[reply_len++] = PACKET_HEAD;
	//data_len
	reply[reply_len++] = 0x00;
	reply[reply_len++] = 0x06;
	//data ID
	reply[reply_len++] = (TIME_24_SETTING_ID>>8);		
	reply[reply_len++] = (u8_t)(TIME_24_SETTING_ID&0x00ff);
	//status
	reply[reply_len++] = 0x80;
	//control
	reply[reply_len++] = 0x00;
	//CRC
	reply[reply_len++] = 0x00;
	//packet end
	reply[reply_len++] = PACKET_END;

	for(i=0;i<(reply_len-2);i++)
		reply[reply_len-2] += reply[i];

	ble_send_date_handle(reply, reply_len);
}


void APP_set_date_format(u8_t *buf, u32_t len)
{
	u8_t reply[256] = {0};
	u32_t i,reply_len = 0;
	
	if(buf[7] == 0x00)
		global_settings.date_format = DATE_FORMAT_YYYYMMDD;
	else if(buf[7] == 0x01)
		global_settings.date_format = DATE_FORMAT_MMDDYYYY;
	else if(buf[7] == 0x02)
		global_settings.date_format = DATE_FORMAT_DDMMYYYY;
	else
		global_settings.date_format = DATE_FORMAT_YYYYMMDD;

	update_date = true;

	//packet head
	reply[reply_len++] = PACKET_HEAD;
	//data_len
	reply[reply_len++] = 0x00;
	reply[reply_len++] = 0x06;
	//data ID
	reply[reply_len++] = (DATE_FORMAT_ID>>8);		
	reply[reply_len++] = (u8_t)(DATE_FORMAT_ID&0x00ff);
	//status
	reply[reply_len++] = 0x80;
	//control
	reply[reply_len++] = 0x00;
	//CRC
	reply[reply_len++] = 0x00;
	//packet end
	reply[reply_len++] = PACKET_END;

	for(i=0;i<(reply_len-2);i++)
		reply[reply_len-2] += reply[i];

	ble_send_date_handle(reply, reply_len);
}

void APP_set_date_time(u8_t *buf, u32_t len)
{
	u8_t reply[256] = {0};
	u32_t i,reply_len = 0;
	sys_date_timer_t datetime = {0};
	
	datetime.year = 256*buf[7]+buf[8];
	datetime.month = buf[9];
	datetime.day = buf[10];
	datetime.hour = buf[11];
	datetime.minute = buf[12];
	datetime.second = buf[13];
	
	if(CheckSystemDateTimeIsValid(datetime))
	{
		datetime.week = GetWeekDayByDate(datetime);
		//SetSystemDateTime(datetime);

		memcpy(&date_time, &datetime, sizeof(sys_date_timer_t));

		update_date_time = true;
	}

	//packet head
	reply[reply_len++] = PACKET_HEAD;
	//data_len
	reply[reply_len++] = 0x00;
	reply[reply_len++] = 0x06;
	//data ID
	reply[reply_len++] = (TIME_SYNC_ID>>8);		
	reply[reply_len++] = (u8_t)(TIME_SYNC_ID&0x00ff);
	//status
	reply[reply_len++] = 0x80;
	//control
	reply[reply_len++] = 0x00;
	//CRC
	reply[reply_len++] = 0x00;
	//packet end
	reply[reply_len++] = PACKET_END;

	for(i=0;i<(reply_len-2);i++)
		reply[reply_len-2] += reply[i];

	ble_send_date_handle(reply, reply_len);	
}

/**********************************************************************************
*Name: ble_receive_date_handle
*Function:  �����������յ�������
*Parameter: 
*			Input:
*				buf ���յ�������
*				len ���յ������ݳ���
*			Output:
*				none
*			Return:
*				void
*Description:
*	���յ������ݰ��ĸ�ʽ����:
*	��ͷ			���ݳ���		����		״̬����	����		����1	���ݡ�		У��		��β
*	(StarFrame)		(Data length)	(ID)		(Status)	(Control)	(Data1)	(Data��)	(CRC8)		(EndFrame)
*	(1 bytes)		(2 byte)		(2 byte)	(1 byte)	(1 byte)	(��ѡ)	(��ѡ)		(1 bytes)	(1 bytes)
*
*	�������±���ʾ��
*	Offset	Field		Size	Value(ʮ������)		Description
*	0		StarFrame	1		0xAB				��ʼ֡
*	1		Data length	2		0x0000-0xFFFF		���ݳ���,��ID��ʼһֱ����β
*	3		Data ID		2		0x0000-0xFFFF	    ID
*	5		Status		1		0x00-0xFF	        Status
*	6		Control		1		0x00-0x01			����
*	7		Data0		1-14	0x00-0xFF			����0
*	8+n		CRC8		1		0x00-0xFF			����У��,�Ӱ�ͷ��ʼ��CRCǰһλ
*	9+n		EndFrame	1		0x88				����֡
**********************************************************************************/
void ble_receive_date_handle(u8_t *buf, u32_t len)
{
	u8_t CRC_data,data_status;
	u16_t data_len,data_ID;
	u32_t i;
	
	if((buf[0] != PACKET_HEAD) || (buf[len-1] != PACKET_END))	//format is error
	{
		printk("format is error! HEAD:%x, END:%x\n", buf[0], buf[len-1]);
		return;
	}

	for(i=0;i<len-2;i++)
		CRC_data = CRC_data+buf[i];

	if(CRC_data != buf[len-2])									//crc is error
	{
		printk("CRC is error! data:%x, CRC:%x\n", buf[len-2], CRC_data);
		return;
	}

	data_len = buf[1]*256+buf[2];
	data_ID = buf[3]*256+buf[4];

	switch(data_ID)
	{
	case HEART_RATE_ID:			//����
		break;
	case BLOOD_OXYGEN_ID:		//Ѫ��
		break;
	case BLOOD_PRESSURE_ID:		//Ѫѹ
		break;
	case ONE_KEY_MEASURE_ID:	//һ������
		break;
	case PULL_REFRESH_ID:		//����ˢ��
		break;
	case SLEEP_DETAILS_ID:		//˯������
		break;
	case FIND_DEVICE_ID:		//�����ֻ�
		break;
	case SMART_NOTIFY_ID:		//��������
		break;
	case ALARM_SETTING_ID:		//��������
		break;
	case USER_INFOR_ID:			//�û���Ϣ
		break;
	case SEDENTARY_ID:			//��������
		break;
	case SHAKE_SCREEN_ID:		//̧������
		break;
	case MEASURE_HOURLY_ID:		//�����������
		break;
	case SHAKE_PHOTO_ID:		//ҡһҡ����
		break;
	case LANGUAGE_SETTING_ID:	//��Ӣ�����л�
		break;
	case TIME_24_SETTING_ID:	//12/24Сʱ����
		APP_set_time_24_format(buf, len);
		break;
	case FIND_PHONE_ID:			//�����ֻ��ظ�
		break;
	case WEATHER_INFOR_ID:		//������Ϣ�·�
		break;
	case TIME_SYNC_ID:			//ʱ��ͬ��
		APP_set_date_time(buf, len);
		break;
	case TARGET_STEPS_ID:		//Ŀ�경��
		break;
	case BATTERY_LEVEL_ID:		//��ص���
		break;
	case FIRMWARE_INFOR_ID:		//�̼��汾��
		break;
	case FACTORY_RESET_ID:		//����ֻ�����
		break;
	case ECG_ID:				//�ĵ�
		break;
	case LOCATION_ID:			//��ȡ��λ��Ϣ
		break;
	case DATE_FORMAT_ID:		//�����ո�ʽ
		APP_set_date_format(buf, len);
		break;
	case BLE_CONNECT_ID:		//BLE��������
		ble_connect_or_disconnect_handle(buf, len);
		break;
	default:
		printk("data_id is unknown! \n");
		break;
	}
}

void ble_send_date_handle(u8_t *buf, u32_t len)
{
	printk("ble_send_date_handle\n");

	uart_fifo_fill(uart_dev, buf, len);
	uart_irq_tx_enable(uart_dev); 
}

static void uart_receive_data(u8_t data, u32_t datalen)
{
	rx_buf[rece_len++] = data;
	if(rece_len == (256*rx_buf[1]+rx_buf[2]+3))	//receivive complete
	{
		ble_receive_date_handle(rx_buf, rece_len);

		memset(rx_buf, 0, sizeof(rx_buf));
		rece_len = 0;
	}
	else				//continue receive
	{
	}
}

void uart_send_data(void)
{
	printk("uart_send_data\n");
	
	uart_fifo_fill(uart_dev, "Hello World!", strlen("Hello World!"));
	uart_irq_tx_enable(uart_dev); 
}

static void uart_cb(struct device *x)
{
	u8_t tmpbyte = 0;
	u32_t len=0;

	uart_irq_update(x);

	if(uart_irq_rx_ready(x)) 
	{
		while((len = uart_fifo_read(x, &tmpbyte, 1)) > 0)
			uart_receive_data(tmpbyte, 1);
	}
	
	if(uart_irq_tx_ready(x))
	{
		struct uart_data_t *buf;
		u16_t written = 0;

		buf = k_fifo_get(&fifo_uart_tx_data, K_NO_WAIT);
		/* Nothing in the FIFO, nothing to send */
		if(!buf)
		{
			uart_irq_tx_disable(x);
			return;
		}

		while(buf->len > written)
		{
			written += uart_fifo_fill(x, &buf->data[written], buf->len - written);
		}

		while (!uart_irq_tx_complete(x))
		{
			/* Wait for the last byte to get
			* shifted out of the module
			*/
		}

		if (k_fifo_is_empty(&fifo_uart_tx_data))
		{
			uart_irq_tx_disable(x);
		}

		k_free(buf);
	}
}

void ble_init(void)
{
	printk("ble_init\n");
	
	uart_dev = device_get_binding(UART_DEV);
	if(!uart_dev)
	{
		printk("Could not get %s device\n", UART_DEV);
		return;
	}

	uart_irq_callback_set(uart_dev, uart_cb);
	uart_irq_rx_enable(uart_dev);
}

void test_uart_ble(void)
{
	printk("test_uart_ble\n");
	
	ble_init();

	while(1)
	{
		ble_send_date_handle("Hello World!", strlen("Hello World!"));
		k_sleep(K_MSEC(1000));
	}
}

