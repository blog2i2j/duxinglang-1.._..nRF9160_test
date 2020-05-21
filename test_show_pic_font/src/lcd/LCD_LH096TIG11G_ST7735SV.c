#include <drivers/spi.h>
#include <drivers/gpio.h>

#include "lcd.h"
#include "font.h" 

#ifdef LCD_LH096TIG11G_ST7735SV
#include "LCD_LH096TIG11G_ST7735SV.h"

#define SPI_DEV "SPI_3"
#define SPI_BUF_LEN	8

struct device *spi_lcd;
struct device *gpio_lcd;

static struct spi_config spi_cfg;
static struct spi_cs_control spi_cs_ctr;

static u8_t tx_buffer[SPI_BUF_LEN] = {0};
static u8_t rx_buffer[SPI_BUF_LEN] = {0};

bool lcd_is_sleeping = true;

static void spi_init(void)
{
	printk("spi_init\n");
	
	spi_lcd = device_get_binding(SPI_DEV);
	if (!spi_lcd) 
	{
		printk("Could not get %s device\n", SPI_DEV);
		return;
	}

	spi_cfg.operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB | SPI_MODE_CPOL | SPI_MODE_CPHA;
	spi_cfg.frequency = 4000000;
	spi_cfg.slave = 0;

	spi_cs_ctr.gpio_dev = device_get_binding(LCD_PORT);
	if (!spi_cs_ctr.gpio_dev)
	{
		printk("Unable to get GPIO SPI CS device\n");
		return;
	}

	spi_cs_ctr.gpio_pin = CS;
	spi_cs_ctr.delay = 0U;
	spi_cfg.cs = &spi_cs_ctr;
}

//LCD��ʱ����
void Delay(unsigned int dly)
{
	k_sleep(K_MSEC(dly));
}

//���ݽӿں���
//i:8λ����
void Write_Data(uint8_t i) 
{	
	int err;

	tx_buffer[0] = i;

	const struct spi_buf tx_buf = {
		  .buf = tx_buffer,
		  .len = 1
		};
	const struct spi_buf_set tx = {
		  .buffers = &tx_buf,
		  .count = 1
		};

	struct spi_buf rx_buf = {
		  .buf = rx_buffer,
		  .len = sizeof(rx_buffer),
		};
	const struct spi_buf_set rx = {
		  .buffers = &rx_buf,
		  .count = 1
		};

	err = spi_transceive(spi_lcd, &spi_cfg, &tx, &rx);
	if(err)
	{
		printk("SPI error: %d\n", err);
	}
	else
	{
		//printk("Write_Data sent: %x\n", tx_buffer[0]);
		//printk("Write_Data recv: %x\n", rx_buffer[0]);
	}	
}

//----------------------------------------------------------------------
//д�Ĵ�������
//i:�Ĵ���ֵ
void WriteComm(unsigned int i)
{
	//gpio_pin_write(gpio_lcd, CS, 0);//CS��0
	gpio_pin_write(gpio_lcd, RS, 0);//RS��0

	Write_Data(i);  

	//gpio_pin_write(gpio_lcd, CS, 1);
}

//дLCD����
//i:Ҫд���ֵ
void WriteData(unsigned int i)
{
	//gpio_pin_write(gpio_lcd, CS, 0);
	gpio_pin_write(gpio_lcd, RS, 1);
		
	Write_Data(i);  

	//gpio_pin_write(gpio_lcd, CS, 0);
}

void WriteDispData(unsigned char DataH,unsigned char DataL)
{
	Write_Data(DataH);  
	Write_Data(DataL);  
}

//LCD���㺯��
//color:Ҫ������ɫ
void WriteOneDot(unsigned int color)
{ 
	//gpio_pin_write(gpio_lcd, CS, 0);
	gpio_pin_write(gpio_lcd, RS, 1);

	Write_Data(color>>8);  
	Write_Data(color);  

	//gpio_pin_write(gpio_lcd, CS, 1);
}

////////////////////////////////////////////////���Ժ���//////////////////////////////////////////
void BlockWrite(unsigned int x,unsigned int y,unsigned int w,unsigned int h) //reentrant
{
	x += 26;
	y += 1;

	WriteComm(0x2A);             
	WriteData(x>>8);             
	WriteData(x);             
	WriteData((x+w)>>8);             
	WriteData((x+w));             

	WriteComm(0x2B);             
	WriteData(y>>8);             
	WriteData(y);             
	WriteData((y+h)>>8);//	WriteData((Yend+1)>>8);             
	WriteData((y+h));//	WriteData(Yend+1);   	

	WriteComm(0x2c);
}

