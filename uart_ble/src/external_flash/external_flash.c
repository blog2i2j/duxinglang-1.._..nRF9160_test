/****************************************Copyright (c)************************************************
** File name:			external_flash.c
** Last modified Date:          
** Last Version:		V1.0   
** Created by:			л��
** Created date:		2020-06-30
** Version:			    1.0
** Descriptions:		���flash����Դ�ļ�
******************************************************************************************************/
#include <drivers/spi.h>
#include <drivers/gpio.h>
#include <zephyr.h>
#include <device.h>
#include <stdio.h>
#include <string.h>
#include "img.h"
#include "font.h"
#include "external_flash.h"

struct device *spi_flash;
struct device *gpio_flash;

//SPI���ͻ������飬ʹ��EasyDMAʱһ��Ҫ����Ϊstatic����
static uint8_t    spi_tx_buf[6] = {0};  
//SPI���ջ������飬ʹ��EasyDMAʱһ��Ҫ����Ϊstatic����
static uint8_t    spi_rx_buf[6] = {0};  

//SPI���ͻ������飬ʹ��EasyDMAʱһ��Ҫ����Ϊstatic����
//static uint8_t    my_tx_buf[4096] = {0};
//SPI���ͻ������飬ʹ��EasyDMAʱһ��Ҫ����Ϊstatic����
//static uint8_t    my_rx_buf[4096] = {0};

static struct spi_buf_set tx_bufs,rx_bufs;
static struct spi_buf tx_buff,rx_buff;

static struct spi_config spi_cfg;
static struct spi_cs_control spi_cs_ctr;

void SpiFlash_CS_LOW(void)
{
	gpio_pin_write(gpio_flash, CS, 0);
}

void SpiFlash_CS_HIGH(void)
{
	gpio_pin_write(gpio_flash, CS, 1);
}

/*****************************************************************************
** ��  ����д��һ���ֽ�
** ��  ����Dat����д�������
** ����ֵ����
******************************************************************************/
void Spi_WriteOneByte(uint8_t Dat)
{   
	int err;

	spi_tx_buf[0] = Dat;
	tx_buff.buf = spi_tx_buf;
	tx_buff.len = 1;
	tx_bufs.buffers = &tx_buff;
	tx_bufs.count = 1;

	SpiFlash_CS_LOW();
	err = spi_transceive(spi_flash, &spi_cfg, &tx_bufs, NULL);
	SpiFlash_CS_HIGH();	
	if(err)
	{
		printk("SPI error: %d\n", err);
	}
	else
	{
		printk("ok\n");
	}
}
/*****************************************************************************
** ��  ����дʹ��
** ��  ������
** ����ֵ����
******************************************************************************/
static void SpiFlash_Write_Enable(void)
{
	Spi_WriteOneByte(SPIFlash_WriteEnable);
}
/*****************************************************************************
** ��  ������ȡW25Q64FWоƬID
** ��  ������
** ����ֵ��16λID��W25Q64FWоƬIDΪ��0xEF16
******************************************************************************/
uint16_t SpiFlash_ReadID(void)
{
	int err;
	uint16_t dat = 0;

	//׼������
	spi_tx_buf[0] = SPIFlash_ReadID;
	spi_tx_buf[1] = 0x00;
	spi_tx_buf[2] = 0x00;
	spi_tx_buf[3] = 0x00;
	spi_tx_buf[4] = 0xFF;
	spi_tx_buf[5] = 0xFF;
	
	tx_buff.buf = spi_tx_buf;
	tx_buff.len = 6;
	tx_bufs.buffers = &tx_buff;
	tx_bufs.count = 1;

	memset(spi_rx_buf, 0, sizeof(spi_rx_buf));
	
	rx_buff.buf = spi_rx_buf;
	rx_buff.len = 6;
	rx_bufs.buffers = &rx_buff;
	rx_bufs.count = 1;

	SpiFlash_CS_LOW();
	err = spi_transceive(spi_flash, &spi_cfg, &tx_bufs, &rx_bufs);
	SpiFlash_CS_HIGH();
	
	if(err)
	{
		printk("SPI error: %d\n", err);
	}
	else
	{
		printk("TX sent: %x,%x,%x,%x,%x,%x\n", 
			spi_tx_buf[0],
			spi_tx_buf[1],
			spi_tx_buf[2],
			spi_tx_buf[3],
			spi_tx_buf[4],
			spi_tx_buf[5]
			);
		
		printk("RX recv: %x,%x,%x,%x,%x,%x\n", 
			spi_rx_buf[0],
			spi_rx_buf[1],
			spi_rx_buf[2],
			spi_rx_buf[3],
			spi_rx_buf[4],
			spi_rx_buf[5]
			);
		
		//����������������ֽڲ��Ƕ�ȡ��ID
		dat|=spi_rx_buf[4]<<8;  
		dat|=spi_rx_buf[5];
		
		printk("flash ID: %x\n", dat);
	}



	return dat;
}
/*****************************************************************************
** ��  ������ȡW25Q64FW״̬�Ĵ���
** ��  ������
** ����ֵ��
******************************************************************************/
static uint8_t SpiFlash_ReadSR(void)
{
	int err;

	spi_tx_buf[0] = SPIFlash_ReadStatusReg;
	spi_tx_buf[1] = 0x00;
	tx_buff.buf = spi_tx_buf;
	tx_buff.len = 2;
	tx_bufs.buffers = &tx_buff;
	tx_bufs.count = 1;

	memset(spi_rx_buf, 0, sizeof(spi_rx_buf));
	rx_buff.buf = spi_rx_buf;
	rx_buff.len = 2;
	rx_bufs.buffers = &rx_buff;
	rx_bufs.count = 1;

	SpiFlash_CS_LOW();
	err = spi_transceive(spi_flash, &spi_cfg, &tx_bufs, &rx_bufs);
	SpiFlash_CS_HIGH();
	
	if(err)
	{
		printk("SPI error: %d\n", err);
	}
	else
	{
		printk("StatusReg: %x\n", spi_rx_buf[1]);
	}
	
	return spi_rx_buf[1];
}

