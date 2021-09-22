#ifndef __IMG_H__
#define __IMG_H__

/*Image2Lcd�����ͼ��������֯��ʽΪ��ͼ��ͷ����-��ɫ������-ͼ�����ݡ�
����ɫ/4��/16��/256ɫ����ͼ������ͷ���£�
typedef struct _HEADGRAY
{
   unsigned char scan;
   unsigned char gray;
   unsigned short w;
   unsigned short h;
}HEADGRAY;

scan: ɨ��ģʽ
Bit7: 0:��������ɨ�裬1:��������ɨ�衣 
Bit6: 0:�Զ�����ɨ�裬1:�Ե�����ɨ�衣 
Bit5: 0:�ֽ����������ݴӸ�λ����λ���У�1:�ֽ����������ݴӵ�λ����λ���С� 
Bit4: 0:WORD���͸ߵ�λ�ֽ�˳����PC��ͬ��1:WORD���͸ߵ�λ�ֽ�˳����PC�෴�� 
Bit3~2: ������ 
Bit1~0: [00]ˮƽɨ�裬[01]��ֱɨ�裬[10]����ˮƽ,�ֽڴ�ֱ��[11]���ݴ�ֱ,�ֽ�ˮƽ�� 

gray: �Ҷ�ֵ 
   �Ҷ�ֵ��1:��ɫ��2:�Ļң�4:ʮ���ң�8:256ɫ��12:4096ɫ��16:16λ��ɫ��24:24λ��ɫ��32:32λ��ɫ��

w: ͼ��Ŀ�ȡ�

h: ͼ��ĸ߶ȡ�

��4096ɫ/16λ���ɫ/18λ���ɫ/24λ���ɫ/32λ���ɫ����ͼ������ͷ���£�
typedef struct _HEADCOLOR
{
   unsigned char scan;
   unsigned char gray;
   unsigned short w;
   unsigned short h;
   unsigned char is565;
   unsigned char rgb;
}HEADCOLOR; 

scan��gray��w��h��HEADGRAY�ṹ�е�ͬ����Ա����������ͬ��

is565: ��4096ɫģʽ��Ϊ0��ʾʹ��[16bits(WORD)]��ʽ����ʱͼ��������ÿ��WORD��ʾһ�����أ�Ϊ1��ʾʹ��[12bits(�����ֽ���)]��ʽ����ʱ�������е�ÿ12Bits����һ�����ء�
��16λ��ɫģʽ��Ϊ0��ʾR G B��ɫ������ռ�õ�λ����Ϊ5Bits��Ϊ1��ʾR G B��ɫ������ռ�õ�λ���ֱ�Ϊ5Bits,6Bits,5Bits��
��18λ��ɫģʽ��Ϊ0��ʾ"6Bits in Low Byte"��Ϊ1��ʾ"6Bits in High Byte"��
��24λ��ɫ��32λ��ɫģʽ��is565��Ч��

rgb: ����R G B��ɫ����������˳��rgb��ÿ2Bits��ʾһ����ɫ������[00]��ʾ�հף�[01]��ʾRed��[10]��ʾGreen��[11]��ʾBlue��

��256ɫ���ĵ�ɫ�����ݽṹ���£�
typedef struct _PALENTRY
{
   unsigned char red;
   unsigned char green;
   unsigned char blue;
}PALENTRY;

typedef struct _PALETTE
{
   unsigned short palnum;
   PALENTRY palentry[palnum];
}PALETTE;

����256ɫģʽ�´��ڵ�ɫ�����ݽṹ,��ɫ�����ݽṹ���������ݽṹHEADGRAY֮��
*/

