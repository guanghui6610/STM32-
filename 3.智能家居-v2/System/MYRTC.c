#include "stm32f10x.h"                  // Device header
#include <time.h>//�ṩ��׼ʱ�䴦����
#include "MYRTC.h"

extern uint16_t MyRTC_Time[];//ȫ�ֱ��������ڱ��浱ǰ���ꡢ�¡��ա�ʱ���֡���
extern uint8_t _year,_mon,_day,_hour,_min,_sec;//ʱ����������������Զ�ȡ��ʱ�����ƫ�Ƶ���
uint8_t get_day[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};//ÿ�µ�������
/*
	��Ҫ�������ṩ��������
	uint16_t MyRTC_Time[] = {2024,2,22,10,0,0};	//����ȫ�ֵ�ʱ�����飬�������ݷֱ�Ϊ�ꡢ�¡��ա�ʱ���֡���

	ʹ��LSI(Low-Speed Internal oscillator,������ʱ��Դ)����RTCCLK��LSI�޷��ɱ��õ�Դ���磬������Դ����ʱ��RTC��ʱ����ͣ
	��ʼ��оƬ��ʵʱʱ�ӣ�RTC��ģ�飬��ȷ��RTCģ��������׼ȷ�ؽ��м�ʱ����
*/
void MyRTC_Init(void)
{
	//�򿪵�Դ�ͺ󱸽ӿڵ�ʱ�ӣ�ʹ�ܶԱ��ݼĴ�����RTC�ķ���
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);//PWR����Դģ�飩
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP, ENABLE);//BKP������ģ�飩�ڸ�λ���ߵ�������£�����һЩ��Ҫ����
	
	/*
		������ʱ�����
	*/
	PWR_BackupAccessCmd(ENABLE);
	
	/*
		ǿ�ƶԱ�������и�λ����
		��λ�����Ὣ�������е���ؼĴ�����״̬�ָ�����ʼֵ����Ԥ����ĸ�λ״̬
		���Է�ֹ֮ǰ�����ø��ŵ�ǰ�� RTC ��ʼ����ȷ���������õ���ȷ�ԡ�
	*/
	
	RCC_BackupResetCmd(ENABLE);
	/*
		ȡ��֮ǰ�������õı�����λ״̬��������������������
		��һ��ͨ������ɸ�λ������ʹ�ñ������еĹ���ģ�飨�籸�ݼĴ����� RTC���ܹ������ض�д���ݺ�ִ���书�ܡ�
		���ڱ�����λ�ᵼ�����ݶ�ʧ��ϵͳ���ܻ���Ϊ��ʧ���ݼĴ����е���Ҫ���ݶ��޷������û�֮ǰ����������������
	*/
	/*
	��λ��ȡ����λ���ʹ�ã�
		������λ���������������ݣ��������ȡ����λ״̬��������ģ�齫�޷�����������
		��λ��ȡ����λ�Ľ��ʹ�ã�����ȷ���������״̬�ɾ���ͬʱ�ܹ����������书�ܡ�
	*/
	RCC_BackupResetCmd(DISABLE);
	
	/*
		��鱸�ݼĴ����е�BKP_DR1���Ƿ�洢�˱�־ֵ0xA5A5
		����Ĵ���ֵ����0xA5A5��˵��RTCδ��ʼ���������״������߼���
		����RTC�����ù���������״������߼���
		�״������߼�
	*/
	if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)
	{
		/*
			���õ����ڲ�ʱ�ӣ�LSI�����ȴ����ȶ�
			LSIʱ����STM32�ڲ��ṩ�ĵ;���ʱ�ӣ�ͨ������RTC�Ϳ��Ź���ʱ
			RCC_GetFlagStatus(RCC_FLAG_LSIRDY)���LSI�Ƿ�׼������
		*/
		RCC_LSICmd(ENABLE);
		while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) != SET);
		
		/*
			����RTCʱ��ԴΪLSI
			RTCģ����Ҫһ��ʱ��Դ���ܹ���������ѡ��LSI��Ϊʱ��Դ
			����RTCʱ��
		*/
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
		RCC_RTCCLKCmd(ENABLE);
		
		/*
			ͬ��RTC�Ĵ���
			RTC_WaitForSynchro()ȷ��RTC�Ĵ�����ϵͳʱ��ͬ��
			RTC_WaitForLastTask()�ȴ���һ��RTC������ɣ������ͻ
		*/
		RTC_WaitForSynchro();
		RTC_WaitForLastTask();
		
		/*
			����RTC��Ԥ��Ƶ��
			40000-1��Ԥ��Ƶֵ����LSIƵ�ʣ�Լ40kHz����Ƶ��1Hz
			RTC������ÿ�����1��ʵ���뼶��ʱ
		*/
		RTC_SetPrescaler(40000 - 1);
		RTC_WaitForLastTask();
		
		/*
			�����Զ��庯��MyRTC_SetTime����ȫ�������еĳ�ʼʱ��д��RTC
		*/
		MyRTC_SetTime();
		
		/*
			�ڱ��ݼĴ�����д���־ֵ0xA5A5������RTC����ɳ�ʼ��
		*/
		BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
	}
	
	/*
	���״������߼�:����������LSI��RTCʱ�ӣ�ȷ��RTC������ʱ
	*/
	else
	{
		RCC_LSICmd(ENABLE);				//��ʹ���ǵ�һ�����ã�Ҳ��Ҫ�ٴο���LSIʱ��
		while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) != SET);
		
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
		RCC_RTCCLKCmd(ENABLE);
		
		RTC_WaitForSynchro();
		RTC_WaitForLastTask();
	}
}