//�ȴ�W25Q64FW����
void SpiFlash_Wait_Busy(void)   
{   
	while((SpiFlash_ReadSR()&0x01)==0x01);  		// �ȴ�BUSYλ���
} 
/*****************************************************************************
** ��  ��������������W25Q64FW��С�Ĳ�����λ������
** ��  ����[in]SecAddr��������ַ
** ����ֵ����
******************************************************************************/
void SPIFlash_Erase_Sector(uint32_t SecAddr)
{
	int err;

	//����дʹ������
	SpiFlash_Write_Enable();

	//������������
	spi_tx_buf[0] = SPIFlash_SecErase;		
	//24λ��ַ
	spi_tx_buf[1] = (uint8_t)((SecAddr&0x00ff0000)>>16);
	spi_tx_buf[2] = (uint8_t)((SecAddr&0x0000ff00)>>8);
	spi_tx_buf[3] = (uint8_t)SecAddr;
	
	tx_buff.buf = spi_tx_buf;
	tx_buff.len = SPIFLASH_CMD_LENGTH;
	tx_bufs.buffers = &tx_buff;
	tx_bufs.count = 1;

	SpiFlash_CS_LOW();
	err = spi_transceive(spi_flash, &spi_cfg, &tx_bufs, NULL);
	SpiFlash_CS_HIGH();	
	if(err)
	{
		printk("SPI error: %d\n", err);
	}
	
	//�ȴ�W25Q64FW��ɲ���
	SpiFlash_Wait_Busy();
}
/*****************************************************************************
** ��  ����ȫƬ����W25Q64FW��ȫƬ���������ʱ�����ֵΪ��40��
** ��  ������
** ����ֵ����
******************************************************************************/
void SPIFlash_Erase_Chip(void)
{
	int err;
	
	//����дʹ������
	SpiFlash_Write_Enable();
	
	//ȫƬ��������
	spi_tx_buf[0] = SPIFlash_ChipErase;
	tx_buff.buf = spi_tx_buf;
	tx_buff.len = 1;
	tx_bufs.buffers = &tx_buff;
	tx_bufs.count = 1;

	SpiFlash_CS_LOW();
	err = spi_transceive(spi_flash, &spi_cfg, &tx_bufs, NULL);
	SpiFlash_CS_HIGH();
	
	if(err)
	{
		printk("SPI error: %d\n", err);
	}

	//�ȴ�W25Q64FW��ɲ���
	SpiFlash_Wait_Busy();	
}
/*****************************************************************************
** ��  ������ָ���ĵ�ַд������,���д��ĳ��Ȳ��ܳ����õ�ַ����ҳ���ʣ��ռ�
**         *pBuffer:ָ���д������ݻ���
**         WriteAddr:д�����ʼ��ַ
**         WriteBytesNum:д����ֽ�����һ�����256���ֽ�
** ����ֵ��RET_SUCCESS
******************************************************************************/
uint8_t SpiFlash_Write_Page(uint8_t *pBuffer, uint32_t WriteAddr, uint32_t size)
{
	int err;
	
	//���д������ݳ����Ƿ�Ϸ���д�볤�Ȳ��ܳ���ҳ��Ĵ�С
	if (size > (SPIFlash_PAGE_SIZE - (WriteAddr%SPIFlash_PAGE_SIZE)))
	{
		return false;
	}

	if(size == 0) 
		return false;

	//����дʹ������
	SpiFlash_Write_Enable();
	
	//ҳ�������
	spi_tx_buf[0] = SPIFlash_PageProgram;
	//24λ��ַ���ߵ�ַ��ǰ
	spi_tx_buf[1] = (uint8_t)((WriteAddr&0x00ff0000)>>16);
	spi_tx_buf[2] = (uint8_t)((WriteAddr&0x0000ff00)>>8);
	spi_tx_buf[3] = (uint8_t)WriteAddr;

	tx_buff.buf = spi_tx_buf;
	tx_buff.len = SPIFLASH_CMD_LENGTH;
	tx_bufs.buffers = &tx_buff;
	tx_bufs.count = 1;

	SpiFlash_CS_LOW();
	
	err = spi_transceive(spi_flash, &spi_cfg, &tx_bufs, NULL);
	if(err)
	{
		printk("SPI error: %d\n", err);
	}

	tx_buff.buf = pBuffer;
	tx_buff.len = size;
	tx_bufs.buffers = &tx_buff;
	tx_bufs.count = 1;

	err = spi_transceive(spi_flash, &spi_cfg, &tx_bufs, NULL);
	if(err)
	{
		printk("SPI error: %d\n", err);
	}

	SpiFlash_CS_HIGH();
	
	//�ȴ�W25Q64FW��ɲ���
	SpiFlash_Wait_Busy();

	return true;
}
/*****************************************************************************
** ��  ������ָ���ĵ�ַд�����ݣ���д����ҳ
**         *pBuffer:ָ���д�������
**         WriteAddr:д�����ʼ��ַ
**         size:д����ֽ���
** ����ֵ��RET_SUCCESS
******************************************************************************/
uint8_t SpiFlash_Write_Buf(uint8_t *pBuffer, uint32_t WriteAddr, uint32_t size)
{
    uint32_t PageByteRemain = 0;
	
	//������ʼ��ַ����ҳ���ʣ��ռ�
    PageByteRemain = SPIFlash_PAGE_SIZE - WriteAddr%SPIFlash_PAGE_SIZE;
	//�����̵����ݳ��Ȳ�����ҳ���ʣ��ռ䣬������ݳ��ȵ���size
    if(size <= PageByteRemain)
    {
        PageByteRemain = size;
    }
	//�ִα�̣�ֱ�����е����ݱ�����
    while(true)
    {
        //���PageByteRemain���ֽ�
		SpiFlash_Write_Page(pBuffer,WriteAddr,PageByteRemain);
		//��������ɣ��˳�ѭ��
        if(size == PageByteRemain)
        {
            break;
        }
        else
        {
            //������ȡ���ݵĻ����ַ
			pBuffer += PageByteRemain;
			//�����̵�ַ
            WriteAddr += PageByteRemain;
			//���ݳ��ȼ�ȥPageByteRemain
            size -= PageByteRemain;
			//�����´α�̵����ݳ���
            if(size > SPIFlash_PAGE_SIZE)
            {
                PageByteRemain = SPIFlash_PAGE_SIZE;
            }
            else
            {
                PageByteRemain = size;
            }
        }
    }
    return true;
}