#if defined(LCD_VGM068A4W01_SH1106G)||defined(LCD_VGM096064A6W01_SP5090)
//LOGO
extern unsigned char logo_1_96X64[774];
extern unsigned char logo_2_96X64[774];
extern unsigned char logo_3_96X64[774];
extern unsigned char logo_4_96X64[774];
extern unsigned char logo_5_96X64[774];
//BATTERY
extern unsigned char IMG_BAT_0[38];
extern unsigned char IMG_BAT_1[38];
extern unsigned char IMG_BAT_2[38];
extern unsigned char IMG_BAT_3[38];
extern unsigned char IMG_BAT_4[38];
extern unsigned char IMG_BAT_5[38];
//SIGNAL
extern unsigned char IMG_SIG_0[22];
extern unsigned char IMG_SIG_1[22];
extern unsigned char IMG_SIG_2[22];
extern unsigned char IMG_SIG_3[22];
extern unsigned char IMG_SIG_4[22];
//AM&PM
extern unsigned char IMG_AM_CN[40];
extern unsigned char IMG_AM_EN[34];
extern unsigned char IMG_PM_CN[40];
extern unsigned char IMG_PM_EN[34];
//DATE
extern unsigned char IMG_DATE_LINK[24];
extern unsigned char IMG_DATE_NUM_0[36];
extern unsigned char IMG_DATE_NUM_1[36];
extern unsigned char IMG_DATE_NUM_2[36];
extern unsigned char IMG_DATE_NUM_3[36];
extern unsigned char IMG_DATE_NUM_4[36];
extern unsigned char IMG_DATE_NUM_5[36];
extern unsigned char IMG_DATE_NUM_6[36];
extern unsigned char IMG_DATE_NUM_7[36];
extern unsigned char IMG_DATE_NUM_8[36];
extern unsigned char IMG_DATE_NUM_9[36];
//TIME
extern unsigned char IMG_TIME_SPE_NO[42];
extern unsigned char IMG_TIME_SPE[42];
extern unsigned char IMG_TIME_NUM_0[57];
extern unsigned char IMG_TIME_NUM_1[57];
extern unsigned char IMG_TIME_NUM_2[57];
extern unsigned char IMG_TIME_NUM_3[57];
extern unsigned char IMG_TIME_NUM_4[57];
extern unsigned char IMG_TIME_NUM_5[57];
extern unsigned char IMG_TIME_NUM_6[57];
extern unsigned char IMG_TIME_NUM_7[57];
extern unsigned char IMG_TIME_NUM_8[57];
extern unsigned char IMG_TIME_NUM_9[57];
//MONTH
extern unsigned char IMG_MON_JAN[78];
extern unsigned char IMG_MON_FEB[78];
extern unsigned char IMG_MON_MAR[78];
extern unsigned char IMG_MON_APR[78];
extern unsigned char IMG_MON_MAY[78];
extern unsigned char IMG_MON_JUN[78];
extern unsigned char IMG_MON_JUL[78];
extern unsigned char IMG_MON_AUG[78];
extern unsigned char IMG_MON_SEP[78];
extern unsigned char IMG_MON_OCT[78];
extern unsigned char IMG_MON_NOV[78];
extern unsigned char IMG_MON_DEC[78];
//WEEK
extern unsigned char IMG_WEEK_1[93];
extern unsigned char IMG_WEEK_2[93];
extern unsigned char IMG_WEEK_3[93];
extern unsigned char IMG_WEEK_4[93];
extern unsigned char IMG_WEEK_5[93];
extern unsigned char IMG_WEEK_6[93];
extern unsigned char IMG_WEEK_7[93];
extern unsigned char IMG_WEEK_MON[84];
extern unsigned char IMG_WEEK_TUE[84];
extern unsigned char IMG_WEEK_WED[84];
extern unsigned char IMG_WEEK_THU[84];
extern unsigned char IMG_WEEK_FRI[84];
extern unsigned char IMG_WEEK_SAT[84];
extern unsigned char IMG_WEEK_SUN[84];
//BLE
extern unsigned char IMG_BLE_LINK[20];
extern unsigned char IMG_BLE_UNLINK[20];
//STEP
extern unsigned char IMG_STEP_ICON_1[390];
extern unsigned char IMG_STEP_ICON_2[390];
extern unsigned char IMG_STEP_CN[50];
extern unsigned char IMG_STEP_EN[90];
extern unsigned char IMG_STEP_NOTE_ICON_1[390];
extern unsigned char IMG_STEP_NOTE_ICON_2[390];
extern unsigned char IMG_STEP_NOTE_CN[66];
extern unsigned char IMG_STEP_NOTE_EN[122];
//DISTANCE
extern unsigned char IMG_DISTANCE_ICON_1[390];
extern unsigned char IMG_DISTANCE_ICON_2[390];
extern unsigned char IMG_KM_CN[102];
extern unsigned char IMG_KM_EN[102];
extern unsigned char IMG_MILE_CN[102];
extern unsigned char IMG_MILE_EN[102];
//CALORIE
extern unsigned char IMG_CAL_ICON_1[390];
extern unsigned char IMG_CAL_ICON_2[390];
extern unsigned char IMG_CAL_CN[106];
extern unsigned char IMG_CAL_EN[94];
//SLEEP
extern unsigned char IMG_SLEEP_ICON_1[390];
extern unsigned char IMG_SLEEP_ICON_2[390];
extern unsigned char IMG_SLEEP_ICON_3[390];
extern unsigned char IMG_SLEEP_ICON_4[390];
extern unsigned char IMG_SLEEP_HR_CN[50];
extern unsigned char IMG_SLEEP_HR_EN[54];
extern unsigned char IMG_SLEEP_MIN_CN[50];
extern unsigned char IMG_SLEEP_MIN_EN[66];
//FALL
extern unsigned char IMG_FALL_ICON_CN_1[774];
extern unsigned char IMG_FALL_ICON_CN_2[774];
extern unsigned char IMG_FALL_ICON_CN_3[774];
extern unsigned char IMG_FALL_ICON_EN_1[774];
extern unsigned char IMG_FALL_ICON_EN_2[774];
extern unsigned char IMG_FALL_ICON_EN_3[774];
extern unsigned char IMG_FALL_MSG_SEND_CN[774];
extern unsigned char IMG_FALL_MSG_CANCEL_CN[774];
extern unsigned char IMG_FALL_MSG_SEND_EN[774];
extern unsigned char IMG_FALL_MSG_CANCEL_EN[774];
//FIND
extern unsigned char IMG_FIND_ICON_1[390];
extern unsigned char IMG_FIND_ICON_2[390];
extern unsigned char IMG_FIND_ICON_3[390];
extern unsigned char IMG_FIND_CN[390];
extern unsigned char IMG_FIND_EN[390];
#if 0
//CHARGING
extern unsigned char IMG_CHARGING_OK[774];
extern unsigned char IMG_CHARGE_PLS_CN[774];
extern unsigned char IMG_CHARGE_PLS_EN[774];
extern unsigned char IMG_CHARGING_CN_0[774];
extern unsigned char IMG_CHARGING_CN_1[774];
extern unsigned char IMG_CHARGING_CN_2[774];
extern unsigned char IMG_CHARGING_CN_3[774];
extern unsigned char IMG_CHARGING_CN_4[774];
extern unsigned char IMG_CHARGING_CN_5[774];
extern unsigned char IMG_CHARGING_EN_0[774];
extern unsigned char IMG_CHARGING_EN_1[774];
extern unsigned char IMG_CHARGING_EN_2[774];
extern unsigned char IMG_CHARGING_EN_3[774];
extern unsigned char IMG_CHARGING_EN_4[774];
extern unsigned char IMG_CHARGING_EN_5[774];
#endif
//SOS
extern unsigned char IMG_SOS_ICON[390];
extern unsigned char IMG_SOS_SENDING_1[390];
extern unsigned char IMG_SOS_SENDING_2[390];
extern unsigned char IMG_SOS_SENDING_3[390];
extern unsigned char IMG_SOS_SENDING_OK[390];
//WRIST OFF
extern unsigned char IMG_WRIST_OFF_CN[774];
extern unsigned char IMG_WRIST_OFF_EN[774];
//ALARM
extern unsigned char IMG_ALARM_ICON_1[390];
extern unsigned char IMG_ALARM_ICON_2[390];
extern unsigned char IMG_ALARM_ICON_3[390];
//NUM
extern unsigned char IMG_MID_NUM_0[62];
extern unsigned char IMG_MID_NUM_1[62];
extern unsigned char IMG_MID_NUM_2[62];
extern unsigned char IMG_MID_NUM_3[62];
extern unsigned char IMG_MID_NUM_4[62];
extern unsigned char IMG_MID_NUM_5[62];
extern unsigned char IMG_MID_NUM_6[62];
extern unsigned char IMG_MID_NUM_7[62];
extern unsigned char IMG_MID_NUM_8[62];
extern unsigned char IMG_MID_NUM_9[58];
//DOT
extern unsigned char IMG_DOT[21];
//POWER
extern unsigned char IMG_PWROFF_KEY_CN[774];
extern unsigned char IMG_PWROFF_KEY_EN[774];
extern unsigned char IMG_PWROFF_CN[774];
extern unsigned char IMG_PWROFF_EN[774];

#endif/*LCD_VGM068A4W01_SH1106G*/

#endif/*__IMG_H__*/
