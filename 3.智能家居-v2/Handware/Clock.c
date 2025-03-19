#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "LED.h"
#include "PWM.h"
#include "OLED.h"

extern uint8_t Flag_Count;							//�Ƿ��ڼ�ʱ��־��0Ϊ���ڼ�ʱ
extern uint8_t Flag_Change ;							//������������/����/ʱ�䣬0Ϊ�������ӣ�1Ϊ�������ڣ�2Ϊ����ʱ��

uint8_t KeyNum;									//��������ֵ
uint8_t Hour,Min,Sec;							//������������ʱ��ı���
uint8_t _year,_mon,_day,_hour,_min,_sec;		//�����������ʱ�����
uint32_t Alarm_CNT,Alarm_Time;	//�趨������ֵ

//���������Ӱ���

void Key_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA , &GPIO_InitStructure);
}

uint8_t Key_GetNum(void)
{
	//2024.12.18�����12.17�����⣬������δ��KeyNum��ʼ��Ϊ0
	/*
		�� C �����У��ֲ��������� KeyNum����û�г�ʼ��ʱ�����ʼֵ�ǲ�ȷ����*******
		��� KeyNum û�г�ʼ�������ĳ�ʼֵ�������κ����ֵ
		�ݣ�ԭ�������ԭ��
	*/
	uint8_t KeyNum = 0;
	uint32_t Temp = 600000;
	
	//����++
	if(GPIO_ReadInputDataBit(GPIOA , GPIO_Pin_3) == 0)
	{
		Delay_ms(30);
		while(GPIO_ReadInputDataBit(GPIOA , GPIO_Pin_3) == 0)//����������if���������ر�����
		{
			Temp--;
			if(Temp == 0)
			{
				//�Ӳ��������ʲô�����أ�
				//A:�����������
				Delay_ms(20);
				return 2;
			}
		}
		Delay_ms(20);
		KeyNum = 2;
	}
	
	if(GPIO_ReadInputDataBit(GPIOA , GPIO_Pin_4) == 0)
	{
		Delay_ms(30);
		while(GPIO_ReadInputDataBit(GPIOA , GPIO_Pin_4) == 0)
		{
			Temp--;
			if(Temp == 0)
			{
				//�Ӳ��������ʲô�����أ�
				Delay_ms(20);
				return 4;
			}
		}
		Delay_ms(20);
		KeyNum = 4;
	}
	return KeyNum;
}

/*�������ƺ���*/
void Key_Control(void)
{
	KeyNum = Key_GetNum();	//��ȡ��������
	
	if(Flag_Change == 0)	//��������
	{
		if(KeyNum == 1)		//1�Ű�������Сʱ
		{
			Hour++;
			if(Hour > 60)
				Hour = 0;
		}
		else if(KeyNum == 2)	//2�Ű�����������
		{
			Min++;
			if(Min > 60)
				Min = 0;
		}
		else if(KeyNum == 3)	//3�Ű���������
		{
			Sec++;
			if(Sec > 60)
				Sec = 0;
		}
		else if(KeyNum == 4)	//4�Ű���
		{	
			Alarm_Time = Hour*3600 + Min*60 + Sec;			//��������ʱ������λ����
			if(Alarm_Time > 0)
			{
				Alarm_CNT = RTC_GetCounter()+ Alarm_Time-1;			//�趨����ֵ����Ҫ-1
				//RTC_SetAlarm(Alarm_CNT);							//д������ֵ��RTC��ALR�Ĵ������ó�������û���������ν
																	//��Ϊ����ж����ӵ�û���Ǹ���ʱ��Ȳ��������������е�ʱ��
				Flag_Count = 1;
			}
			else	//������ʱ��Ϊ0����ת��������������
			{
				Flag_Change = 1;	
			}
		}
	}
	
	else if(Flag_Change == 1)		//��������
	{
		if(KeyNum == 1)			//1�Ű���������
		{
			//MyRTC_Time[0]++;
			_year++;
			//MyRTC_SetTime();
		}
		else if(KeyNum == 2)	//2�Ű���������
		{
			//MyRTC_Time[1]++;
			_mon++;
			//MyRTC_SetTime();
		}
		else if(KeyNum == 3)	//3�Ű���������
		{
			//MyRTC_Time[2]++;
			_day++;
			//MyRTC_SetTime();
		}
		else if(KeyNum == 4)	//4�Ű�������Ϊ����ʱ��
		{
			Flag_Change = 2;
		}			
	}
	
	else if(Flag_Change == 2)		//����ʱ��
	{
		if(KeyNum == 1)			//1�Ű�������Сʱ
		{
			//MyRTC_Time[3]++;
			_hour++;
			//MyRTC_SetTime();
		}
		else if(KeyNum == 2)	//2�Ű�����������
		{
			//MyRTC_Time[4]++;
			_min++;
			//MyRTC_SetTime();
		}
		else if(KeyNum == 3)	//3�Ű���������
		{
			//MyRTC_Time[5]++;
			_sec++;
			//MyRTC_SetTime();
		}
		else if(KeyNum == 4)	//4�Ű���
		{
			Flag_Change = 3;	//��Ϊ��������
		}			
	}
	else if(Flag_Change == 3)   //�˳�����
	{
		if(KeyNum == 4)
		{
			Flag_Change = 0;
			OLED_ShowString(4 , 1 , "                 ");
			OLED_ShowString(4,1,"SetClock");	//ˢ��oled������
		}
	}
}