//�ж�����Ƿ�������
int isLeapYear(int year) 
{
	if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
	{
		return 1;  // ������
	} 
	else
	{
		return 0;  // ��������
	}
}

/* 	
	��MYRTC_Time�����еĸ�Ԫ�ؽ��к����Լ��ͷ�Χ������ȷ��ʱ�����ݴ��ں����ȡֵ��Χ֮�ڡ�
	������ʱ��ʱ���ֹ���ַǳ���ʱ�䣩
*/
void check(void)
{
	if(isLeapYear(MyRTC_Time[0])) get_day[2] = 29;else get_day[2] = 28;					// �ж�����
	if(MyRTC_Time[1] != 13) MyRTC_Time[1] %= 13;	else MyRTC_Time[1] = 1;				// �·�
	if(MyRTC_Time[1] == 0) MyRTC_Time[1] = 1;
	if(MyRTC_Time[2] != get_day[MyRTC_Time[1]]) MyRTC_Time[2] %= get_day[MyRTC_Time[1]];// ����
	MyRTC_Time[3] %= 24;// Сʱ
	MyRTC_Time[4] %= 60;// ����
	MyRTC_Time[5] %= 60;// ��
}

/**
  * ��    ����RTC����ʱ��
  * ��    ������
  * �� �� ֵ����
  * ˵    �������ô˺�����ȫ��������ʱ��ֵ��ˢ�µ�RTCӲ����·
  */
