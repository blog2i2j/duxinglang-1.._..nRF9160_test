/*
* Copyright (c) 2019 Nordic Semiconductor ASA
*
* SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
*/
#include <nrf9160.h>
#include <kernel_structs.h>
#include <device.h>
#include <stdio.h>
#include <stdlib.h>
#include <zephyr.h>
#include <drivers/gpio.h>
#include <logging/log_ctrl.h>
#include <logging/log.h>
LOG_MODULE_REGISTER(dk_crc_test, CONFIG_LOG_DEFAULT_LEVEL);

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
 
const CRC_8 crc_8 = {0x07,0x00,0x00,false,false};
const CRC_8 crc_8_ITU = {0x07,0x00,0x55,false,false};
const CRC_8 crc_8_ROHC = {0x07,0xff,0x00,true,true};
const CRC_8 crc_8_MAXIM = {0x31,0x00,0x00,true,true};
const CRC_8 crc_8_CUSTOM = {0x31,0xff,0x00,false,false};

const CRC_16 crc_16_IBM = {0x8005,0x0000,0x0000,true,true};
const CRC_16 crc_16_MAXIM = {0x8005,0x0000,0xffff,true,true};
const CRC_16 crc_16_USB = {0x8005,0xffff,0xffff,true,true};
const CRC_16 crc_16_MODBUS = {0x8005,0xffff,0x0000,true,true};
const CRC_16 crc_16_CCITT = {0x1021,0x0000,0x0000,true,true};
const CRC_16 crc_16_CCITT_FALSE = {0x1021,0xffff,0x0000,false,false};
const CRC_16 crc_16_X5 = {0x1021,0xffff,0xffff,true,true};
const CRC_16 crc_16_XMODEM = {0x1021,0x0000,0x0000,false,false};
const CRC_16 crc_16_DNP = {0x3d65,0x0000,0xffff,true,true};
const CRC_16 crc_16_CUSTOM = {0x3d65,0x0000,0xffff,true,true};

const CRC_32 crc_32 = {0x04c11db7,0xffffffff,0xffffffff,true,true};
const CRC_32 crc_32_MPEG2 = {0x04c11db7,0xffffffff,0x00000000,false,false};
const CRC_32 crc_32_CUSTOM = {0x04c11db7,0xffffffff,0x00000000,false,false};

u8_t crc8_cal(u8_t *addr, int num, CRC_8 type);
u16_t crc16_cal(u8_t *addr, int num, CRC_16 type);
u32_t crc32_cal(u8_t *addr, int num, CRC_32 type);

/*****************************************************************************
*function name:reverse8
*function: �ֽڷ�ת����1100 0101 ��ת��Ϊ1010 0011
*input��1�ֽ�
*output:��ת���ֽ�
******************************************************************************/
u8_t reverse8(u8_t data)
{
    u8_t i;
    u8_t temp=0;
	
    for(i=0;i<8;i++)	//�ֽڷ�ת
        temp |= ((data>>i) & 0x01)<<(7-i);
    return temp;
}

/*****************************************************************************
*function name:reverse16
*function: ˫�ֽڷ�ת����1100 0101 1110 0101��ת��Ϊ1010 0111 1010 0011
*input��˫�ֽ�
*output:��ת��˫�ֽ�
******************************************************************************/
u16_t reverse16(u16_t data)
{
    u8_t i;
    u16_t temp=0;
	
    for(i=0;i<16;i++)		//��ת
        temp |= ((data>>i) & 0x0001)<<(15-i);
    return temp;
}

/*****************************************************************************
*function name:reverse32
*function: 32bit�ַ�ת
*input��32bit��
*output:��ת��32bit��
******************************************************************************/
u32_t reverse32(u32_t data)
{
    u8_t i;
    u32_t temp=0;
	
    for(i=0;i<32;i++)		//��ת
        temp |= ((data>>i) & 0x01)<<(31-i);
    return temp;
}
 
