#include <stdlib.h>
#include <stdint.h>
#include "lcd.h"
#include "font.h"
#include "external_flash.h"

#if defined(LCD_R154101_ST7796S)
#include "LCD_R154101_ST7796S.h"
#elif defined(LCD_LH096TIG11G_ST7735SV)
#include "LCD_LH096TIG11G_ST7735SV.h"
#elif defined(LCD_ORCT012210N_ST7789V2)
#include "LCD_ORCT012210N_ST7789V2.h"
#elif defined(LCD_R108101_GC9307)
#include "LCD_R108101_GC9307.h"
#endif 


//LCD��Ļ�ĸ߶ȺͿ��
uint16_t LCD_WIDTH = COL;
uint16_t LCD_HEIGHT = ROW;

//LCD�Ļ�����ɫ�ͱ���ɫ	   
uint16_t POINT_COLOR=WHITE;	//������ɫ
uint16_t BACK_COLOR=BLACK;  //����ɫ 

//Ĭ�������С
#ifdef FONT_16
system_font_size system_font = FONT_SIZE_16;
#elif defined(FONT_24)
system_font_size system_font = FONT_SIZE_24;
#elif defined(FONT_32)
system_font_size system_font = FONT_SIZE_32;
#else
system_font_size system_font = FONT_SIZE_16;
#endif

//���ٻ���
//x,y:����
//color:��ɫ
void LCD_Fast_DrawPoint(uint16_t x, uint16_t y, uint16_t color)
{	   
	BlockWrite(x,y,1,1);	//������
	WriteOneDot(color);				//���㺯��	
}	 

//��ָ����������䵥����ɫ
//(x,y),(w,h):�����ζԽ�����,�����СΪ:w*h   
//color:Ҫ������ɫ
void LCD_Fill(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{          
	u32_t i;

	if((x+w)>LCD_WIDTH)
		w = LCD_WIDTH - x;
	if((y+h)>LCD_HEIGHT)
		h = LCD_HEIGHT - y;
	
	BlockWrite(x,y,w,h);

#ifdef LCD_TYPE_SPI
	DispColor((w*h), color);
#else
	for(i=0;i<(w*h);i++)
		WriteOneDot(color); //��ʾ��ɫ 
#endif
}

//��ָ�����������ָ����ɫ��	(��ʾͼƬ)		 
//(x,y),(w,h):�����ζԽ�����,�����СΪ:w*h   
//color:Ҫ������ɫ
void LCD_Pic_Fill(uint16_t x, uint16_t y, uint16_t w, uint16_t h, unsigned char *color)
{  
	uint16_t high,width;
	uint16_t i,j;
	u8_t databuf[2*COL] = {0};
	
	width=256*color[2]+color[3]; 			//��ȡͼƬ���
	high=256*color[4]+color[5];			//��ȡͼƬ�߶�
	
 	for(i=0;i<h;i++)
	{
		BlockWrite(x,y+i,w,1);	  	//����ˢ��λ��

	#ifdef LCD_TYPE_SPI
		for(j=0;j<w;j++)
		{
			databuf[2*j] = color[8+2*(i*width+j)];
			databuf[2*j+1] = color[8+2*(i*width+j)+1];
		}

		DispDate(2*w, databuf);
	#else
		for(j=0;j<w;j++)
			WriteDispData(color[8+2*(i*width+j)],color[8+2*(i*width+j)+1]);	//��ʾ��ɫ 
	#endif
	}			
} 

//����
//x1,y1:�������
//x2,y2:�յ�����  
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol;
	
	delta_x=x2-x1; //������������ 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //���õ������� 
	else if(delta_x==0)incx=0;//��ֱ�� 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//ˮƽ�� 
	else{incy=-1;delta_y=-delta_y;} 
	if(delta_x>delta_y)distance=delta_x; //ѡȡ�������������� 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//������� 
	{  
		LCD_Fast_DrawPoint(uRow,uCol,POINT_COLOR);//���� 
		xerr+=delta_x;
		yerr+=delta_y;
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
}    

//������	  
//(x1,y1),(x2,y2):���εĶԽ�����
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	LCD_DrawLine(x1,y1,x2,y1);
	LCD_DrawLine(x1,y1,x1,y2);
	LCD_DrawLine(x1,y2,x2,y2);
	LCD_DrawLine(x2,y1,x2,y2);
}

//��ָ��λ�û�һ��ָ����С��Բ
//(x,y):���ĵ�
//r    :�뾶
void LCD_Draw_Circle(uint16_t x0, uint16_t y0, uint8_t r)
{
	int a,b;
	int di;
	a=0;b=r;	  
	di=3-(r<<1);             //�ж��¸���λ�õı�־
	while(a<=b)
	{
		LCD_Fast_DrawPoint(x0+a,y0-b,POINT_COLOR);//5
		LCD_Fast_DrawPoint(x0+b,y0-a,POINT_COLOR);//0 
		LCD_Fast_DrawPoint(x0+b,y0+a,POINT_COLOR);//4
		LCD_Fast_DrawPoint(x0+a,y0+b,POINT_COLOR);//6
		
		LCD_Fast_DrawPoint(x0-a,y0+b,POINT_COLOR);//1
		LCD_Fast_DrawPoint(x0-b,y0+a,POINT_COLOR);
		LCD_Fast_DrawPoint(x0-a,y0-b,POINT_COLOR);//2
		LCD_Fast_DrawPoint(x0-b,y0-a,POINT_COLOR);//7 
                	         
		a++;
		//ʹ��Bresenham�㷨��Բ     
		if(di<0)di +=4*a+6;	  
		else
		{
			di+=10+4*(a-b);   
			b--;
		} 						    
	}
} 

//��ȡͼƬ�ߴ�
//color:ͼƬ����ָ��
//width:��ȡ����ͼƬ��������ַ
//height:��ȡ����ͼƬ�߶������ַ
void LCD_get_pic_size(unsigned char *color, uint16_t *width, uint16_t *height)
{
	*width = 256*color[2]+color[3];
	*height = 256*color[4]+color[5];
}

//��ȡflash�е�ͼƬ�ߴ�
//color:ͼƬ����ָ��
//width:��ȡ����ͼƬ��������ַ
//height:��ȡ����ͼƬ�߶������ַ
void LCD_get_pic_size_from_flash(u32_t pic_addr, uint16_t *width, uint16_t *height)
{
	u8_t databuf[8] = {0};

	SpiFlash_Read(databuf, pic_addr, 8);
	
	*width = 256*databuf[2]+databuf[3]; 			//��ȡͼƬ���
	*height = 256*databuf[4]+databuf[5];			//��ȡͼƬ�߶�

}