void DispColor(unsigned int color)
{
	unsigned int i,j;

	BlockWrite(0,0,COL-1,ROW-1);

	gpio_pin_write(gpio_lcd, RS, 1);

	for(i=0;i<ROW;i++)
	{
		for(j=0;j<COL;j++)
		{    
			Write_Data(color>>8);  
			Write_Data(color);  
		}
	}
}

//���Ժ�������ʾRGB���ƣ�
void DispBand(void)	 
{
  unsigned int i,j,k;
  //unsigned int color[8]={0x001f,0x07e0,0xf800,0x07ff,0xf81f,0xffe0,0x0000,0xffff};
  unsigned int color[8]={0xf800,0xf800,0x07e0,0x07e0,0x001f,0x001f,0xffff,0xffff};//0x94B2
  //unsigned int gray16[]={0x0000,0x1082,0x2104,0x3186,0x42,0x08,0x528a,0x630c,0x738e,0x7bcf,0x9492,0xa514,0xb596,0xc618,0xd69a,0xe71c,0xffff};

  BlockWrite(0,0,COL-1,ROW-1);

  gpio_pin_write(gpio_lcd, RS, 1);

  for(i=0;i<8;i++)
  {
    for(j=0;j<ROW/8;j++)
    {
      for(k=0;k<COL;k++)
      {
        Write_Data(color[i]>>8);  
        Write_Data(color[i]);  
      } 
    }
  }
  for(j=0;j<(ROW%8);j++)
  {
    for(k=0;k<COL;k++)
    {
      Write_Data(color[7]>>8);  
      Write_Data(color[7]);  
    } 
  }
}

//���Ժ��������߿�
void DispFrame(void)
{
	unsigned int i,j;

	BlockWrite(0,0,COL-1,ROW-1);

	gpio_pin_write(gpio_lcd, RS, 1);

	Write_Data(0xf8);  
	Write_Data(0x00);  

	for(i=0;i<COL-2;i++)
	{
		Write_Data(0xFF);  
		Write_Data(0xFF);  
	}
	Write_Data(0x00);  
	Write_Data(0x1F);  

	for(j=0;j<ROW-2;j++)
	{
		Write_Data(0xf8);  
		Write_Data(0x00);  
		for(i=0;i<COL-2;i++)
		{
			Write_Data(0x00);  
			Write_Data(0x00);  
		}
		Write_Data(0x00);  
		Write_Data(0x1f);  
	}

	Write_Data(0xf8);  
	Write_Data(0x00);  
	for(i=0;i<COL-2;i++)
	{
		Write_Data(0xff);  
		Write_Data(0xff);  
	}
	Write_Data(0x00);  
	Write_Data(0x1f);  
}

//////////////////////////////////////////////////////////////////////////////////////////////
bool LCD_CheckID(void)
{
	WriteComm(0x04);
	Delay(10); 

	if(rx_buffer[0] == 0x89 && rx_buffer[1] == 0xF0)
		return true;
	else
		return false;
}

//��������
//color:Ҫ���������ɫ
void LCD_Clear(uint16_t color)
{
	u32_t index=0;      
	u32_t totalpoint=ROW;

	totalpoint*=COL; 			//�õ��ܵ���

	BlockWrite(0,0,COL-1,ROW-1);//��λ

	gpio_pin_write(gpio_lcd, RS, 1);

	for(index=0;index<totalpoint;index++)
	{
		Write_Data(color>>8);
		Write_Data(color);  
	}
} 

//��Ļ˯��
void LCD_SleepIn(void)
{
	if(lcd_is_sleeping)
		return;
	
    WriteComm(0x28);	
	WriteComm(0x10);  		//Sleep in	
	Delay(120);             //��ʱ120ms

	//�رձ���
	gpio_pin_write(gpio_lcd, LEDK, 1);

	lcd_is_sleeping = true;
}

//��Ļ����
void LCD_SleepOut(void)
{
	if(!lcd_is_sleeping)
		return;
	
	WriteComm(0x11);  		//Sleep out	
	Delay(120);             //��ʱ120ms
    WriteComm(0x29);

	//��������
	gpio_pin_write(gpio_lcd, LEDK, 0);
                                                                                                                  
	lcd_is_sleeping = false;
}

