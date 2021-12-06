#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "lsm6dso.h"
#include "datetime.h"
#include "max20353.h"
#include "logger.h"

#define SLEEP_TIME_START	20
#define SLEEP_TIME_END		8

u16_t light_sleep_offset = 0;
u16_t deep_sleep_offset = 0;
u16_t light_sleep_time = 0;
u16_t deep_sleep_time = 0;
u16_t g_light_sleep = 0;
u16_t g_deep_sleep = 0;
int waggle_level[12] = {0};
int hour_time = 0;

bool update_sleep_parameter = false;

static struct k_timer sleep_timer;

void set_sleep_parameter(int light_sleep, int deep_sleep,int *waggle) /* �ش�˯�߲���*/
{
	/* ÿ�����ϵͳnvram �����洢���ݣ���ֹϵͳ�쳣����ʱ˯�����ݶ�ʧ*/
	deep_sleep_time = deep_sleep;  
	light_sleep_time = light_sleep;
	memset(waggle_level, 0, sizeof(waggle_level));
	strncpy(waggle_level, waggle,10);
}

void Set_Gsensor_data(signed short x, signed short y, signed short z, int step, int hr, int hour, int charging)
{	
	int test=0,i=0;
	static int move = 0;
	static int rtc_sec = 0;
	static int gsensor = 0;
	static int move_flag = 0;
	static uint16_t watch_state = 0;
	static int waggle_flag = 0;
	static int sedentary_time_temp = 0;

	if(charging)
	{
		/* ����в�ִ��*/
		return;
	}

	test = abs(x+y+z);
	hour_time = hour;

	if((abs(gsensor-test)) >= 120)  /* �ж��ֱ��Ƿ�ζ�����ֵ����gsensor �����ȵ���*/
	{
		watch_state = 0;
	}
	else
	{
		watch_state++;
	}

	if(((abs(gsensor-test)) >= 200)||(hr != 0))
	{
		if(hour < 6)
			waggle_level[hour]++;  /* ͳ���ֱ�0�㵽6�����״̬����ֵ����gsensor �����ȵ�����0Ϊδ�������0ֵΪ���*/
	}

	gsensor = test;

	if((rtc_sec % 60) == 0)  /* һ��������һ������*/
	{
		if(((move <step)&&(move>0))||(move_flag > 0))  /*  move_flag  �ƶ������һ�ξ�ֹĬ��Ϊ�ƶ�*/
		{
			if((move_flag == 0)||((move <step)&&(move>0)))
				move_flag = 1;
			else if(move_flag > 0)
				move_flag--;

			if(hour<SLEEP_TIME_END)
				sedentary_time_temp++;
		}	
		                    
		else if((watch_state <= 60)&&(move_flag == 0))
		{
			if(hour<SLEEP_TIME_END)
				sedentary_time_temp++;   /*  ˯�߼��ʱ��Σ��ۼ��߶�ʱ��*/
			else
				sedentary_time_temp = 0;
		}

		if((hour>=SLEEP_TIME_START)||(hour<SLEEP_TIME_END))  /*����ʱ���� 24Сʱ�� �����ʱ�������8�㵽����8�� */
		{
			if((hour>=6)&&(hour<=SLEEP_TIME_END)&&(waggle_flag==0))
			{
				for(i=0;i<6;i++)	
				{
					if(waggle_level[i]==0)
						waggle_flag++; /* ��������0�㵽6�㼸��Сʱ�����״̬ */
				}
			}

			if((watch_state >= (3600*2))||(waggle_flag >= 7)||/* (3)����2��3ʱ�ն�û�ж����ж�Ϊ����˯��û����������ۼ�˯�ߡ������������*/
				((hour<8)&&(sedentary_time_temp > 180)))  /* ����˯�߼��ʱ���ۼ�150�������߶����߾�����Ĭ��˯�߼������Ϊ0*/
			{
				sedentary_time_temp = 0;
				light_sleep_time = 0;				 
				deep_sleep_time = 0;

				light_sleep_offset = 0;
				deep_sleep_offset = 0;
				g_light_sleep = 0;
				g_deep_sleep = 0;
			}
			else if((move == step)&&(watch_state >= 60)) /* ���ڼ�û���߶��� */
			{
				if(is_wearing())	//xb add 2021-03-02 ����֮�󲻼���˯��
				{
					if(watch_state >= (60*10))/* ����15��������δ�ζ������ж�Ϊ���˯��*/
						deep_sleep_time++; 
					else
						light_sleep_time++;
				}

				g_light_sleep = light_sleep_time + light_sleep_offset;
				g_deep_sleep = deep_sleep_time + deep_sleep_offset;
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
			light_sleep_offset = 0;
			deep_sleep_offset = 0;
			light_sleep_time = 0;
			deep_sleep_time = 0; 
			sedentary_time_temp = 0;			
			memset(waggle_level,0,sizeof(waggle_level)); /* �ζ��ȼ� */
		}

		last_sport.timestamp.year = date_time.year;
		last_sport.timestamp.month = date_time.month; 
		last_sport.timestamp.day = date_time.day;
		last_sport.timestamp.hour = date_time.hour;
		last_sport.timestamp.minute = date_time.minute;
		last_sport.timestamp.second = date_time.second;
		last_sport.timestamp.week = date_time.week;
		last_sport.deep_sleep = g_deep_sleep;
		last_sport.light_sleep = g_light_sleep;
		save_cur_sport_to_record(&last_sport);		
	}

	rtc_sec++;
	move = step;
}

int get_light_sleep_time(void) /* ǳ˯ʱ��*/
{
	return light_sleep_time*1.0;
}

int get_deep_sleep_time(void) /* ��˯ʱ��*/
{
	return deep_sleep_time*1.0;
}

static void sleep_timer_handler(struct k_timer *timer)
{
	update_sleep_parameter = true;
}

void StartSleepTimeMonitor(void)
{
	k_timer_init(&sleep_timer, sleep_timer_handler, NULL);
	k_timer_start(&sleep_timer, K_MSEC(1000), K_MSEC(1000));

	light_sleep_offset = last_sport.light_sleep;
	deep_sleep_offset = last_sport.deep_sleep;
	g_light_sleep = light_sleep_offset;
	g_deep_sleep = deep_sleep_offset;
}

void GetSleepTimeData(u16_t *deep_sleep, u16_t *light_sleep)
{
	*deep_sleep = g_deep_sleep;
	*light_sleep = g_light_sleep;
}

void GetSleepInfor(void)
{
	u16_t deep_sleep, light_sleep;

	GetSleepTimeData(&deep_sleep, &light_sleep);

	LOGD("deep_sleep:%d, light_sleep:%d", deep_sleep, light_sleep);
}

void UpdateSleepPara(void)
{
	u16_t steps;
	float sensor_x,sensor_y,sensor_z;
	int chg = 0;

	if(g_chg_status != BAT_CHARGING_NO)
		chg = 1;
	
	get_sensor_reading(&sensor_x, &sensor_y, &sensor_z);
	GetImuSteps(&steps);
	Set_Gsensor_data((signed short)sensor_x, (signed short)sensor_x, (signed short)sensor_x, steps, 80, date_time.hour, chg);
}
