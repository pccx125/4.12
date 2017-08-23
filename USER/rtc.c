#include"main.h"

static int month_days[12] = {	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

__IO uint32_t TimeDisplay = 0;
struct rtc_time systmtime;

/**
  * @brief  Configures the RTC.
  * @param  None
  * @retval None
  */
void RTC_Configuration(void)
{
			/* Reset Backup Domain */
			BKP_DeInit();
			/* Enable LSE */
			RCC_LSEConfig(RCC_LSE_ON);
			/* Wait till LSE is ready */
			while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
			{}
			/* Select LSE as RTC Clock Source */
			RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
			/* Enable RTC Clock */
			RCC_RTCCLKCmd(ENABLE);
			/* Wait for RTC registers synchronization */
			RTC_WaitForSynchro();
			/* Wait until last write operation on RTC registers has finished */
			RTC_WaitForLastTask();

// 			/* Enable the RTC Second */
//  			RTC_ITConfig(RTC_IT_SEC, ENABLE);
			/* Enable the RTC Alarm interrupt */
      RTC_ITConfig(RTC_IT_ALR, ENABLE);
			/* Wait until last write operation on RTC registers has finished */
			RTC_WaitForLastTask();

			/* Set RTC prescaler: set RTC period to 1sec */
			RTC_SetPrescaler(32767); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */
			/* Set RTC prescaler: set RTC period to 0.05sec */
// 			RTC_SetPrescaler(1637); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(1637+1) */
			/* Wait until last write operation on RTC registers has finished */
			RTC_WaitForLastTask();
}
void Time_Regulate(struct rtc_time *tm)
{
	tm->tm_year=2016;
	tm->tm_mon=3;
	tm->tm_mday=19;
	tm->tm_hour=15;
	tm->tm_min=1;
	tm->tm_sec=0;	
}
void Time_Adjust(void)//设置时间信息
{
  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();
  /* Get time entred by the user on the hyperterminal */
  Time_Regulate(&systmtime);
  /* Get wday */
  GregorianDay(&systmtime);
  /* Change the current time */
  RTC_SetCounter(mktimev(&systmtime));
  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();
}

void RTC_BKP_Config(void)//RTC初始化
{
// 	  /* Enable PWR and BKP clocks */
//   RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

//   /* Allow access to BKP Domain */
//   PWR_BackupAccessCmd(ENABLE);
		
	/* RTC Configuration */
	RTC_Configuration();
		
  /* Adjust time by users typed on the hyperterminal */
	Time_Adjust();
	  /* Clear reset flags */
	RCC_ClearFlag();
}
void SYSCLKConfig_STOP(void)//从stop模式恢复后，使能时钟
{
  /* Enable HSE */
  RCC_HSEConfig(RCC_HSE_ON);

  /* Wait till HSE is ready */
  HSEStartUpStatus = RCC_WaitForHSEStartUp();

  if(HSEStartUpStatus == SUCCESS)
  {
    /* Enable PLL */ 
    RCC_PLLCmd(ENABLE);
    /* Wait till PLL is ready */
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
    {
    }
    /* Select PLL as system clock source */
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
    /* Wait till PLL is used as system clock source */
    while(RCC_GetSYSCLKSource() != 0x08)
    {
    }
  }
}

void GregorianDay(struct rtc_time * tm)
{
	int leapsToDate;
	int lastYear;
	int day;
	int MonthOffset[] = { 0,31,59,90,120,151,181,212,243,273,304,334 };

	lastYear=tm->tm_year-1;

	/*
	 * Number of leap corrections to apply up to end of last year
	 */
	leapsToDate = lastYear/4 - lastYear/100 + lastYear/400;

	/*
	 * This year is a leap year if it is divisible by 4 except when it is
	 * divisible by 100 unless it is divisible by 400
	 *
	 * e.g. 1904 was a leap year, 1900 was not, 1996 is, and 2000 will be
	 */
	if((tm->tm_year%4==0) &&
	   ((tm->tm_year%100!=0) || (tm->tm_year%400==0)) &&
	   (tm->tm_mon>2)) {
		/*
		 * We are past Feb. 29 in a leap year
		 */
		day=1;
	} else {
		day=0;
	}

	day += lastYear*365 + leapsToDate + MonthOffset[tm->tm_mon-1] + tm->tm_mday;

	tm->tm_wday=day%7;
}

/* Converts Gregorian date to seconds since 1970-01-01 00:00:00.
 * Assumes input in normal date format, i.e. 1980-12-31 23:59:59
 * => year=1980, mon=12, day=31, hour=23, min=59, sec=59.
 *
 * [For the Julian calendar (which was used in Russia before 1917,
 * Britain & colonies before 1752, anywhere else before 1582,
 * and is still in use by some communities) leave out the
 * -year/100+year/400 terms, and add 10.]
 *
 * This algorithm was first published by Gauss (I think).
 *
 * WARNING: this function will overflow on 2106-02-07 06:28:16 on
 * machines were long is 32-bit! (However, as time_t is signed, we
 * will already get problems at other places on 2038-01-19 03:14:08)
 */
u32 mktimev(struct rtc_time *tm)//把时间信息转换为32位的数，相当于秒
{
	if (0 >= (int) (tm->tm_mon -= 2)) {	/* 1..12 -> 11,12,1..10 */
		tm->tm_mon += 12;		/* Puts Feb last since it has leap day */
		tm->tm_year -= 1;
	}

	return (((
		(u32) (tm->tm_year/4 - tm->tm_year/100 + tm->tm_year/400 + 367*tm->tm_mon/12 + tm->tm_mday) +
			tm->tm_year*365 - 719499
	    )*24 + tm->tm_hour /* now have hours */
	  )*60 + tm->tm_min /* now have minutes */
	)*60 + tm->tm_sec; /* finally seconds */
}

void to_tm(u32 tim, struct rtc_time * tm)//把32位的数转换成用时间信息表示
{
	register u32    i;
	register long   hms, day;

	day = tim / SECDAY;
	hms = tim % SECDAY;

	/* Hours, minutes, seconds are easy */
	tm->tm_hour = hms / 3600;
	tm->tm_min = (hms % 3600) / 60;
	tm->tm_sec = (hms % 3600) % 60;

	/* Number of years in days */
	for (i = STARTOFTIME; day >= days_in_year(i); i++) {
		day -= days_in_year(i);
	}
	tm->tm_year = i;

	/* Number of months in days left */
	if (leapyear(tm->tm_year)) {
		days_in_month(FEBRUARY) = 29;
	}
	for (i = 1; day >= days_in_month(i); i++) {
		day -= days_in_month(i);
	}
	days_in_month(FEBRUARY) = 28;
	tm->tm_mon = i;

	/* Days are what is left over (+1) from all that. */
	tm->tm_mday = day + 1;

	/*
	 * Determine the day of week
	 */
	GregorianDay(tm);
}