//ָ��λ����ʾͼƬ
//color:ͼƬ����ָ��
//x:ͼƬ��ʾX����
//y:ͼƬ��ʾY����
void LCD_dis_pic(uint16_t x, uint16_t y, unsigned char *color)
{  
	uint16_t h,w,show_w,show_h;
	uint16_t i;
	u8_t databuf[LCD_DATA_LEN]={0};
	u32_t offset=8,datelen,showlen=0,readlen=LCD_DATA_LEN;
	
	w=256*color[2]+color[3]; 			//��ȡͼƬ���
	h=256*color[4]+color[5];			//��ȡͼƬ�߶�

	if((x+w)>LCD_WIDTH)
		show_w = LCD_WIDTH-x;
	else
		show_w = w;
	
	if((y+h)>LCD_HEIGHT)
		show_h = LCD_HEIGHT-y;
	else
		show_h = h;

	BlockWrite(x,y,show_w,show_h);	//����ˢ��λ��
	datelen = 2*show_w*show_h;
	if(show_w < w)
		readlen = 2*show_w;
	
	while(datelen)
	{
		if(datelen < readlen)
		{
			readlen = datelen;
			datelen = 0;
		}
		else
		{
			readlen = readlen;
			datelen -= readlen;
		}
		
		memset(databuf, 0, LCD_DATA_LEN);
		memcpy(databuf, &color[offset], readlen);
		
		if(show_w < w)
			offset += 2*w;
		else
			offset += readlen;

	#ifdef LCD_TYPE_SPI
		DispDate(readlen, databuf);
	#else
		for(i=0;i<(readlen/2);i++)
			WriteDispData(databuf[2*i],databuf[2*i+1]);	//��ʾ��ɫ 
	#endif
	}
}


//ָ��λ����ʾflash�е�ͼƬ
//pic_addr:ͼƬ��flash�еĵ�ַ
//x:ͼƬ��ʾX����
//y:ͼƬ��ʾY����
void LCD_dis_pic_from_flash(uint16_t x, uint16_t y, u32_t pic_addr)
{
	uint16_t h,w,show_w,show_h;
	uint16_t i;
	u8_t databuf[LCD_DATA_LEN]={0};
	u32_t datelen,showlen=0,readlen=LCD_DATA_LEN;
	
	SpiFlash_Read(databuf, pic_addr, 8);

	w=256*databuf[2]+databuf[3]; 			//��ȡͼƬ���
	h=256*databuf[4]+databuf[5];			//��ȡͼƬ�߶�

	pic_addr += 8;

	if((x+w)>LCD_WIDTH)
		show_w = LCD_WIDTH-x;
	else
		show_w = w;
	
	if((y+h)>LCD_HEIGHT)
		show_h = LCD_HEIGHT-y;
	else
		show_h = h;
	
	BlockWrite(x,y,show_w,show_h);	//����ˢ��λ��

	datelen = 2*show_w*show_h;
	if(show_w < w)
		readlen = 2*show_w;
	
	while(datelen)
	{
		if(datelen < readlen)
		{
			readlen = datelen;
			datelen = 0;
		}
		else
		{
			readlen = readlen;
			datelen -= readlen;
		}
		
		memset(databuf, 0, LCD_DATA_LEN);
		SpiFlash_Read(databuf, pic_addr, readlen);
		
		if(show_w < w)
			pic_addr += 2*w;
		else
			pic_addr += readlen;

	#ifdef LCD_TYPE_SPI
		DispDate(readlen, databuf);
	#else
		for(i=0;i<(readlen/2);i++)
			WriteDispData(databuf[2*i],databuf[2*i+1]);	//��ʾ��ɫ 
	#endif
	}
}

//ָ��λ����ʾͼƬ,����ɫ����
//color:ͼƬ����ָ��
//x:ͼƬ��ʾX����
//y:ͼƬ��ʾY����
void LCD_dis_trans_pic(uint16_t x, uint16_t y, unsigned char *color, uint16_t trans)
{  
	uint16_t h,w,show_w,show_h;
	uint16_t i;
	u8_t databuf[LCD_DATA_LEN]={0};
	u32_t offset=8,datelen,showlen=0,readlen=LCD_DATA_LEN;

	w=256*color[2]+color[3]; 			//��ȡͼƬ���
	h=256*color[4]+color[5];			//��ȡͼƬ�߶�

	if((x+w)>LCD_WIDTH)
		show_w = LCD_WIDTH-x;
	else
		show_w = w;

	if((y+h)>LCD_HEIGHT)
		show_h = LCD_HEIGHT-y;
	else
		show_h = h;

	BlockWrite(x,y,show_w,show_h);	//����ˢ��λ��
	datelen = 2*show_w*show_h;
	if(show_w < w)
		readlen = 2*show_w;

	while(datelen)
	{
		if(datelen < readlen)
		{
			readlen = datelen;
			datelen = 0;
		}
		else
		{
			readlen = readlen;
			datelen -= readlen;
		}
		
		memset(databuf, 0, LCD_DATA_LEN);
		memcpy(databuf, &color[offset], readlen);
		
		if(show_w < w)
			offset += 2*w;
		else
			offset += readlen;

		for(i=0;i<(readlen/2);i++)
		{
			if(trans == (256*databuf[2*i]+databuf[2*i+1]))
			{
				databuf[2*i] = BACK_COLOR>>8;
				databuf[2*i+1] = BACK_COLOR;
			}
		}
		
	#ifdef LCD_TYPE_SPI
		DispDate(readlen, databuf);
	#else
		for(i=0;i<(readlen/2);i++)
			WriteDispData(databuf[2*i],databuf[2*i+1]); //��ʾ��ɫ 
	#endif
	}
}