/*****************************************************************************
** ��  ������ָ���ĵ�ַ����ָ�����ȵ�����
** ��  ����pBuffer��ָ���Ŷ������ݵ��׵�ַ       
**         ReadAddr�����������ݵ���ʼ��ַ
**         size���������ֽ�����ע��size���ܳ���pBuffer�Ĵ�С��������������
** ����ֵ��
******************************************************************************/
uint8_t SpiFlash_Read(uint8_t *pBuffer,uint32_t ReadAddr,uint32_t size)
{
	int err;
	uint8_t read_size;
	
	spi_tx_buf[0] = SPIFlash_ReadData;
	//24λ��ַ���ߵ�ַ��ǰ
	spi_tx_buf[1] = (uint8_t)((ReadAddr&0x00ff0000)>>16);
	spi_tx_buf[2] = (uint8_t)((ReadAddr&0x0000ff00)>>8);
	spi_tx_buf[3] = (uint8_t)ReadAddr;

	tx_buff.buf = spi_tx_buf;
	tx_buff.len = SPIFLASH_CMD_LENGTH;
	tx_bufs.buffers = &tx_buff;
	tx_bufs.count = 1;

	SpiFlash_CS_LOW();
	
	err = spi_transceive(spi_flash, &spi_cfg, &tx_bufs, NULL);
	if(err)
	{
		printk("SPI error: %d\n", err);
	}

	//��ʼ��ȡ����
	while(size!=0)
	{
		if(size<=SPI_TXRX_MAX_LEN)
		{
			read_size = size;
			size = 0;
		}
		else
		{
			read_size = SPI_TXRX_MAX_LEN;
			size -= SPI_TXRX_MAX_LEN;
		}

		rx_buff.buf = pBuffer;
		rx_buff.len = read_size;
		rx_bufs.buffers = &rx_buff;
		rx_bufs.count = 1;

		err = spi_transceive(spi_flash, &spi_cfg, NULL, &rx_bufs);
		if(err)
		{
			printk("SPI error: %d\n", err);
		}
		
		pBuffer += read_size;
	}

	SpiFlash_CS_HIGH();
	
    return true;
}


