#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

uint16_t light_sleep_time = 0;
uint16_t deep_sleep_time = 0;
int waggle_level[12] = {0};
int hour_time = 0;

extern bool clear_data;

void set_sleep_parameter(int light_sleep, int deep_sleep,int *waggle) /* �ش�˯�߲���*/
{
	/* ÿ�����ϵͳnvram �����洢���ݣ���ֹϵͳ�쳣����ʱ˯�����ݶ�ʧ*/
	deep_sleep_time = deep_sleep;  
	light_sleep_time = light_sleep;
	memset(waggle_level, 0, sizeof(waggle_level));
	strncpy(waggle_level, waggle,10);
}

void Set_Gsensor_data(signed short x, signed short y, signed short z, int setp,int hr,int hour,int charging)
{	
  int test = 0 , i = 0;
  static int move = 0;
  static int rtc_sec = 0;
  static int gsensor = 0;
  static int move_flag = 0;
  static uint16_t watch_state = 0;
  static int waggle_flag = 0;
  static int sedentary_time_temp = 0;

    if(charging){ /* ����в�ִ��*/
	return;
    }
		
    test = abs(x+y+z);
    hour_time = hour;

    if((abs(gsensor-test)) >= 100)  /* �ж��ֱ��Ƿ�ζ�����ֵ����gsensor �����ȵ���*/
    {
      watch_state = 0;
    }
    else
    {
      watch_state ++;
    }
	
    if(((abs(gsensor-test)) >= 200)||(hr != 0))
    {
      if(hour < 6)
      waggle_level[hour] ++;  /* ͳ���ֱ�0�㵽6�����״̬����ֵ����gsensor �����ȵ�����0Ϊδ�������0ֵΪ���*/
    }
	
    gsensor = test;
	
    if((rtc_sec % 60) == 0)  /* һ��������һ������*/
    {
		if(((move <setp)&&(move>0))||(move_flag > 0))  /*  move_flag  �ƶ������һ�ξ�ֹĬ��Ϊ�ƶ�*/
		{
			if((move_flag == 0)||((move <setp)&&(move>0)))
			move_flag = 1;
			else if(move_flag > 0)
			move_flag --;
			if(hour<8)
			sedentary_time_temp ++;
		}	
                                
		else if((watch_state <= 60)&&(move_flag == 0))
		{
			if(hour<8)
			sedentary_time_temp ++;   /*  ˯�߼��ʱ��Σ��ۼ��߶�ʱ��*/
			else
			sedentary_time_temp = 0;
		}
		
		if((hour>=20)||(hour<8))  /*����ʱ���� 24Сʱ�� �����ʱ�������8�㵽����8�� */
		{
			if((hour >= 6)&&(hour <= 8)&&(waggle_flag == 0))
			{
				for(i = 0; i < 6; i++)	
				{
				   if(waggle_level[i]==0)
				   waggle_flag ++; /* ��������0�㵽6�㼸��Сʱ�����״̬ */
				}
			}
			
			if((watch_state >= (3600*2))||(waggle_flag >= 3)||/* (3)����2��3ʱ�ն�û�ж����ж�Ϊ����˯��û����������ۼ�˯�ߡ������������*/
			((hour<8)&&(sedentary_time_temp > 180)))  /* ����˯�߼��ʱ���ۼ�150�������߶����߾�����Ĭ��˯�߼������Ϊ0*/
			{
				sedentary_time_temp = 0;
				light_sleep_time = 0;				 
				deep_sleep_time = 0;
                                clear_data = true;
			}
			else if((move == setp)&&(watch_state >= 60)) /* ���ڼ�û���߶��� */
			{
				if(watch_state >= (60*10))/* ����15��������δ�ζ������ж�Ϊ���˯��*/
				deep_sleep_time ++; 
				else
				light_sleep_time ++; 
			}
			
		}
		else
		{
			move = 0;	 
			gsensor = 0;
			rtc_sec = 0;
			move_flag = 0;
			watch_state = 0;
			waggle_flag = 0;
			waggle_flag = 0;
			deep_sleep_time = 0; 
			light_sleep_time = 0;
			sedentary_time_temp = 0;			memset(waggle_level,0,sizeof(waggle_level)); /* �ζ��ȼ� */
		}
    }

    rtc_sec ++;
    move = setp;
}

int get_light_sleep_time(void) /* ǳ˯ʱ��*/
{
  return light_sleep_time*1.1;
}

int get_deep_sleep_time(void) /* ��˯ʱ��*/
{
  return deep_sleep_time*1.1;
}