//ָ��λ����ת�Ƕ���ʾͼƬ
//color:ͼƬ����ָ��
//x:ͼƬ��ʾX����
//y:ͼƬ��ʾY����
//rotate:��ת�Ƕ�,0,90,180,270,
void LCD_dis_pic_rotate(uint16_t x, uint16_t y, unsigned char *color, unsigned int rotate)
{
	uint16_t h,w,show_w,show_h;
	uint32_t i,j=0;
	u8_t databuf[LCD_DATA_LEN]={0};
	u32_t offset=8,datelen,showlen=0,readlen=LCD_DATA_LEN;
	
	w=256*color[2]+color[3]; 			//��ȡͼƬ���
	h=256*color[4]+color[5];			//��ȡͼƬ�߶�

	switch(rotate)
	{
	case 0:
		if((x+w)>LCD_WIDTH)
			show_w = LCD_WIDTH-x;
		else
			show_w = w;
		
		if((y+h)>LCD_HEIGHT)
			show_h = LCD_HEIGHT-y;
		else
			show_h = h;

		BlockWrite(x,y,show_w,show_h);	//����ˢ��λ��
		datelen = 2*show_w*show_h;
		if(show_w < w)
			readlen = 2*show_w;
		
		while(datelen)
		{
			if(datelen < readlen)
			{
				readlen = datelen;
				datelen = 0;
			}
			else
			{
				readlen = readlen;
				datelen -= readlen;
			}
			
			memset(databuf, 0, LCD_DATA_LEN);
			memcpy(databuf, &color[offset], readlen);
			
			if(show_w < w)
				offset += 2*w;
			else
				offset += readlen;

		#ifdef LCD_TYPE_SPI
			DispDate(readlen, databuf);
		#else
			for(i=0;i<(readlen/2);i++)
				WriteDispData(databuf[2*i],databuf[2*i+1]);	//��ʾ��ɫ 
		#endif
		}		
		break;
		
	case 90:
		offset += 2*w*(h-1);
	
		if((x+h)>LCD_WIDTH)
			show_w = LCD_WIDTH-x;
		else
			show_w = h;
		
		if((y+w)>LCD_HEIGHT)
			show_h = LCD_HEIGHT-y;
		else
			show_h = w;

		BlockWrite(x,y,show_w,show_h);	//����ˢ��λ��
		datelen = 2*show_w*show_h;
		if(show_w < h)
			readlen = 2*show_w;
		
		while(datelen)
		{
			if(datelen < readlen)
			{
				readlen = datelen;
				datelen = 0;
			}
			else
			{
				readlen = readlen;
				datelen -= readlen;
			}
			
			memset(databuf, 0, LCD_DATA_LEN);
			for(i=0;i<(readlen/2);i++,j++)
			{
				databuf[2*i] = color[(offset+2*((j/h)-w*(j%h)))];
				databuf[2*i+1] = color[(offset+2*((j/h)-w*(j%h)))+1];
			}

			if(show_w < h)
			{
				offset += 2;
				j = 0;
			}
			
		#ifdef LCD_TYPE_SPI
			DispDate(readlen, databuf);
		#else
			for(i=0;i<(readlen/2);i++)
				WriteDispData(databuf[2*i],databuf[2*i+1]);	//��ʾ��ɫ 
		#endif
		}	
		break;
		
	case 180:
		offset += 2*(w*h-1);
		
		if((x+w)>LCD_WIDTH)
			show_w = LCD_WIDTH-x;
		else
			show_w = w;
		
		if((y+h)>LCD_HEIGHT)
			show_h = LCD_HEIGHT-y;
		else
			show_h = h;

		BlockWrite(x,y,show_w,show_h);	//����ˢ��λ��
		datelen = 2*show_w*show_h;
		if(show_w < w)
			readlen = 2*show_w;
		
		while(datelen)
		{
			if(datelen < readlen)
			{
				readlen = datelen;
				datelen = 0;
			}
			else
			{
				readlen = readlen;
				datelen -= readlen;
			}
			
			memset(databuf, 0, LCD_DATA_LEN);
			for(i=0;i<(readlen/2);i++,j++)
			{
				databuf[2*i] = color[(offset-2*j)];
				databuf[2*i+1] = color[(offset-2*j)+1];
			}

			if(show_w < w)
			{
				offset -= 2*w;
				j = 0;
			}
			
		#ifdef LCD_TYPE_SPI
			DispDate(readlen, databuf);
		#else
			for(i=0;i<(readlen/2);i++)
				WriteDispData(databuf[2*i],databuf[2*i+1]); //��ʾ��ɫ 
		#endif
		}
		break;
		
	case 270:
		offset += 2*(w-1);
			
		if((x+h)>LCD_WIDTH)
			show_w = LCD_WIDTH-x;
		else
			show_w = h;
		
		if((y+w)>LCD_HEIGHT)
			show_h = LCD_HEIGHT-y;
		else
			show_h = w;

		BlockWrite(x,y,show_w,show_h);	//����ˢ��λ��
		datelen = 2*show_w*show_h;
		if(show_w < h)
			readlen = 2*show_w;
		
		while(datelen)
		{
			if(datelen < readlen)
			{
				readlen = datelen;
				datelen = 0;
			}
			else
			{
				readlen = readlen;
				datelen -= readlen;
			}
			
			memset(databuf, 0, LCD_DATA_LEN);
			for(i=0;i<(readlen/2);i++,j++)
			{
				databuf[2*i] = color[(offset-2*(j/h)+2*w*(j%h))];
				databuf[2*i+1] = color[(offset-2*(j/h)+2*w*(j%h))+1];
			}

			if(show_w < w)
			{
				offset -= 2;
				j = 0;
			}
			
		#ifdef LCD_TYPE_SPI
			DispDate(readlen, databuf);
		#else
			for(i=0;i<(readlen/2);i++)
				WriteDispData(databuf[2*i],databuf[2*i+1]); //��ʾ��ɫ 
		#endif
		}
		break;
	}
}

//ָ��λ����ת�Ƕ���ʾͼƬ,����ɫ����
//color:ͼƬ����ָ��
//x:ͼƬ��ʾX����
//y:ͼƬ��ʾY����
//rotate:��ת�Ƕ�,0,90,180,270,
void LCD_dis_trans_pic_rotate(uint16_t x, uint16_t y, unsigned char *color, uint16_t trans, unsigned int rotate)
{
	uint16_t h,w,show_w,show_h;
	uint32_t i,j=0;
	u8_t databuf[LCD_DATA_LEN]={0};
	u32_t offset=8,datelen,showlen=0,readlen=LCD_DATA_LEN;
	
	w=256*color[2]+color[3]; 			//��ȡͼƬ���
	h=256*color[4]+color[5];			//��ȡͼƬ�߶�

	switch(rotate)
	{
	case 0:
		if((x+w)>LCD_WIDTH)
			show_w = LCD_WIDTH-x;
		else
			show_w = w;
		
		if((y+h)>LCD_HEIGHT)
			show_h = LCD_HEIGHT-y;
		else
			show_h = h;

		BlockWrite(x,y,show_w,show_h);	//����ˢ��λ��
		datelen = 2*show_w*show_h;
		if(show_w < w)
			readlen = 2*show_w;
		
		while(datelen)
		{
			if(datelen < readlen)
			{
				readlen = datelen;
				datelen = 0;
			}
			else
			{
				readlen = readlen;
				datelen -= readlen;
			}
			
			memset(databuf, 0, LCD_DATA_LEN);
			memcpy(databuf, &color[offset], readlen);
			
			if(show_w < w)
				offset += 2*w;
			else
				offset += readlen;

			for(i=0;i<(readlen/2);i++)
			{
				if(trans == (256*databuf[2*i]+databuf[2*i+1]))
				{
					databuf[2*i] = BACK_COLOR>>8;
					databuf[2*i+1] = BACK_COLOR;
				}
			}
			
		#ifdef LCD_TYPE_SPI
			DispDate(readlen, databuf);
		#else
			for(i=0;i<(readlen/2);i++)
				WriteDispData(databuf[2*i],databuf[2*i+1]);	//��ʾ��ɫ 
		#endif
		}		
		break;
		
	case 90:
		offset += 2*w*(h-1);
	
		if((x+h)>LCD_WIDTH)
			show_w = LCD_WIDTH-x;
		else
			show_w = h;
		
		if((y+w)>LCD_HEIGHT)
			show_h = LCD_HEIGHT-y;
		else
			show_h = w;

		BlockWrite(x,y,show_w,show_h);	//����ˢ��λ��
		datelen = 2*show_w*show_h;
		if(show_w < h)
			readlen = 2*show_w;
		
		while(datelen)
		{
			if(datelen < readlen)
			{
				readlen = datelen;
				datelen = 0;
			}
			else
			{
				readlen = readlen;
				datelen -= readlen;
			}
			
			memset(databuf, 0, LCD_DATA_LEN);
			for(i=0;i<(readlen/2);i++,j++)
			{
				databuf[2*i] = color[(offset+2*((j/h)-w*(j%h)))];
				databuf[2*i+1] = color[(offset+2*((j/h)-w*(j%h)))+1];
			}

			if(show_w < h)
			{
				offset += 2;
				j = 0;
			}

			for(i=0;i<(readlen/2);i++)
			{
				if(trans == (256*databuf[2*i]+databuf[2*i+1]))
				{
					databuf[2*i] = BACK_COLOR>>8;
					databuf[2*i+1] = BACK_COLOR;
				}
			}
					
		#ifdef LCD_TYPE_SPI
			DispDate(readlen, databuf);
		#else
			for(i=0;i<(readlen/2);i++)
				WriteDispData(databuf[2*i],databuf[2*i+1]);	//��ʾ��ɫ 
		#endif
		}
		break;
		
	case 180:
		offset += 2*(w*h-1);
		
		if((x+w)>LCD_WIDTH)
			show_w = LCD_WIDTH-x;
		else
			show_w = w;
		
		if((y+h)>LCD_HEIGHT)
			show_h = LCD_HEIGHT-y;
		else
			show_h = h;

		BlockWrite(x,y,show_w,show_h);	//����ˢ��λ��
		datelen = 2*show_w*show_h;
		if(show_w < w)
			readlen = 2*show_w;
		
		while(datelen)
		{
			if(datelen < readlen)
			{
				readlen = datelen;
				datelen = 0;
			}
			else
			{
				readlen = readlen;
				datelen -= readlen;
			}
			
			memset(databuf, 0, LCD_DATA_LEN);
			for(i=0;i<(readlen/2);i++,j++)
			{
				databuf[2*i] = color[(offset-2*j)];
				databuf[2*i+1] = color[(offset-2*j)+1];
			}

			if(show_w < w)
			{
				offset -= 2*w;
				j = 0;
			}

			for(i=0;i<(readlen/2);i++)
			{
				if(trans == (256*databuf[2*i]+databuf[2*i+1]))
				{
					databuf[2*i] = BACK_COLOR>>8;
					databuf[2*i+1] = BACK_COLOR;
				}
			}	
		#ifdef LCD_TYPE_SPI
			DispDate(readlen, databuf);
		#else
			for(i=0;i<(readlen/2);i++)
				WriteDispData(databuf[2*i],databuf[2*i+1]); //��ʾ��ɫ 
		#endif
		}		
		break;
		
	case 270:
		offset += 2*(w-1);
			
		if((x+h)>LCD_WIDTH)
			show_w = LCD_WIDTH-x;
		else
			show_w = h;
		
		if((y+w)>LCD_HEIGHT)
			show_h = LCD_HEIGHT-y;
		else
			show_h = w;

		BlockWrite(x,y,show_w,show_h);	//����ˢ��λ��
		datelen = 2*show_w*show_h;
		if(show_w < h)
			readlen = 2*show_w;
		
		while(datelen)
		{
			if(datelen < readlen)
			{
				readlen = datelen;
				datelen = 0;
			}
			else
			{
				readlen = readlen;
				datelen -= readlen;
			}
			
			memset(databuf, 0, LCD_DATA_LEN);
			for(i=0;i<(readlen/2);i++,j++)
			{
				databuf[2*i] = color[(offset-2*(j/h)+2*w*(j%h))];
				databuf[2*i+1] = color[(offset-2*(j/h)+2*w*(j%h))+1];
			}

			if(show_w < w)
			{
				offset -= 2;
				j = 0;
			}

			for(i=0;i<(readlen/2);i++)
			{
				if(trans == (256*databuf[2*i]+databuf[2*i+1]))
				{
					databuf[2*i] = BACK_COLOR>>8;
					databuf[2*i+1] = BACK_COLOR;
				}
			}
					
		#ifdef LCD_TYPE_SPI
			DispDate(readlen, databuf);
		#else
			for(i=0;i<(readlen/2);i++)
				WriteDispData(databuf[2*i],databuf[2*i+1]); //��ʾ��ɫ 
		#endif
		}		
		break;
	}
}

