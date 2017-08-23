#ifndef __RTC_H
#define __RTC_H


#define FEBRUARY		2
#define	STARTOFTIME		1970
#define SECDAY			86400L
#define SECYR			(SECDAY * 365)
#define	leapyear(year)		((year) % 4 == 0)
#define	days_in_year(a) 	(leapyear(a) ? 366 : 365)
#define	days_in_month(a) 	(month_days[(a) - 1])

struct rtc_time {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
};

void RTC_Configuration(void);
void Time_Regulate(struct rtc_time *tm);
void Time_Adjust(void);
void RTC_BKP_Config(void);
void SYSCLKConfig_STOP(void);
void GregorianDay(struct rtc_time * tm);
u32 mktimev(struct rtc_time *tm);
void to_tm(u32 tim, struct rtc_time * tm);


#endif

