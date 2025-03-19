#include "stm32f10x.h"                  // Device header

void AD_Init(void)
{
	 //1.�����ⲿʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 , ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	//2.����GPIO
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA , &GPIO_InitStructure);
	//3.����ADCת����
	ADC_InitTypeDef ADC_InitStructure;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None ;
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent ;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_Init(ADC1 , &ADC_InitStructure);
	//4.����ADC���ؿ���
	ADC_Cmd(ADC1 , ENABLE);
	//5.ADCУ׼���̶����̣��ڲ��е�·���Զ�ִ��У׼
	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1) == SET);
	ADC_StartCalibration(ADC1);
	while(ADC_GetSoftwareStartConvStatus(ADC1) == SET);
}

uint16_t AD_GetValue(uint16_t ADC_Channel)
{
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);//ADC�������
	
	ADC_RegularChannelConfig(ADC1 , ADC_Channel , 1 , ADC_SampleTime_55Cycles5);//����������1��λ�ã�����Ϊͨ��ADC_Channel����ÿ��ת��ǰ�����ݺ����β������Ĺ������ͨ��1
	while(ADC_GetFlagStatus(ADC1 ,ADC_FLAG_EOC) == RESET);//�ȴ�ת����ɲ���ȡ��Ӳ�����Զ����EOC��־λ
	
	return ADC_GetConversionValue(ADC1);
}