//��ָ��λ����ʾһ���ַ�
//x,y:��ʼ����
//num:Ҫ��ʾ���ַ�:" "--->"~"
//mode:���ӷ�ʽ(1)���Ƿǵ��ӷ�ʽ(0)
#ifndef IMG_FONT_FROM_FLASH
void LCD_ShowChar(uint16_t x,uint16_t y,uint8_t num,uint8_t mode)
{
    u8_t temp,t1,t,i=0;
	u16_t y0=y,x0=x;
	u8_t cbyte=(system_font/2)/8+(((system_font/2)%8)?1:0);		//��ɨ�裬ÿ���ַ�ÿһ��ռ�õ��ֽ���(Ӣ�Ŀ�����ֿ��һ��)
	u8_t csize=cbyte*system_font;		//�õ�����һ���ַ���Ӧ������ռ���ֽ���	
 	u8_t databuf[2*COL] = {0};
	
	num=num-' ';//�õ�ƫ�ƺ��ֵ��ASCII�ֿ��Ǵӿո�ʼȡģ������-' '���Ƕ�Ӧ�ַ����ֿ⣩
	for(t=0;t<csize;t++)
	{
		switch(system_font)
		{
		#ifdef FONT_16
			case FONT_SIZE_16:
				temp=asc2_1608[num][t]; 	 	//����1608����
				break;
		#endif
		#ifdef FONT_24
			case FONT_SIZE_24:
				temp=asc2_2412[num][t];			//����2412����
				break;
		#endif
		#ifdef FONT_32
			case FONT_SIZE_32:
				temp=asc2_3216[num][t];			//����3216����
				break;
		#endif
			default:
				return;							//û�е��ֿ�
		}

	#ifdef LCD_TYPE_SPI
		BlockWrite(x0,y,(system_font/2),1);	  	//����ˢ��λ��
	#endif
	
		for(t1=0;t1<8;t1++)
		{
		#ifdef LCD_TYPE_SPI
			if(temp&0x80)
			{
				databuf[2*i] = POINT_COLOR>>8;
				databuf[2*i+1] = POINT_COLOR;
			}
			else if(mode==0)
			{
				databuf[2*i] = BACK_COLOR>>8;
				databuf[2*i+1] = BACK_COLOR;
			}
			
			temp<<=1;
			i++;
			x++;
			if(x>=LCD_WIDTH)				//����������ֱ����ʾ��һ��
			{
				DispDate(2*i, databuf);
				i=0;

				x=x0;
				y++;
				if(y>=LCD_HEIGHT)return;	//��������
				t=t+(cbyte-(t%cbyte))-1;	//��ȡ��һ�ж�Ӧ���ֽڣ�ע��forѭ��������1��������������ǰ��ȥ1
				break;

			}
			if((x-x0)==(system_font/2))
			{
				DispDate(2*i, databuf);
				i=0;
				
				x=x0;
				y++;
				if(y>=LCD_HEIGHT)return;	//��������
				break;
			}
		#else
			if(temp&0x80)LCD_Fast_DrawPoint(x,y,POINT_COLOR);
			else if(mode==0)LCD_Fast_DrawPoint(x,y,BACK_COLOR);
			temp<<=1;
			x++;
			if(x>=LCD_WIDTH)				//����������ֱ����ʾ��һ��
			{
				x=x0;
				y++;
				if(y>=LCD_HEIGHT)return;	//��������
				t=t+(cbyte-(t%cbyte))-1;	//��ȡ��һ�ж�Ӧ���ֽڣ�ע��forѭ��������1��������������ǰ��ȥ1
				break;				
			}
			if((x-x0)==(system_font/2))
			{
				x=x0;
				y++;
				if(y>=LCD_HEIGHT)return; 	//��������
				break;
			}
		#endif
		}	
	}  	    	   	 	  
}