//LCD��ʼ������
void LCD_Init(void)
{
	int err;

	printk("LCD_Init\n");
	
  	//�˿ڳ�ʼ��
  	gpio_lcd = device_get_binding(LCD_PORT);
	if(!gpio_lcd)
	{
		printk("Cannot bind gpio device\n");
		return;
	}

	err = gpio_pin_configure(gpio_lcd, LEDK, GPIO_DIR_OUT);
	if(err)
	{
		printk("Cannot configure LEDK gpio\n");
		return;
	}
	
	err = gpio_pin_configure(gpio_lcd, CS, GPIO_DIR_OUT);
	if(err)
	{
		printk("Cannot configure CS gpio\n");
		return;
	}
	
	err = gpio_pin_configure(gpio_lcd, RST, GPIO_DIR_OUT);
	if(err)
	{
		printk("Cannot configure RST gpio\n");
		return;
	}
	
	err = gpio_pin_configure(gpio_lcd, RS, GPIO_DIR_OUT);
	if(err)
	{
		printk("Cannot configure RS gpio\n");
		return;
	}

	spi_init();

	gpio_pin_write(gpio_lcd, RST, 1);
	Delay(10);
	gpio_pin_write(gpio_lcd, RST, 0);
	Delay(10);
	gpio_pin_write(gpio_lcd, RST, 1);
	Delay(120);

	WriteComm(0x11);     //Sleep out
	Delay(120);          //Delay 120ms

	WriteComm(0xB1);     //Normal mode
	WriteData(0x05);   
	WriteData(0x3C);   
	WriteData(0x3C);  

	WriteComm(0xB2);     //Idle mode
	WriteData(0x05);   
	WriteData(0x3C);   
	WriteData(0x3C);  

	WriteComm(0xB3);     //Partial mode
	WriteData(0x05);   
	WriteData(0x3C);   
	WriteData(0x3C);   
	WriteData(0x05);   
	WriteData(0x3C);   
	WriteData(0x3C);  

	WriteComm(0xB4);     //Dot inversion
	WriteData(0x00); 

	WriteComm(0xB6);    //column inversion
	WriteData(0xB4); 
	WriteData(0xF0);	

	WriteComm(0xC0);     //AVDD GVDD
	WriteData(0xAB);   
	WriteData(0x0B);   
	WriteData(0x04);  

	WriteComm(0xC1);     //VGH VGL
	WriteData(0xC5);   	//C0

	WriteComm(0xC2);     //Normal Mode
	WriteData(0x0D);   
	WriteData(0x00);  

	WriteComm(0xC3);     //Idle
	WriteData(0x8D);   
	WriteData(0x6A);  

	WriteComm(0xC4);     //Partial+Full
	WriteData(0x8D);   
	WriteData(0xEE); 

	WriteComm(0xC5);     //VCOM
	WriteData(0x0F);  

	WriteComm(0x36); 	//MX,MY,RGB mode
	WriteData(0xC8); 	//my mx ml MV,rgb,000; =1,=MH=MX=MY=ML=0 and RGB filter panel 
	    
	WriteComm(0xE0);     //positive gamma
	WriteData(0x07);   
	WriteData(0x0E);   
	WriteData(0x08);   
	WriteData(0x07);   
	WriteData(0x10);   
	WriteData(0x07);   
	WriteData(0x02);   
	WriteData(0x07);   
	WriteData(0x09);   
	WriteData(0x0F);   
	WriteData(0x25);   
	WriteData(0x36);   
	WriteData(0x00);   
	WriteData(0x08);   
	WriteData(0x04);   
	WriteData(0x10); 

	WriteComm(0xE1);     //negative gamma
	WriteData(0x0A);   
	WriteData(0x0D);   
	WriteData(0x08);   
	WriteData(0x07);   
	WriteData(0x0F);   
	WriteData(0x07);   
	WriteData(0x02);   
	WriteData(0x07);   
	WriteData(0x09);   
	WriteData(0x0F);   
	WriteData(0x25);   
	WriteData(0x35);   
	WriteData(0x00);   
	WriteData(0x09);   
	WriteData(0x04);   
	WriteData(0x10);

	WriteComm(0xFC);    
	WriteData(0x80);  

	WriteComm(0xF0);    
	WriteData(0x11);  

	WriteComm(0xD6);    
	WriteData(0xCB);  

	WriteComm(0x3A);     
	WriteData(0x05);  

	WriteComm(0x21);     //Display inversion
	WriteComm(0x29);     //Display on

	//��������
	gpio_pin_write(gpio_lcd, LEDK, 0);

	lcd_is_sleeping = false;

	LCD_Clear(BLACK);		//����Ϊ��ɫ
}

#endif
