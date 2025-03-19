#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "PWM.h"
#include "OLED.h"
#include "ADC.h"
#include "LED.h"
#include "stdio.h"
#include "time.h"
#include "TemperSensor.h"
#include "MYRTC.h"
#include "Clock.h"
#include "Scene.h"
#include "Sensing.h"
#include "SerialPort.h"

extern uint16_t Key_Value;
extern uint8_t Hour,Min,Sec;							//�����������ӵ�ʱ�����
extern uint8_t _year,_mon,_day,_hour,_min,_sec;			//�����������ʱ�����
extern uint32_t Alarm_Time;	//������ر�������λ������

uint16_t i;
float LightValue;
float TemperValue;

uint16_t MyRTC_Time[] = {2024,12,17,00,00,50};	//����ȫ�ֵ�ʱ�����飬�������ݷֱ�Ϊ�ꡢ�¡��ա�ʱ���֡���
uint8_t Flag_Count = 0;							//�Ƿ��ڼ�ʱ��־��0Ϊ���ڼ�ʱ
uint8_t Flag_Change = 3;						//������������/����/ʱ�䣬0Ϊ�������ӣ�1Ϊ�������ڣ�2Ϊ����ʱ��


int main(void)
{
	OLED_Init();
	MyRTC_Init();
	Key_Init();//�����������Ӳ���
	AD_Init();
	TemperSensor_Init();
	Sensing_Init();
	SerialPort_Init();
	//�������Ƴ����л�����
	Key1Interrupt_Init();//����1��ʼ������Ӧ����1��
	Key2Interrupt_Init();//����2��ʼ������Ӧ����2��
	Key3Interrupt_Init();//�����ƶ���
	
	OLED_ShowString(1, 1, "Date:XXXX-XX-XX");
	OLED_ShowString(2, 1, "Time:XX:XX:XX");
	OLED_ShowString(3, 1, "Alarm:XX:XX:XX");
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	while(1)
	{
		MyRTC_ReadTime();
		Key_Control();//�������ӵİ���
		check();//Ҫ�ȴ�RTC�ж�ȡʱ���ٽ���check����
		
		/*��������������������Сת����0~100*/
		LightValue = (float)AD_GetValue(ADC_Channel_0) / 4095 * 100;
		
		OLED_ShowNum(1, 6, MyRTC_Time[0], 4);		//��
		OLED_ShowNum(1, 11, MyRTC_Time[1], 2);		//��
		OLED_ShowNum(1, 14, MyRTC_Time[2], 2);		//��
		OLED_ShowNum(2, 6, MyRTC_Time[3], 2);		//ʱ
		OLED_ShowNum(2, 9, MyRTC_Time[4], 2);		//��
		OLED_ShowNum(2, 12, MyRTC_Time[5], 2);		//��
		OLED_ShowNum(4 , 16 , Key_GetValue() , 1);	//��ʾ����ģʽ��
		
		//Flag_Count��Ӧ�Ƿ������Ӽ�ʱ��1Ϊ���Ӽ�ʱ��0Ϊû����������
		
		/*
			2024.12.17�о����������⣬�߼����󣬻�ͷ���ĸ���
		*/
		if(Flag_Count)										//���ڼ�ʱ������ʾ��������ʣ��ʱ��
		{
			if((MyRTC_Time[3] == Hour && MyRTC_Time[4] == Min && MyRTC_Time[5] == Sec))			
			//if(RTC_GetFlagStatus(RTC_FLAG_ALR) == 1)		//����ʱ�䵽������־λΪ1
			{
				//RTC_ClearFlag(RTC_FLAG_ALR);				//�����־λ
				Flag_Count = 0;Alarm_Time = 0;				//������ز���
				Hour = 0;Min = 0; Sec = 0;
				
				//2024.12.11���е���һ��
				//Ŀǰ���ӽ�����Ʋ��仯������ܽ���أ���
				//�����л��ʹ���������û���⣿���Ƿ�LED���ó�һ�������ĺ�����������Ԥ���ǿ��Եģ�
				
				Key_Value = 3;			//���ӵ��ˣ��ø�ֵΪ4��ʹ֮�ܹ�ʵ������ʱ�䵽������Ĺ���
				OLED_ShowString(4,1,"Time Out");
			}

			else											//����ʱ��δ��
			{
				OLED_ShowString(4,1,"Counting");			//��ʾ���ڼ�ʱ
			}
		}
		else												//���ڼ�ʱ������ʾ��Ҫ�趨������ʱ��
		{
			OLED_ShowNum(3,7,Hour,2);
			OLED_ShowNum(3,10,Min,2);
			OLED_ShowNum(3,13,Sec,2);
		}
		
		/*
			��ʾĿǰ�����ý���
			1.�������ӣ�����Ϊʲôû����ʾ��ԭ���ǣ���ʾSet Clock֮ǰҪ������һ�У���Ϊ��������е�OLED��������ʹ���Դ��������ʽ��
						�������������Ļ��������һֱѭ����������˸�Ļ��棩
			2.��������
			3.����ʱ��
		*/
		if(Flag_Change == 1)		//��ʾ���������ڡ�
		{
			OLED_ShowString(4 , 1 , "Change Date");
		}
		else if(Flag_Change == 2)	//��ʾ������ʱ�䡱
		{
			OLED_ShowString(4 , 1 , "Change Time");
		}
		else if(Flag_Change == 3)	//��������գ������˳�����
		{
			OLED_ShowString(4 , 1 , "              ");
		}
		
		/*
			��⴮�ڽ������ݱ�־λ 
			����1��Ӧ����1
			����2��Ӧ����2
			�����������ֶ�Ӧ�ص�
		*/
		if(SerialPort_GetRxFlag() == 1)
		{
			if(SerialPort_GetRxData() == 1)
			{
				Key_Value = 1;
			}
			else if(SerialPort_GetRxData() == 2)
			{
				Key_Value = 2;
			}
			else
			{
				Key_Value = 3;
			}	
		}
		
		/* �����ǳ����л��Լ��������������¶ȴ��������Ƶƹ� */
		switch(Key_GetValue())
		{
			case 1:
				if(TemperSensor_Get() == 1)//�¶����ߣ����������С�����ڵ������ֲ��䣬ͬ������˵�ѹ��С����ʱ��������˵�ѹ����ͬ������ˣ�����͵�ƽ
				{
					PWM1_Init();
					PWM1_SetCompare(130 - LightValue);//��Խ��������������ֵԽС����ֵԽ��LED��Խ��
				}
				if(TemperSensor_Get() == 0)
				{
					LED1_OFF();
				}
				LED2_OFF();
				break;
			case 2:
				if(Sensing_Get() == 1)
				{
					PWM2_Init();
					PWM2_SetCompare(130 - LightValue);//��Խ��������������ֵԽС����ֵԽ��LED��Խ��
				}
				if(Sensing_Get() == 0)
				{
					LED2_OFF();
				}
				LED1_OFF();
				break;
			case 3:
				LED1_OFF();
				LED2_OFF();
				break;
		}
	}
}