/*********************************************************************************************************************
* Name:LCD_Show_Ex_Char
* Function:��ʾfontmaker�������ɵ�bin��ʽ�ĵ����ֿ�
* Description:
* 	������: 
* 	��00000010h ��ʼ��ÿ 4 ���ֽڱ�ʾһ���ַ��ļ�����Ϣ�� �Ҵ��ַ� 0x0 ��ʼ���ʿո��ַ���' '������Ϊ 0x20���ļ�����Ϣ
* 	���ļ�ͷ�ĳ���+�ַ�����*4 = 0x10 + 0x20 *4 = 0x90�� �� 000000090h��Ϊ��10 04 00 10�����ó�һ�� 32 λ��Ϊ�� 
* 	0x10000410��ʮ�����ƣ� --- ��00010000 00000000 00000100 00010000��. 
* 	�� 6 λ����ʾ��ǰ�ַ��Ŀ�ȡ� �ʵó� 000100 -- 4 ���ֿ���Ϊ 4 ��
* 	�� 26 λ����ʾ�����ַ��ĵ������ݵ�ƫ�Ƶ�ַ���ʵó� 00 00000000 00000100 00010000 -- 0x410 ��������Ϣ����ʼ��ַΪ 0x410) 
* 
* 	�������� 
* 	���ڿո��ַ�����ʼ��ַΪ 0x410�������ݳ���Ϊ������������+7��/8��* ����߶� = ((4+7)/8)*16 = 16. 
* 	��ȡ���� 16 �ֽڣ���Ϊ�ո��ַ��ĵ������ݡ�
*********************************************************************************************************************/
u8_t LCD_Show_Ex_Char(uint16_t x,uint16_t y,uint8_t num,uint8_t mode)
{
	u8_t temp,t1,t,i=0,*ptr_font;
	u16_t y0=y,x0=x;
	u8_t cbyte=0;		//��ɨ�裬ÿ���ַ�ÿһ��ռ�õ��ֽ���(Ӣ�Ŀ�����ֿ��һ��)
	u8_t csize=0;		//�õ�����һ���ַ���Ӧ������ռ���ֽ���	
 	u8_t databuf[2*COL] = {0};
	u32_t index_addr,data_addr=0;

	switch(system_font)
	{
	#ifdef FONT_16
		case FONT_SIZE_16:
			ptr_font=asc2_16_rm; 	 		//����1608����
			break;
	#endif
	#ifdef FONT_24
		case FONT_SIZE_24:
			ptr_font=asc2_24_rm;			//����2412����
			break;
	#endif
	#ifdef FONT_32
		case FONT_SIZE_32:
			ptr_font=asc2_32_rm;			//����3216����
			break;
	#endif
		default:
			return 0;						//û�е��ֿ�
	}

	index_addr = FONT_MBCS_HEAD_LEN+4*num;
	data_addr = ptr_font[index_addr]+0x100*ptr_font[index_addr+1]+0x10000*ptr_font[index_addr+2];
	cbyte = ptr_font[index_addr+3]>>2;
	csize = ((cbyte+7)/8)*system_font;

	for(t=0;t<csize;t++)
	{
		temp=ptr_font[data_addr+t];

		BlockWrite(x0,y,(cbyte),1);	  	//����ˢ��λ��

		for(t1=0;t1<8;t1++)
		{
		#ifdef LCD_TYPE_SPI
			if(temp&0x80)
			{
				databuf[2*i] = POINT_COLOR>>8;
				databuf[2*i+1] = POINT_COLOR;
			}
			else if(mode==0)
			{
				databuf[2*i] = BACK_COLOR>>8;
				databuf[2*i+1] = BACK_COLOR;
			}
			
			temp<<=1;
			i++;
			x++;
			if(x>=LCD_WIDTH)				//����������ֱ����ʾ��һ��
			{
				DispDate(2*i, databuf);
				i=0;

				x=x0;
				y++;
				if(y>=LCD_HEIGHT)return cbyte;	//��������
				t=t+(cbyte-(t%cbyte))-1;	//��ȡ��һ�ж�Ӧ���ֽڣ�ע��forѭ��������1��������������ǰ��ȥ1
				break;

			}
			if((x-x0)==cbyte)
			{
				DispDate(2*i, databuf);
				i=0;
				
				x=x0;
				y++;
				if(y>=LCD_HEIGHT)return cbyte;	//��������
				break;
			}
		#else
			if(temp&0x80)LCD_Fast_DrawPoint(x,y,POINT_COLOR);
			else if(mode==0)LCD_Fast_DrawPoint(x,y,BACK_COLOR);
			temp<<=1;
			x++;
			if(x>=LCD_WIDTH)				//����������ֱ����ʾ��һ��
			{
				x=x0;
				y++;
				if(y>=LCD_HEIGHT)return;	//��������
				t=t+(cbyte-(t%cbyte))-1;	//��ȡ��һ�ж�Ӧ���ֽڣ�ע��forѭ��������1��������������ǰ��ȥ1
				break;				
			}
			if((x-x0)==cbyte)
			{
				x=x0;
				y++;
				if(y>=LCD_HEIGHT)return;	//��������
				break;
			}
		#endif
		}	
	}  	 

	return cbyte;
}
#endif

//��ָ��λ����ʾflash��һ���ַ�
//x,y:��ʼ����
//num:Ҫ��ʾ���ַ�:" "--->"~"
//mode:���ӷ�ʽ(1)���Ƿǵ��ӷ�ʽ(0)
void LCD_ShowChar_from_flash(uint16_t x,uint16_t y,uint8_t num,uint8_t mode)
{
	u8_t temp,t1,t;
	u8_t cbyte=(system_font/2)/8+(((system_font/2)%8)?1:0);		//��ɨ�裬ÿ���ַ�ÿһ��ռ�õ��ֽ���(Ӣ�Ŀ�����ֿ��һ��)
	u8_t csize=cbyte*system_font;		//�õ�����һ���ַ���Ӧ������ռ���ֽ���	
 	u8_t databuf[2*1024] = {0};
	u8_t fontbuf[128] = {0};
	u16_t y0=y,x0=x;
	u32_t i=0;
	
	num=num-' ';//�õ�ƫ�ƺ��ֵ��ASCII�ֿ��Ǵӿո�ʼȡģ������-' '���Ƕ�Ӧ�ַ����ֿ⣩
	switch(system_font)
	{
	#ifdef FONT_16
		case FONT_SIZE_16:
			SpiFlash_Read(fontbuf, FONT_ASC_1608_ADDR+csize*num, csize);
			break;
	#endif
	#ifdef FONT_24
		case FONT_SIZE_24:
			SpiFlash_Read(fontbuf, FONT_ASC_2412_ADDR+csize*num, csize);
			break;
	#endif
	#ifdef FONT_32
		case FONT_SIZE_32:
			SpiFlash_Read(fontbuf, FONT_ASC_3216_ADDR+csize*num, csize);
			break;
	#endif
		default:
			return;							//û�е��ֿ�
	}

#ifdef LCD_TYPE_SPI
	BlockWrite(x,y,(system_font/2),system_font); 	//����ˢ��λ��
#endif

	for(t=0;t<csize;t++)
	{		
		temp = fontbuf[t];
		for(t1=0;t1<8;t1++)
		{
		#ifdef LCD_TYPE_SPI
			if(temp&0x80)
			{
				databuf[2*i] = POINT_COLOR>>8;
				databuf[2*i+1] = POINT_COLOR;
			}
			else if(mode==0)
			{
				databuf[2*i] = BACK_COLOR>>8;
				databuf[2*i+1] = BACK_COLOR;
			}
			
			temp<<=1;
			i++;
			x++;
			if(x>=LCD_WIDTH)				//����������ֱ����ʾ��һ��
			{
				x=x0;
				y++;
				if(y>=LCD_HEIGHT)
				{
					DispDate(2*i, databuf);
					return;	//��������
				}
				
				t=t+(cbyte-(t%cbyte))-1;	//��ȡ��һ�ж�Ӧ���ֽڣ�ע��forѭ��������1��������������ǰ��ȥ1
				break;

			}
			if((x-x0)==(system_font/2))
			{
				x=x0;
				y++;
				if(y>=LCD_HEIGHT)
				{
					DispDate(2*i, databuf);
					return;	//��������
				}
				
				break;
			}
		#else
			if(temp&0x80)LCD_Fast_DrawPoint(x,y,POINT_COLOR);
			else if(mode==0)LCD_Fast_DrawPoint(x,y,BACK_COLOR);
			temp<<=1;
			x++;
			if(x>=LCD_WIDTH)				//����������ֱ����ʾ��һ��
			{
				x=x0;
				y++;
				if(y>=LCD_HEIGHT)return;	//��������
				t=t+(cbyte-(t%cbyte))-1;	//��ȡ��һ�ж�Ӧ���ֽڣ�ע��forѭ��������1��������������ǰ��ȥ1
				break;				
			}
			if((x-x0)==(system_font/2))
			{
				x=x0;
				y++;
				if(y>=LCD_HEIGHT)return; 	//��������
				break;
			}
		#endif
		}
	}

#ifdef LCD_TYPE_SPI
	DispDate(2*i, databuf);
#endif
}   

