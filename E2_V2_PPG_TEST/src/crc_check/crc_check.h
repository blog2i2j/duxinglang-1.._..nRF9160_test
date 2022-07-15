/****************************************Copyright (c)************************************************
** File Name:			    crc_check.h
** Descriptions:			data crc check process head file
** Created By:				xie biao
** Created Date:			2021-12-27
** Modified Date:      		2021-12-27 
** Version:			    	V1.0
******************************************************************************************************/
#ifndef __CRC_CHECK_H__
#define __CRC_CHECK_H__

typedef struct
{
	u8_t poly;				//����ʽ
	u8_t InitValue;			//��ʼֵ
	u8_t xor;				//������ֵ
	bool InputReverse;		//���뷴ת
	bool OutputReverse;		//�����ת
}CRC_8;
 
typedef struct
{
	u16_t poly;				//����ʽ
	u16_t InitValue;		//��ʼֵ
	u16_t xor;				//������ֵ
	bool InputReverse;		//���뷴ת
	bool OutputReverse;		//�����ת
}CRC_16;
 
typedef struct
{
	u32_t poly;				//����ʽ
	u32_t InitValue;		//��ʼֵ
	u32_t xor;				//������ֵ
	bool InputReverse;		//���뷴ת
	bool OutputReverse;		//�����ת
}CRC_32;


extern CRC_8 crc_8;
extern CRC_8 crc_8_ITU;
extern CRC_8 crc_8_ROHC;
extern CRC_8 crc_8_MAXIM;

extern CRC_16 crc_16_IBM;
extern CRC_16 crc_16_MAXIM;
extern CRC_16 crc_16_USB;
extern CRC_16 crc_16_MODBUS;
extern CRC_16 crc_16_CCITT;
extern CRC_16 crc_16_CCITT_FALSE;
extern CRC_16 crc_16_X5;
extern CRC_16 crc_16_XMODEM;
extern CRC_16 crc_16_DNP;

extern CRC_32 crc_32;
extern CRC_32 crc_32_MPEG2;


extern u8_t crc8_cal(u8_t *addr, int num, CRC_8 type);
extern u16_t crc16_cal(u8_t *addr, int num, CRC_16 type);
extern u32_t crc32_cal(u8_t *addr, int num, CRC_32 type);

#endif/*__CRC_CHECK_H__*/