/*****************************************************************************
*function name:crc8_cal
*function: CRCУ�飬У��ֵΪ8λ
*input:addr-�����׵�ַ��num-���ݳ��ȣ��ֽڣ���type-CRC8���㷨����
*output:8λУ��ֵ
******************************************************************************/
u8_t crc8_cal(u8_t *addr, int num, CRC_8 type)  
{  
    u8_t data;
    u8_t crc = type.InitValue;					//��ʼֵ
    int i;
	
    for(;num>0;num--)               
    {  
        data = *addr++;
        if(type.InputReverse == true)
        	data = reverse8(data);				//�ֽڷ�ת
        crc = crc^data;							//��crc��ʼֵ��� 
        for(i=0;i<8;i++)						//ѭ��8λ 
        {  
            if(crc&0x80)						//�����Ƴ���λΪ1�����ƺ������ʽ���
                crc = (crc<<1)^type.poly;    
            else								//����ֱ������
                crc <<= 1;                  
        }
    }

    if(type.OutputReverse == true)				//������������ת
        crc = reverse8(crc);
    crc = crc^type.xor;							//����������ֵ���
    return(crc);								//��������У��ֵ
}
 
/*****************************************************************************
*function name:crc16_cal
*function: CRCУ�飬У��ֵΪ16λ
*input:addr-�����׵�ַ��num-���ݳ��ȣ��ֽڣ���type-CRC16���㷨����
*output:16λУ��ֵ
******************************************************************************/
u16_t crc16_cal(u8_t *addr, int num, CRC_16 type)  
{  
    u8_t data;
    u16_t crc = type.InitValue;					//��ʼֵ
    int i; 
	
    for(;num>0;num--)               
    {  
        data = *addr++;
        if(type.InputReverse == true)
            data = reverse8(data);				//�ֽڷ�ת
        crc = crc^(data<<8);					//��crc��ʼֵ��8λ��� 
        for(i=0;i<8;i++)						//ѭ��8λ 
        {  
            if(crc&0x8000)						//�����Ƴ���λΪ1�����ƺ������ʽ���
                crc = (crc<<1)^type.poly;    
            else								//����ֱ������
                crc <<= 1;                  
        }
    }
    if(type.OutputReverse == true)				//������������ת
        crc = reverse16(crc);
    crc = crc^type.xor;							//����������ֵ���
    return(crc);								//��������У��ֵ
}

/*****************************************************************************
*function name:crc32_cal
*function: CRCУ�飬У��ֵΪ32λ
*input:addr-�����׵�ַ��num-���ݳ��ȣ��ֽڣ���type-CRC32���㷨����
*output:32λУ��ֵ
******************************************************************************/
u32_t crc32_cal(u8_t *addr, int num, CRC_32 type)  
{  
    u8_t data;
    u32_t crc = type.InitValue;					//��ʼֵ
    int i;
	
    for(;num>0;num--)               
    {  
        data = *addr++;
        if(type.InputReverse == true)
            data = reverse8(data);				//�ֽڷ�ת
        crc = crc^(data<<24);					//��crc��ʼֵ��8λ��� 
        for(i=0;i<8;i++)						//ѭ��8λ 
        {  
            if(crc&0x80000000)					//�����Ƴ���λΪ1�����ƺ������ʽ���
                crc = (crc<<1)^type.poly;    
            else								//����ֱ������
                crc <<= 1;                  
        }
    }
    if(type.OutputReverse == true)				//������������ת
        crc = reverse32(crc);
    crc = crc^type.xor;							//����������ֵ���
    return(crc);								//��������У��ֵ
}

void main(void)
{
	u8_t crc;
	u8_t data[] = "355755000000109";
	
	LOG_INF("Application started\n");

	crc = crc8_cal(data, strlen(data), crc_8_CUSTOM);

	LOG_INF("crc_8_CUSTOM:%02x\n", crc);

}