/*********************************************************************************************************************
* Name:LCD_Show_Ex_Char
* Function:��ʾfontmaker�������ɵ�bin��ʽ�ĵ����ֿ�
* Description:
* 	������: 
* 	��00000010h ��ʼ��ÿ 4 ���ֽڱ�ʾһ���ַ��ļ�����Ϣ�� �Ҵ��ַ� 0x0 ��ʼ���ʿո��ַ���' '������Ϊ 0x20���ļ�����Ϣ
* 	���ļ�ͷ�ĳ���+�ַ�����*4 = 0x10 + 0x20 *4 = 0x90�� �� 000000090h��Ϊ��10 04 00 10�����ó�һ�� 32 λ��Ϊ�� 
* 	0x10000410��ʮ�����ƣ� --- ��00010000 00000000 00000100 00010000��. 
* 	�� 6 λ����ʾ��ǰ�ַ��Ŀ�ȡ� �ʵó� 000100 -- 4 ���ֿ���Ϊ 4 ��
* 	�� 26 λ����ʾ�����ַ��ĵ������ݵ�ƫ�Ƶ�ַ���ʵó� 00 00000000 00000100 00010000 -- 0x410 ��������Ϣ����ʼ��ַΪ 0x410) 
* 
* 	�������� 
* 	���ڿո��ַ�����ʼ��ַΪ 0x410�������ݳ���Ϊ������������+7��/8��* ����߶� = ((4+7)/8)*16 = 16. 
* 	��ȡ���� 16 �ֽڣ���Ϊ�ո��ַ��ĵ������ݡ�
*********************************************************************************************************************/
u8_t LCD_Show_Ex_Char_from_flash(uint16_t x,uint16_t y,uint8_t num,uint8_t mode)
{
	u8_t temp,t1,t;
	u16_t y0=y,x0=x;
	u8_t cbyte=0;		//��ɨ�裬ÿ���ַ�ÿһ��ռ�õ��ֽ���(Ӣ�Ŀ�����ֿ��һ��)
	u8_t csize=0;		//�õ�����һ���ַ���Ӧ������ռ���ֽ���	
 	u8_t databuf[2*1024] = {0};
	u8_t fontbuf[128] = {0};
	u32_t i=0,index_addr,font_addr,data_addr=0;

	switch(system_font)
	{
	#ifdef FONT_16
		case FONT_SIZE_16:
			font_addr = FONT_RM_ASC_16_ADDR;
			break;
	#endif
	#ifdef FONT_24
		case FONT_SIZE_24:
			font_addr = FONT_RM_ASC_24_ADDR;
			break;
	#endif
	#ifdef FONT_32
		case FONT_SIZE_32:
			font_addr = FONT_RM_ASC_32_ADDR;
			break;
	#endif
		default:
			return; 						//û�е��ֿ�
	}

	index_addr = FONT_MBCS_HEAD_LEN+4*num;
	SpiFlash_Read(fontbuf, font_addr+index_addr, 4);
	data_addr = fontbuf[0]+0x100*fontbuf[1]+0x10000*fontbuf[2];
	cbyte = fontbuf[3]>>2;
	csize = ((cbyte+7)/8)*system_font;	
	SpiFlash_Read(fontbuf, font_addr+data_addr, csize);
	
#ifdef LCD_TYPE_SPI
	BlockWrite(x,y,cbyte,system_font);	//����ˢ��λ��
#endif

	for(t=0;t<csize;t++)
	{		
		temp = fontbuf[t];
		for(t1=0;t1<8;t1++)
		{
		#ifdef LCD_TYPE_SPI
			if(temp&0x80)
			{
				databuf[2*i] = POINT_COLOR>>8;
				databuf[2*i+1] = POINT_COLOR;
			}
			else if(mode==0)
			{
				databuf[2*i] = BACK_COLOR>>8;
				databuf[2*i+1] = BACK_COLOR;
			}
			
			temp<<=1;
			i++;
			x++;
			if(x>=LCD_WIDTH)				//����������ֱ����ʾ��һ��
			{
				x=x0;
				y++;
				if(y>=LCD_HEIGHT)
				{
					DispDate(2*i, databuf);
					return; //��������
				}
				
				t=t+(cbyte-(t%cbyte))-1;	//��ȡ��һ�ж�Ӧ���ֽڣ�ע��forѭ��������1��������������ǰ��ȥ1
				break;

			}
			if((x-x0)==cbyte)
			{
				x=x0;
				y++;
				if(y>=LCD_HEIGHT)
				{
					DispDate(2*i, databuf);
					return; //��������
				}
				
				break;
			}
		#else
			if(temp&0x80)LCD_Fast_DrawPoint(x,y,POINT_COLOR);
			else if(mode==0)LCD_Fast_DrawPoint(x,y,BACK_COLOR);
			temp<<=1;
			x++;
			if(x>=LCD_WIDTH)				//����������ֱ����ʾ��һ��
			{
				x=x0;
				y++;
				if(y>=LCD_HEIGHT)return;	//��������
				t=t+(cbyte-(t%cbyte))-1;	//��ȡ��һ�ж�Ӧ���ֽڣ�ע��forѭ��������1��������������ǰ��ȥ1
				break;				
			}
			if((x-x0)==cbyte)
			{
				x=x0;
				y++;
				if(y>=LCD_HEIGHT)return;	//��������
				break;
			}
		#endif
		}
	}

#ifdef LCD_TYPE_SPI
	DispDate(2*i, databuf);
#endif

	return cbyte;
}


