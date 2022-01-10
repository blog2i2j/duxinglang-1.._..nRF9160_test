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

#if 1
//analog clock
extern unsigned char IMG_ANALOG_CLOCK_BG[115208];
extern unsigned char IMG_ANALOG_CLOCK_DOT_WHITE[458];
extern unsigned char IMG_ANALOG_CLOCK_DOT_RED[250];
extern unsigned char IMG_ANALOG_CLOCK_HAND_SEC[332];
extern unsigned char IMG_ANALOG_CLOCK_HAND_MIN[1268];
extern unsigned char IMG_ANALOG_CLOCK_HAND_HOUR[962];
//battery
extern unsigned char IMG_BAT_RECT_RED[968];
extern unsigned char IMG_BAT_RECT_WHITE[968];
//blood pressure
extern unsigned char IMG_BP_BG[35368];
extern unsigned char IMG_BP_UP_ARRAW[586];
extern unsigned char IMG_BP_DOWN_ARRAW[392];
extern unsigned char IMG_BP_DOWN_LINE[64];
extern unsigned char IMG_BP_MEASUREING[6520];
extern unsigned char IMG_BP_MEASURENING_ANI_1[42596];
extern unsigned char IMG_BP_MEASURENING_ANI_2[42596];
extern unsigned char IMG_BP_MEASURENING_ANI_3[42596];
extern unsigned char IMG_BP_MMHG[1044];
extern unsigned char IMG_BP_TIME_DOT[28];
//heart rate
extern unsigned char IMG_HR_BG[35440];
extern unsigned char IMG_HR_BPM_BIG[1988];
extern unsigned char IMG_HR_BPM_MID[1232];
extern unsigned char IMG_HR_BPM_SMALL[764];
extern unsigned char IMG_HR_DOWN_ARRAW[548];
extern unsigned char IMG_HR_DOWN_LINE[80];
extern unsigned char IMG_HR_ICON[2116];
extern unsigned char IMG_HR_MEASURING[2816];
extern unsigned char IMG_HR_STATIC[2340];
extern unsigned char IMG_HR_TIME_DOT[28];
extern unsigned char IMG_HR_UP_ARRAW[484];
//idle step
extern unsigned char IMG_IDLE_STEP_LOGO[20072];
//ota
extern unsigned char IMG_OTA_DOWNLOADING[7460];
extern unsigned char IMG_OTA_FAILED_ICON[21640];
extern unsigned char IMG_OTA_FINISH[115208];
extern unsigned char IMG_OTA_LOGO[7208];
extern unsigned char IMG_OTA_NO[3208];
extern unsigned char IMG_OTA_RUNNING_FAIL[3572];
extern unsigned char IMG_OTA_RUNNING_STR[5516];
extern unsigned char IMG_OTA_STR[19748];
extern unsigned char IMG_OTA_YES[3208];
//power off
extern unsigned char IMG_PWROFF_ANI_1[29776];
extern unsigned char IMG_PWROFF_ANI_2[29776];
extern unsigned char IMG_PWROFF_ANI_3[29776];
extern unsigned char IMG_PWROFF_ANI_4[29776];
extern unsigned char IMG_PWROFF_ANI_5[29776];
extern unsigned char IMG_PWROFF_ANI_6[29776];
extern unsigned char IMG_PWROFF_ANI_7[29776];
extern unsigned char IMG_PWROFF_ANI_8[29776];
extern unsigned char IMG_PWROFF_BG[115208];
extern unsigned char IMG_PWROFF_BUTTON[12808];
extern unsigned char IMG_PWROFF_LOGO[1508];
extern unsigned char IMG_PWROFF_NO[3208];
extern unsigned char IMG_PWROFF_RUNNING_STR[8626];
extern unsigned char IMG_PWROFF_STR[5732];
extern unsigned char IMG_PWROFF_YES[3208];
//power on
extern unsigned char IMG_PWRON_ANI_1[13388];
extern unsigned char IMG_PWRON_ANI_2[13388];
extern unsigned char IMG_PWRON_ANI_3[13388];
extern unsigned char IMG_PWRON_ANI_4[13388];
extern unsigned char IMG_PWRON_ANI_5[13388];
extern unsigned char IMG_PWRON_ANI_6[13388];
extern unsigned char IMG_PWRON_BG[115208];
extern unsigned char IMG_PWRON_STR[9908];
//3 dot gif
extern unsigned char IMG_RUNNING_ANI_1[2114];
extern unsigned char IMG_RUNNING_ANI_2[2114];
extern unsigned char IMG_RUNNING_ANI_3[2114];
extern unsigned char IMG_RUNNING_ANI_4[2114];
//signal
extern unsigned char IMG_SIG_0[1744];
extern unsigned char IMG_SIG_1[1744];
extern unsigned char IMG_SIG_2[1744];
extern unsigned char IMG_SIG_3[1744];
extern unsigned char IMG_SIG_4[1744];
//sim information
extern unsigned char IMG_SIM_ICCID[2524];
extern unsigned char IMG_SIM_IMSI[2388];
//sleep
extern unsigned char IMG_SLEEP_ANI_1[4608];
extern unsigned char IMG_SLEEP_ANI_2[4608];
extern unsigned char IMG_SLEEP_ANI_3[4608];
extern unsigned char IMG_SLEEP_DEEP_ICON[2600];
extern unsigned char IMG_SLEEP_HOUR[844];
extern unsigned char IMG_SLEEP_LIGHT_ICON[2600];
extern unsigned char IMG_SLEEP_LINE[712];
extern unsigned char IMG_SLEEP_MIN[1460];
//step
extern unsigned char IMG_STEP_ANI_1[4036];
extern unsigned char IMG_STEP_ANI_2[4036];
extern unsigned char IMG_STEP_CAL_ICON[2024];
extern unsigned char IMG_STEP_DIS_ICON[2096];
extern unsigned char IMG_STEP_ICON[2208];
extern unsigned char IMG_STEP_KCAL[708];
extern unsigned char IMG_STEP_KM[484];
extern unsigned char IMG_STEP_LINE[668];
//sync
extern unsigned char IMG_SYNC_ERR[24868];
extern unsigned char IMG_SYNC_FINISH[24868];
extern unsigned char IMG_SYNC_LOGO[20808];
extern unsigned char IMG_SYNC_STR[8288];
//wrist off
extern unsigned char IMG_WRIST_OFF_ICON[23372];
#endif

#endif/*__FONT_H__*/
