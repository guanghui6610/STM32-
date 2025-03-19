#include "RTC.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_rtc.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_bkp.h"
#include <stdint.h>  // ȷ������ uint8_t, uint16_t �ȶ���
#include <stddef.h>  // ���� NULL �Ķ���

// �������յ������� (ƽ��)
const uint8_t RTC_MonthDays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// �����������ж��Ƿ�Ϊ����
static uint8_t RTC_IsLeapYear(uint16_t year) {
    return ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0));
}

// ��������ת��Ϊ BCD ��ʽ
uint8_t ByteToBCD2(uint8_t value) {
    return ((value / 10U) << 4U) | (value % 10U);
}

// �� BCD ת��Ϊ������
uint8_t BCD2ToByte(uint8_t value) {
    return ((value >> 4U) * 10U) + (value & 0x0FU);
}

// RTC ��ʼ������
void RTC_Init(void) {
    // ���� PWR �� BKP ʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP, ENABLE);

    // ʹ���ڲ�����ʱ�ӣ�LSI����Ϊ RTC ʱ��Դ
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);  // LSI ʱ��Դ 40kHz
    RCC_RTCCLKCmd(ENABLE); // ���� RTC ʱ��

    // ������� RTC �Ĵ���
    PWR_BackupAccessCmd(ENABLE);

    // �ȴ� RTC �Ĵ���ͬ��
    RTC_WaitForSynchro();

    // ���� RTC ��Ƶ��������ʱ���׼Ϊ 1 ��
    RTC_SetPrescaler(127); // LSI ʱ��Ϊ 40kHz����Ƶ������Ϊ 127
    RTC_WaitForLastTask();

    // RTC ������ɺ����ѡ���������ʱ���趨�������Ҫ��
}

// �Զ��� RTC ���ü�����
void RTC_SetCounterCustom(uint32_t CounterValue) {
    RTC_SetCounter(CounterValue);
    RTC_WaitForLastTask();
}

// �Զ��� RTC ��ȡ������
uint32_t RTC_GetCounterCustom(void) {
    return RTC_GetCounter();
}


// RTC �Զ���ʱ�����ú���
void RTC_SetTime(RTC_TimeTypeDef *sTime) {
    uint32_t counter_time;

    // ת��ʱ��Ϊ����
    counter_time = sTime->Hours * 3600U + sTime->Minutes * 60U + sTime->Seconds;

    // ����ʱ��
    RTC_SetCounter(counter_time);
}

// RTC �Զ���ʱ���ȡ����
ErrorStatus RTC_GetTime(RTC_TimeTypeDef *sTime, uint8_t Format) {
    uint32_t counter_time;
    uint32_t hours;

    if (sTime == NULL) {
        return ERROR;
    }

    // ��ȡ RTC ����ֵ
    counter_time = RTC_GetCounter();

    // ����Сʱ�����ӡ���
    hours = counter_time / 3600U;
    sTime->Minutes = (counter_time % 3600U) / 60U;
    sTime->Seconds = (counter_time % 3600U) % 60U;
    sTime->Hours = hours % 24U;

    // �����Ҫת��Ϊ BCD ��ʽ
    if (Format == RTC_FORMAT_BCD) {
        sTime->Hours = ByteToBCD2(sTime->Hours);
        sTime->Minutes = ByteToBCD2(sTime->Minutes);
        sTime->Seconds = ByteToBCD2(sTime->Seconds);
    }

    return SUCCESS;
}

// RTC �Զ����������ú���
void RTC_SetDate(RTC_DateTypeDef *sDate) {
    uint32_t days = 0;

    // ����� 2000 �� 1 �� 1 ���������
    for (uint16_t year = 2000; year < sDate->Year; year++) {
        days += (RTC_IsLeapYear(year) ? 366 : 365);
    }

    for (uint8_t month = 1; month < sDate->Month; month++) {
        days += RTC_MonthDays[month - 1];
        if (month == 2 && RTC_IsLeapYear(sDate->Year)) {
            days += 1; // ���� 2 ��
        }
    }

    days += (sDate->Date - 1);

    // �������ڣ�ת��Ϊ������
    RTC_SetCounter(days * 86400);
}

// RTC �Զ������ڻ�ȡ����
ErrorStatus RTC_GetDate(RTC_DateTypeDef *sDate) {
    uint32_t counter_time = RTC_GetCounter();
    uint32_t days = counter_time / 86400;
    uint16_t year = 2000;

    if (sDate == NULL) {
        return ERROR;
    }

    // �������
    while (days >= (RTC_IsLeapYear(year) ? 366 : 365)) {
        days -= (RTC_IsLeapYear(year) ? 366 : 365);
        year++;
    }
    sDate->Year = year;

    // �����·�
    for (uint8_t month = 1; month <= 12; month++) {
        uint8_t month_days = RTC_MonthDays[month - 1];
        if (month == 2 && RTC_IsLeapYear(year)) {
            month_days += 1; // ���� 2 ��
        }

        if (days < month_days) {
            sDate->Month = month;
            sDate->Date = days + 1;
            break;
        }

        days -= month_days;
    }

    // ��������
    sDate->WeekDay = (counter_time / 86400 + 6) % 7; // 2000-01-01 ��������

    return SUCCESS;
}

