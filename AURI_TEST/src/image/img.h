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

#if 0
//extern unsigned char peppa_pig_80X160[25608];
//extern unsigned char peppa_pig_160X160[51208];
//extern unsigned char peppa_pig_240X240_1[57608];
//extern unsigned char peppa_pig_240X240_2[57600];
//extern unsigned char peppa_pig_320X320_1[51208];
//extern unsigned char peppa_pig_320X320_2[51200];
//extern unsigned char peppa_pig_320X320_3[51200];
//extern unsigned char peppa_pig_320X320_4[51200];
//extern unsigned char RM_LOGO_240X240_1[57608];
//extern unsigned char RM_LOGO_240X240_2[57600];
#endif

#if defined(LCD_VGM068A4W01_SH1106G)||defined(LCD_VGM096064A6W01_SP5090)
extern unsigned char jjph_gc_96X64[774];
extern unsigned char jjph_gc_96X32[390];

extern unsigned char IMG_BAT_0[38];
extern unsigned char IMG_BAT_1[38];
extern unsigned char IMG_BAT_2[38];
extern unsigned char IMG_BAT_3[38];
extern unsigned char IMG_BAT_4[38];
extern unsigned char IMG_BAT_5[38];

extern unsigned char IMG_SIG_0[22];
extern unsigned char IMG_SIG_1[22];
extern unsigned char IMG_SIG_2[22];
extern unsigned char IMG_SIG_3[22];
extern unsigned char IMG_SIG_4[22];

extern unsigned char IMG_COLON[34];
extern unsigned char IMG_NO_COLON[34];
extern unsigned char IMG_BIG_NUM_0[70];
extern unsigned char IMG_BIG_NUM_1[70];
extern unsigned char IMG_BIG_NUM_2[70];
extern unsigned char IMG_BIG_NUM_3[70];
extern unsigned char IMG_BIG_NUM_4[70];
extern unsigned char IMG_BIG_NUM_5[70];
extern unsigned char IMG_BIG_NUM_6[70];
extern unsigned char IMG_BIG_NUM_7[70];
extern unsigned char IMG_BIG_NUM_8[70];
extern unsigned char IMG_BIG_NUM_9[70];

extern unsigned char IMG_NUM_0[46];
extern unsigned char IMG_NUM_1[46];
extern unsigned char IMG_NUM_2[46];
extern unsigned char IMG_NUM_3[46];
extern unsigned char IMG_NUM_4[46];
extern unsigned char IMG_NUM_5[46];
extern unsigned char IMG_NUM_6[46];
extern unsigned char IMG_NUM_7[46];
extern unsigned char IMG_NUM_8[46];
extern unsigned char IMG_NUM_9[46];

extern unsigned char IMG_FALL_ICON[114];
extern unsigned char IMG_FALL_CN[194];
extern unsigned char IMG_FALL_EN[114];

extern unsigned char IMG_SLP_ICON[36];
extern unsigned char IMG_STEP_ICON[66];

extern unsigned char IMG_HOUR_CN[38];
extern unsigned char IMG_MIN_CN[38];
extern unsigned char IMG_HOUR_EN[30];
extern unsigned char IMG_MIN_EN[34];

extern unsigned char IMG_SOS[314];
extern unsigned char IMG_SOS_RECE[342];
extern unsigned char IMG_SOS_SEND[342];

extern unsigned char IMG_WRIST_ICON[90];
extern unsigned char IMG_WRIST_CN[194];
extern unsigned char IMG_WRIST_EN[226];

#endif/*LCD_VGM068A4W01_SH1106G*/

#endif/*__FONT_H__*/