//��ָ��λ����ʾһ�������ַ�
//x,y:��ʼ����
//num:Ҫ��ʾ���ַ�:" "--->"~"
//mode:���ӷ�ʽ(1)���Ƿǵ��ӷ�ʽ(0)
#ifndef IMG_FONT_FROM_FLASH
void LCD_ShowChineseChar(uint16_t x,uint16_t y,uint16_t num,uint8_t mode)
{  							  
	u8_t temp,t1,t,i=0;
	u16_t x0=x,y0=y;
	u16_t index=0;
	u8_t cbyte=system_font/8+((system_font%8)?1:0);		//��ɨ�裬ÿ���ַ�ÿһ��ռ�õ��ֽ���
	u8_t csize=cbyte*(system_font);		//�õ�����һ���ַ���Ӧ������ռ���ֽ���	
	u8_t databuf[2*COL] = {0};

	index=94*((num>>8)-0xa0-1)+1*((num&0x00ff)-0xa0-1);			//offset = (94*(����-1)+(λ��-1))*32
	for(t=0;t<csize;t++)
	{	
		switch(system_font)
		{
		#if 0	//def FONT_16
			case FONT_SIZE_16:
				temp=chinese_1616[index][t]; 	 	//����1616����
				break;
		#endif
		#if 0	//def FONT_24
			case FONT_SIZE_24:
				temp=chinese_2424[index][t];		//����2424����
				break;
		#endif
		#if 0	//def FONT_32
			case FONT_SIZE_32:
				temp=chinese_3232[index][t];		//����3232����
				break;
		#endif
			default:
				return;								//û�е��ֿ�
		}	

	#ifdef LCD_TYPE_SPI
		BlockWrite(x0,y,system_font,1);	  	//����ˢ��λ��
	#endif
	
		for(t1=0;t1<8;t1++)
		{
		#ifdef LCD_TYPE_SPI
			if(temp&0x80)
			{
				databuf[2*i] = POINT_COLOR>>8;
				databuf[2*i+1] = POINT_COLOR;
			}
			else if(mode==0)
			{
				databuf[2*i] = BACK_COLOR>>8;
				databuf[2*i+1] = BACK_COLOR;
			}
			
			temp<<=1;
			i++;
			x++;
			if(x>=LCD_WIDTH)				//����������ֱ����ʾ��һ��
			{
				DispDate(2*i, databuf);
				i=0;

				x=x0;
				y++;
				if(y>=LCD_HEIGHT)return;	//��������
				t=t+(cbyte-(t%cbyte))-1;	//��ȡ��һ�ж�Ӧ���ֽڣ�ע��forѭ��������1��������������ǰ��ȥ1
				break;
			}
			if((x-x0)==(system_font))
			{
				DispDate(2*i, databuf);
				i=0;
				
				x=x0;
				y++;
				if(y>=LCD_HEIGHT)return;	//��������
				break;
			}			
		#else
			if(temp&0x80)LCD_Fast_DrawPoint(x,y,POINT_COLOR);
			else if(mode==0)LCD_Fast_DrawPoint(x,y,BACK_COLOR);
			temp<<=1;
			x++;
			if(x>=LCD_WIDTH)				//����������ֱ����ʾ��һ��
			{
				x=x0;
				y++;
				if(y>=LCD_HEIGHT)return;	//��������
				t=t+(cbyte-(t%cbyte))-1;	//��ȡ��һ�ж�Ӧ���ֽڣ�ע��forѭ��������1��������������ǰ��ȥ1
				break;			
			}
			if((x-x0)==system_font)
			{
				x=x0;
				y++;
				if(y>=LCD_HEIGHT)return;	//��������
				break;
			}
		#endif
		} 
	}  	    	   	 	  
}   
#endif

//��ָ��λ����ʾflash��һ�������ַ�
//x,y:��ʼ����
//num:Ҫ��ʾ���ַ�:" "--->"~"
//mode:���ӷ�ʽ(1)���Ƿǵ��ӷ�ʽ(0)
void LCD_ShowChineseChar_from_flash(uint16_t x,uint16_t y,uint16_t num,uint8_t mode)
{  							  
	u8_t temp,t1,t;
	u16_t x0=x,y0=y;
	u16_t index=0;
	u8_t cbyte=system_font/8+((system_font%8)?1:0);		//��ɨ�裬ÿ���ַ�ÿһ��ռ�õ��ֽ���
	u8_t csize=cbyte*(system_font);		//�õ�����һ���ַ���Ӧ������ռ���ֽ���	
	u8_t databuf[2*1024] = {0};
	u8_t fontbuf[128] = {0};
	u32_t i=0;
	
	index=94*((num>>8)-0xa0-1)+1*((num&0x00ff)-0xa0-1);			//offset = (94*(����-1)+(λ��-1))*32
	switch(system_font)
	{
	#ifdef FONT_16
		case FONT_SIZE_16:
			SpiFlash_Read(fontbuf, FONT_CHN_SM_1616_ADDR+csize*index+t, csize);
			break;
	#endif
	#ifdef FONT_24
		case FONT_SIZE_24:
			SpiFlash_Read(fontbuf, FONT_CHN_SM_2424_ADDR+csize*index+t, csize);
			break;
	#endif
	#ifdef FONT_32
		case FONT_SIZE_32:
			SpiFlash_Read(fontbuf, FONT_CHN_SM_3232_ADDR+csize*index+t, csize);
			break;
	#endif
		default:
			return;								//û�е��ֿ�
	}	

#ifdef LCD_TYPE_SPI
	BlockWrite(x0,y,system_font,system_font); 	//����ˢ��λ��
#endif

	for(t=0;t<csize;t++)
	{
		temp = fontbuf[t];
		for(t1=0;t1<8;t1++)
		{
		#ifdef LCD_TYPE_SPI
			if(temp&0x80)
			{
				databuf[2*i] = POINT_COLOR>>8;
				databuf[2*i+1] = POINT_COLOR;
			}
			else if(mode==0)
			{
				databuf[2*i] = BACK_COLOR>>8;
				databuf[2*i+1] = BACK_COLOR;
			}
			
			temp<<=1;
			i++;
			x++;
			if(x>=LCD_WIDTH)				//����������ֱ����ʾ��һ��
			{
				x=x0;
				y++;
				if(y>=LCD_HEIGHT)
				{
					DispDate(2*i, databuf);
					return;	//��������
				}
				
				t=t+(cbyte-(t%cbyte))-1;	//��ȡ��һ�ж�Ӧ���ֽڣ�ע��forѭ��������1��������������ǰ��ȥ1
				break;
			}
			if((x-x0)==(system_font))
			{
				x=x0;
				y++;
				if(y>=LCD_HEIGHT)
				{
					DispDate(2*i, databuf);
					return;	//��������
				}
				
				break;
			}			
		#else
			if(temp&0x80)LCD_Fast_DrawPoint(x,y,POINT_COLOR);
			else if(mode==0)LCD_Fast_DrawPoint(x,y,BACK_COLOR);
			temp<<=1;
			x++;
			if(x>=LCD_WIDTH)				//����������ֱ����ʾ��һ��
			{
				x=x0;
				y++;
				if(y>=LCD_HEIGHT)return;	//��������
				t=t+(cbyte-(t%cbyte))-1;	//��ȡ��һ�ж�Ӧ���ֽڣ�ע��forѭ��������1��������������ǰ��ȥ1
				break;			
			}
			if((x-x0)==system_font)
			{
				x=x0;
				y++;
				if(y>=LCD_HEIGHT)return;	//��������
				break;
			}
		#endif
		} 
	} 

#ifdef LCD_TYPE_SPI
	DispDate(2*i, databuf);
#endif	
}   

//��ָ��������������ʾ��Ӣ���ַ���
//x,y:�������
//width,height:�����С  
//*p:�ַ�����ʼ��ַ	
void LCD_ShowStringInRect(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint8_t *p)
{
	uint8_t x0=x;
	uint16_t phz=0;
	
	width+=x;
	height+=y;
    while(*p)
    {       
        if(x>=width){x=x0;y+=system_font;}
        if(y>=height)break;//�˳�
		if(*p<0x80)
		{
		#ifdef IMG_FONT_FROM_FLASH
			LCD_ShowChar_from_flash(x,y,*p,0);
		#else
			LCD_ShowChar(x,y,*p,0);
		#endif
			x+=system_font/2;
			p++;
		}
		else if(*(p+1))
        {
			phz = *p<<8;
			phz += *(p+1);
		#ifdef IMG_FONT_FROM_FLASH
			LCD_ShowChineseChar_from_flash(x,y,phz,0);
		#else
			LCD_ShowChineseChar(x,y,phz,0);
		#endif
			x+=system_font;
			p+=2;
		}        
    }
}