/*****************************************************************************
** ��  ����������������W25Q64FW�Ĺܽ�,�ر�ע��д�Ĺ�����CSҪһֱ��Ч�����ܽ���SPI�Զ�����
** ��  �Σ���
** ����ֵ����
******************************************************************************/
void SPI_Flash_Init(void)
{
	spi_flash = device_get_binding(FLASH_DEVICE);
	if (!spi_flash) 
	{
		printk("Could not get %s device\n", FLASH_DEVICE);
		return;
	}

	spi_cfg.operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8);
	spi_cfg.frequency = 4000000;
	spi_cfg.slave = 0;
}

void flash_init(void)
{
	printk("flash_init\n");
		
	gpio_flash = device_get_binding(FLASH_PORT);
	if(!gpio_flash)
	{
		printk("Cannot bind gpio device\n");
		return;
	}

	gpio_pin_configure(gpio_flash, CS, GPIO_DIR_OUT);

	SPI_Flash_Init();
}

void test_flash(void)
{
	uint16_t flash_id;
	uint16_t len;
	u8_t tmpbuf[128] = {0};
	
	//flash_init();


	LCD_ShowString(0,0,"FLASH���Կ�ʼ");

	flash_id = SpiFlash_ReadID();
	sprintf(tmpbuf, "FLASH ID:%X", flash_id);
	LCD_ShowString(0,20,tmpbuf);

#if 0
	//д֮ǰ��Ҫ��ִ�в�������
	LCD_ShowString(0,40,"FLASH��ʼ����...");
	SPIFlash_Erase_Chip();
	//SPIFlash_Erase_Sector(0);
	LCD_ShowString(0,60,"FLASH�����ɹ�!");
#endif

#if 0	
	//д������
	//LCD_ShowString(0,80,"FLASHд��ͼƬ1����...");
	//SpiFlash_Write_Buf(peppa_pig_80X160, IMG_PEPPA_80X160_ADDR, IMG_PEPPA_80X160_SIZE);
	//LCD_ShowString(0,100,"FLASHд��ͼƬ1�ɹ�!");

	//д������
	//LCD_ShowString(0,120,"FLASHд��ͼƬ2����...");
	//SpiFlash_Write_Buf(peppa_pig_160X160, IMG_PEPPA_160X160_ADDR, IMG_PEPPA_160X160_SIZE);
	//LCD_ShowString(0,140,"FLASHд��ͼƬ2�ɹ�!");

	//д������
	LCD_ShowString(0,160,"FLASHд��ͼƬ3����...");
	//SpiFlash_Write_Buf(peppa_pig_240X240_1, IMG_PEPPA_240X240_ADDR, 57608);
	SpiFlash_Write_Buf(peppa_pig_240X240_2, IMG_PEPPA_240X240_ADDR+57608, 57600);
	LCD_ShowString(0,180,"FLASHд��ͼƬ3�ɹ�!");

	//д������
	//LCD_ShowString(0,200,"FLASHд��ͼƬ4����...");
	//SpiFlash_Write_Buf(peppa_pig_320X320, IMG_PEPPA_320X320_ADDR, IMG_PEPPA_320X320_SIZE);
	//LCD_ShowString(0,220,"FLASHд��ͼƬ4�ɹ�!");
#endif

#if 0
	//д������
	LCD_ShowString(0,160,"FLASHд��16X08Ӣ���ֿ�...");
	SpiFlash_Write_Buf(asc2_1608, FONT_ASC_1608_ADDR, FONT_ASC_1608_SIZE);
	LCD_ShowString(0,180,"FLASHд��16X08Ӣ�ĳɹ�");

	//д������
	LCD_ShowString(0,40,"FLASHд��24X12Ӣ���ֿ�...");
	SpiFlash_Write_Buf(asc2_2412, FONT_ASC_2412_ADDR, FONT_ASC_2412_SIZE);
	LCD_ShowString(0,60,"FLASHд��24X12Ӣ�ĳɹ�");

	//д������
	LCD_ShowString(0,80,"FLASHд��32X16Ӣ���ֿ�...");
	SpiFlash_Write_Buf(asc2_3216, FONT_ASC_3216_ADDR, FONT_ASC_3216_SIZE);
	LCD_ShowString(0,100,"FLASHд��32X16Ӣ�ĳɹ�");
#endif

#if 0
	//д������
	LCD_ShowString(0,120,"FLASHд��16X16�����ֿ�...");
	//SpiFlash_Write_Buf(chinese_1616_1, FONT_CHN_SM_1616_ADDR+(2726*32)*0, (2726*32));
	//SpiFlash_Write_Buf(chinese_1616_2, FONT_CHN_SM_1616_ADDR+(2726*32)*1, (2726*32));
	SpiFlash_Write_Buf(chinese_1616_3, FONT_CHN_SM_1616_ADDR+(2726*32)*2, (2726*32));	
	LCD_ShowString(0,140,"FLASHд��16X16���ĳɹ�");
#endif

#if 0
	//д������
	LCD_ShowString(0,160,"FLASHд��24X24�����ֿ�...");
	//SpiFlash_Write_Buf(chinese_2424_1, FONT_CHN_SM_2424_ADDR+(1200*72)*0, (1200*72));
	//SpiFlash_Write_Buf(chinese_2424_2, FONT_CHN_SM_2424_ADDR+(1200*72)*1, (1200*72));
	//SpiFlash_Write_Buf(chinese_2424_3, FONT_CHN_SM_2424_ADDR+(1200*72)*2, (1200*72));
	//SpiFlash_Write_Buf(chinese_2424_4, FONT_CHN_SM_2424_ADDR+(1200*72)*3, (1200*72));
	//SpiFlash_Write_Buf(chinese_2424_5, FONT_CHN_SM_2424_ADDR+(1200*72)*4, (1200*72));
	//SpiFlash_Write_Buf(chinese_2424_6, FONT_CHN_SM_2424_ADDR+(1200*72)*5, (1200*72));
	SpiFlash_Write_Buf(chinese_2424_7, FONT_CHN_SM_2424_ADDR+(1200*72)*6, (977*72));
	LCD_ShowString(0,180,"FLASHд��24X24���ĳɹ�");
#endif

#if 0
	//д������
	LCD_ShowString(0,200,"FLASHд��32X32�����ֿ�...");
	//SpiFlash_Write_Buf(chinese_3232_1, FONT_CHN_SM_3232_ADDR+(700*128)*0, (700*128));
	//SpiFlash_Write_Buf(chinese_3232_2, FONT_CHN_SM_3232_ADDR+(700*128)*1, (700*128));
	//SpiFlash_Write_Buf(chinese_3232_3, FONT_CHN_SM_3232_ADDR+(700*128)*2, (700*128));
	//SpiFlash_Write_Buf(chinese_3232_4, FONT_CHN_SM_3232_ADDR+(700*128)*3, (700*128));
	//SpiFlash_Write_Buf(chinese_3232_5, FONT_CHN_SM_3232_ADDR+(700*128)*4, (700*128));
	//SpiFlash_Write_Buf(chinese_3232_6, FONT_CHN_SM_3232_ADDR+(700*128)*5, (700*128));
	//SpiFlash_Write_Buf(chinese_3232_7, FONT_CHN_SM_3232_ADDR+(700*128)*6, (700*128));
	//SpiFlash_Write_Buf(chinese_3232_8, FONT_CHN_SM_3232_ADDR+(700*128)*7, (700*128));
	//SpiFlash_Write_Buf(chinese_3232_9, FONT_CHN_SM_3232_ADDR+(700*128)*8, (700*128));
	//SpiFlash_Write_Buf(chinese_3232_10, FONT_CHN_SM_3232_ADDR+(700*128)*9, (700*128));
	//SpiFlash_Write_Buf(chinese_3232_11, FONT_CHN_SM_3232_ADDR+(700*128)*10, (700*128));
	SpiFlash_Write_Buf(chinese_3232_12, FONT_CHN_SM_3232_ADDR+(700*128)*11, (478*128));
	LCD_ShowString(0,220,"FLASHд��32X32���ĳɹ�");	
#endif
	
	//��������
	//SpiFlash_Read(my_rx_buf,0,len);
	//LCD_ShowString(0,140,"FLASH��������:");
	//LCD_ShowString(0,160,my_rx_buf);
}