void MyRTC_SetTime(void)
{
/*
	time_t ��һ�����ڱ�ʾʱ��Ļ����������ͣ���ͨ��������Ϊ�ܹ��洢��ĳ���ض���ʼʱ��
	������ Unix ʱ�������ʼʱ�䣬�� 1970 �� 1 �� 1 �� 00:00:00 UTC��������ʾʱ�����������������͡�
*/
	time_t time_cnt;		//�������������������
/*	
	struct tm �� C ���Ա�׼���ж����һ���ṹ�����ͣ�ר�����ڴ�ŷֽ������ں�ʱ����Ϣ��
	���Ľṹ���Աһ��������ꡢ�¡��ա�ʱ���֡����Լ����ڼ��ȶ����ʱ����صķ���
*/
	struct tm time_date;	//��������ʱ����������
	
	//�������ʱ�丳ֵ������ʱ��ṹ��
	time_date.tm_year = MyRTC_Time[0] - 1900;//tm_year��¼���Ǵ� 1900 �꿪ʼ����ǰʱ�侭�������������� 2024 ���������Ա�д洢��ֵ���� 124��2024 - 1900����
	time_date.tm_mon = MyRTC_Time[1] - 1 ;	//tm_monȡֵ��Χ0-11
	time_date.tm_mday = MyRTC_Time[2];		//tm_mdayȡֵ��Χ1-31
	time_date.tm_hour = MyRTC_Time[3];		//tm_hourȡֵ��Χ0-23
	time_date.tm_min = MyRTC_Time[4];		//tm_min ȡֵ��Χ0-59
	time_date.tm_sec = MyRTC_Time[5];		//tm_sec ȡֵ��Χ0-59
	
	/* 
		mktime ������ C ��׼���зǳ���Ҫ������ʱ�䴦��ĺ��������Ĺ����ǽ� struct tm �ṹ���ʾ�����ں�ʱ����Ϣת��Ϊ time_t ���͵�ʱ��ֵ��
		Ҳ���Ǽ������ 1970 �� 1 �� 1 �� 00:00:00 UTC ��ʼ�������ṹ������ʾʱ��������� 
	*/
	time_cnt = mktime(&time_date);	//����mktime������������ʱ��ת��Ϊ���������ʽ
	
	/*
		ͨ���������������������ǰ��ת����ʱ���������ʱ������ time_cnt д�뵽 RTC �ļ�������ʵʱʱ��оƬ�ڲ����ڼ���ʱ��ļĴ�����
		��������������ʵ�ֽ�������洦��õ�ʱ�����ݴ��ݵ�Ӳ��ʵʱʱ�ӵĹؼ�����
		ʹ�� RTC �ܹ�����д���������ʼ׼ȷ��ʱ���Ӷ����ʱ�����õĺ���Ŀ�ꡣ
	*/
	RTC_SetCounter(time_cnt);						//���������д�뵽RTC��CNT��
	RTC_WaitForLastTask();							//�ȴ���һ�β������
}

/**
  * ��    ����RTC��ȡʱ��
  * ��    ������
  * �� �� ֵ����
  * ˵    �������ô˺�����RTCӲ����·��ʱ��ֵ��ˢ�µ�ȫ������
  */

/*
	�ú�����Ҫ���ڴ�Ӳ��ʵʱʱ�ӣ�RTC���ж�ȡ��ǰʱ����Ϣ
	������ת��Ϊ�Զ���� MyRTC_Time �����ʽ���д洢
*/
void MyRTC_ReadTime(void)
{
	time_t time_cnt;		//�������������������
	struct tm time_date;	//��������ʱ����������
	
	time_cnt = RTC_GetCounter();		//��ȡRTC��CNT����ȡ��ǰ���������
	
	/* 
		localtime��������һ��ָ��time_t���͵�ָ����Ϊ������������һ��ָ��tm�ṹ��ָ�룬�ýṹ�����˱���ʱ�����ϸ��Ϣ��
		C������*��ʾ����ָ��ָ���ֵ
	*/
	time_date = *localtime(&time_cnt);				//ʹ��localtime���������������ת��Ϊ����ʱ���ʽ
	
	MyRTC_Time[0] = time_date.tm_year + 1900 + _year;		//������ʱ��ṹ�帳ֵ�������ʱ��
	MyRTC_Time[1] = time_date.tm_mon  + _mon + 1;
	MyRTC_Time[2] = time_date.tm_mday + _day;
	MyRTC_Time[3] = time_date.tm_hour + _hour;
	MyRTC_Time[4] = time_date.tm_min + _min;
	MyRTC_Time[5] = time_date.tm_sec +_sec;
}