//��ʾ��Ӣ���ַ���
//x,y:�������
//*p:�ַ�����ʼ��ַ	
void LCD_ShowString(uint16_t x,uint16_t y,uint8_t *p)
{
	uint8_t x0=x;
	uint8_t width;
	uint16_t phz=0;

	while(*p)
	{       
		if(x>=LCD_WIDTH)break;//�˳�
		if(y>=LCD_HEIGHT)break;//�˳�
		if(*p<0x80)
		{
		#ifdef IMG_FONT_FROM_FLASH
		  #ifdef FONTMAKER_FONT
			width = LCD_Show_Ex_Char_from_flash(x,y,*p,0);
		  	x += width;
		  #else
			LCD_ShowChar_from_flash(x,y,*p,0);
		  	x += system_font/2;
		  #endif
		#else
		  #ifdef FONTMAKER_FONT
			width = LCD_Show_Ex_Char(x,y,*p,0);
		  	x += width;
		  #else
			LCD_ShowChar(x,y,*p,0);
		  	x += system_font/2;
		  #endif
		#endif
			
			p++;
		}
		else if(*(p+1))
		{
			phz = *p<<8;
			phz += *(p+1);
		#ifdef IMG_FONT_FROM_FLASH	
			LCD_ShowChineseChar_from_flash(x,y,phz,0);
		#else
			LCD_ShowChineseChar(x,y,phz,0);
		#endif
			x+=system_font;
			p+=2;
		}        
	}
}

//m^n����
//����ֵ:m^n�η�.
uint32_t LCD_Pow(uint8_t m,uint8_t n)
{
	uint32_t result=1;	 
	while(n--)result*=m;    
	return result;
}

//��ʾ����,��λΪ0,����ʾ
//x,y :�������	 
//len :���ֵ�λ��
//color:��ɫ 
//num:��ֵ(0~4294967295);	 
void LCD_ShowNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len)
{         	
	uint8_t t,temp;
	uint8_t enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/LCD_Pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
			#ifdef IMG_FONT_FROM_FLASH
				LCD_ShowChar_from_flash(x+(system_font/2)*t,y,' ',0);
			#else
				LCD_ShowChar(x+(system_font/2)*t,y,' ',0);
			#endif
				continue;
			}
			else 
				enshow=1; 
		 	 
		}
	#ifdef IMG_FONT_FROM_FLASH
		LCD_ShowChar_from_flash(x+(system_font/2)*t,y,temp+'0',0);
	#else
	 	LCD_ShowChar(x+(system_font/2)*t,y,temp+'0',0); 
	#endif
	}
}

//��ʾ����,��λΪ0,������ʾ
//x,y:�������
//num:��ֵ(0~999999999);	 
//len:����(��Ҫ��ʾ��λ��)
//mode:
//[7]:0,�����;1,���0.
//[6:1]:����
//[0]:0,�ǵ�����ʾ;1,������ʾ.
void LCD_ShowxNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t mode)
{  
	uint8_t t,temp;
	uint8_t enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/LCD_Pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				if(mode&0X80)
				{
				#ifdef IMG_FONT_FROM_FLASH
					LCD_ShowChar_from_flash(x+(system_font/2)*t,y,'0',mode&0X01);
				#else
					LCD_ShowChar(x+(system_font/2)*t,y,'0',mode&0X01);
				#endif
				}
				else 
				{
				#ifdef IMG_FONT_FROM_FLASH
					LCD_ShowChar_from_flash(x+(system_font/2)*t,y,' ',mode&0X01);
				#else
					LCD_ShowChar(x+(system_font/2)*t,y,' ',mode&0X01); 
				#endif
				}
				
 				continue;
			}
			else 
				enshow=1; 
		}
	#ifdef IMG_FONT_FROM_FLASH
		LCD_ShowChar_from_flash(x+(system_font/2)*t,y,temp+'0',mode&0X01);
	#else
	 	LCD_ShowChar(x+(system_font/2)*t,y,temp+'0',mode&0X01);
	#endif
	}
} 

//������������ַ��Ŀ��
//p:�ַ�ָ��
//width,height:���ص��ַ�����Ⱥ͸߶ȱ�����ַ
u8_t LCD_MeasureByte(u8_t byte)
{
	u8_t width, *ptr_font;
	u8_t fontbuf[4] = {0};	
	u32_t index_addr,data_addr,font_addr=0;

#ifdef IMG_FONT_FROM_FLASH
	switch(system_font)
	{
	#ifdef FONT_16
		case FONT_SIZE_16:
			font_addr = FONT_RM_ASC_16_ADDR;
			break;
	#endif
	#ifdef FONT_24
		case FONT_SIZE_24:
			font_addr = FONT_RM_ASC_24_ADDR;
			break;
	#endif
	#ifdef FONT_32
		case FONT_SIZE_32:
			font_addr = FONT_RM_ASC_32_ADDR;
			break;
	#endif
		default:
			return;
	}

	index_addr = FONT_MBCS_HEAD_LEN+4*byte;
	SpiFlash_Read(fontbuf, font_addr+index_addr, 4);
	data_addr = fontbuf[0]+0x100*fontbuf[1]+0x10000*fontbuf[2];
	width = fontbuf[3]>>2;
#else
	switch(system_font)
	{
	#ifdef FONT_16
		case FONT_SIZE_16:
			ptr_font=asc2_16_rm; 	 	//����1608����
			break;
	#endif
	#ifdef FONT_24
		case FONT_SIZE_24:
			ptr_font=asc2_24_rm;			//����2412����
			break;
	#endif
	#ifdef FONT_32
		case FONT_SIZE_32:
			ptr_font=asc2_32_rm;			//����3216����
			break;
	#endif
		default:
			return 0;							//û�е��ֿ�
	}
	
	index_addr = FONT_MBCS_HEAD_LEN+4*byte;
	data_addr = ptr_font[index_addr]+0x100*ptr_font[index_addr+1]+0x10000*ptr_font[index_addr+2];
	width = ptr_font[index_addr+3]>>2;
#endif

	return width;
}

//������������ַ����ĳ��Ⱥ͸߶�
//p:�ַ���ָ��
//width,height:���ص��ַ�����Ⱥ͸߶ȱ�����ַ
void LCD_MeasureString(uint8_t *p, uint16_t *width,uint16_t *height)
{
	uint8_t font_size;
	
	*width = 0;
	*height = 0;
	
	if(p == NULL || strlen((const char *)p) == 0)
		return;
	
	(*height) = system_font;
	
	while(*p)
    {       
		if(*p<0x80)
		{
		#ifdef FONTMAKER_FONT
			(*width) += LCD_MeasureByte(*p);
		#else
			(*width) += system_font/2;
		#endif
			p++;
		}
		else if(*(p+1))
        {
			(*width) += system_font;
			p += 2;
		}        
    }  
}

//����ϵͳ����
//font_size:ö�������С
void LCD_SetFontSize(uint8_t font_size)
{
	if(font_size > FONT_SIZE_MIN && font_size < FONT_SIZE_MAX)
		system_font = font_size;
}
