#ifndef __RTC_H
#define __RTC_H

#include "stm32f10x.h"

// �Զ���ʱ��ṹ��
typedef struct {
    uint8_t Hours;
    uint8_t Minutes;
    uint8_t Seconds;
} RTC_TimeTypeDef;

// �Զ������ڽṹ��
typedef struct {
    uint16_t Year;   // ���
    uint8_t Month;   // �·� (1~12)
    uint8_t Date;    // ���� (1~31)
    uint8_t WeekDay; // ���� (0~6, 0��������)
} RTC_DateTypeDef;

// �����ʽѡ��
#define RTC_FORMAT_BIN 0  // �����Ƹ�ʽ
#define RTC_FORMAT_BCD 1  // BCD ��ʽ

// RTC ���ú���
void RTC_Init(void);
void I2C1_Init(void);

// �Զ��� RTC ���ü�����
void RTC_SetCounterCustom(uint32_t CounterValue);
uint32_t RTC_GetCounterCustom(void);

// RTC �Զ���ʱ�����������ú���
void RTC_SetTime(RTC_TimeTypeDef *sTime);
void RTC_SetDate(RTC_DateTypeDef *sDate);

// RTC �Զ���ʱ�������ڻ�ȡ����
ErrorStatus RTC_GetTime(RTC_TimeTypeDef *sTime, uint8_t Format);
ErrorStatus RTC_GetDate(RTC_DateTypeDef *sDate);

#endif /* __RTC_H */
