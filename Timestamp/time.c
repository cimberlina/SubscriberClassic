
#include "includes.h"




#define TIME_MAX                2147483647L
#define	ABB_LEN					3

int _daylight = 0;                  // Non-zero if daylight savings time is used
long _dstbias = 0;                  // Offset for Daylight Saving Time
long _timezone = 0;                 // Difference in seconds between GMT and local time
char *_tzname[2] = {"GMT", "GMT"};  // Standard/daylight savings time zone names

const char *_days[] = 
{
  "Sunday", "Monday", "Tuesday", "Wednesday",
  "Thursday", "Friday", "Saturday"
};

const char *_days_abbrev[] = 
{
  "Sun", "Mon", "Tue", "Wed", 
  "Thu", "Fri", "Sat"
};

const char *_months[] = 
{
  "January", "February", "March",
  "April", "May", "June",
  "July", "August", "September",
  "October", "November", "December"
};

const char *_months_abbrev[] = 
{
  "Jan", "Feb", "Mar",
  "Apr", "May", "Jun",
  "Jul", "Aug", "Sep",
  "Oct", "Nov", "Dec"
};

const int _ytab[2][12] = 
{
  {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
  {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};

struct tm *gmtime(const time_t *timer, struct tm *tmbuf)
{
  time_t time = *timer;
  unsigned long dayclock, dayno;
  int year = EPOCH_YR;

  dayclock = (unsigned long) time % SECS_DAY;
  dayno = (unsigned long) time / SECS_DAY;

  tmbuf->tm_sec = dayclock % 60;
  tmbuf->tm_min = (dayclock % 3600) / 60;
  tmbuf->tm_hour = dayclock / 3600;
  tmbuf->tm_wday = ((dayno + 4) % 7); // Day 0 was a thursday
  while (dayno >= (unsigned long) YEARSIZE(year)) 
  {
    dayno -= YEARSIZE(year);
    year++;
  }
  tmbuf->tm_year = year - YEAR0;
  tmbuf->tm_yday = dayno;
  tmbuf->tm_mon = 0;
  while (dayno >= (unsigned long) _ytab[LEAPYEAR(year)][tmbuf->tm_mon]) 
  {
    dayno -= _ytab[LEAPYEAR(year)][tmbuf->tm_mon];
    tmbuf->tm_mon++;
  }


  tmbuf->tm_mday = dayno + 1;
  tmbuf->tm_isdst = 0;
  tmbuf->tm_gmtoff = 0;
  tmbuf->tm_zone = "UTC";

  return tmbuf;
}



time_t mktime(struct tm *tmbuf)
{
  long day, year;
  int tm_year;
  int yday, month;
  time_t seconds;
  int overflow;



  tmbuf->tm_min += tmbuf->tm_sec / 60;
  tmbuf->tm_sec %= 60;
  if (tmbuf->tm_sec < 0) 
  {
    tmbuf->tm_sec += 60;
    tmbuf->tm_min--;
  }
  tmbuf->tm_hour += tmbuf->tm_min / 60;
  tmbuf->tm_min = tmbuf->tm_min % 60;
  if (tmbuf->tm_min < 0) 
  {
    tmbuf->tm_min += 60;
    tmbuf->tm_hour--;
  }
  day = tmbuf->tm_hour / 24;
  tmbuf->tm_hour= tmbuf->tm_hour % 24;
  if (tmbuf->tm_hour < 0) 
  {
    tmbuf->tm_hour += 24;
    day--;
  }
  tmbuf->tm_year += tmbuf->tm_mon / 12;
  tmbuf->tm_mon %= 12;
  if (tmbuf->tm_mon < 0) 
  {
    tmbuf->tm_mon += 12;
    tmbuf->tm_year--;
  }
  day += (tmbuf->tm_mday - 1);
  while (day < 0) 
  {
    if(--tmbuf->tm_mon < 0) 
    {
      tmbuf->tm_year--;
      tmbuf->tm_mon = 11;
    }
    day += _ytab[LEAPYEAR(YEAR0 + tmbuf->tm_year)][tmbuf->tm_mon];
  }

  while (day >= _ytab[LEAPYEAR(YEAR0 + tmbuf->tm_year)][tmbuf->tm_mon]) 
  {
    day -= _ytab[LEAPYEAR(YEAR0 + tmbuf->tm_year)][tmbuf->tm_mon];
    if (++(tmbuf->tm_mon) == 12) 
    {
      tmbuf->tm_mon = 0;
      tmbuf->tm_year++;
    }
  }
  tmbuf->tm_mday = day + 1;
  year = EPOCH_YR;

  if (tmbuf->tm_year < year - YEAR0)
	  return (time_t) -1;

  seconds = 0;
  day = 0;                      // Means days since day 0 now
  overflow = 0;

  // Assume that when day becomes negative, there will certainly
  // be overflow on seconds.
  // The check for overflow needs not to be done for leapyears
  // divisible by 400.
  // The code only works when year (1970) is not a leapyear.
  tm_year = tmbuf->tm_year + YEAR0;

  if (TIME_MAX / 365 < tm_year - year) overflow++;
  day = (tm_year - year) * 365;
  if (TIME_MAX - day < (tm_year - year) / 4 + 1) overflow++;
  day += (tm_year - year) / 4 + ((tm_year % 4) && tm_year % 4 < year % 4);
  day -= (tm_year - year) / 100 + ((tm_year % 100) && tm_year % 100 < year % 100);
  day += (tm_year - year) / 400 + ((tm_year % 400) && tm_year % 400 < year % 400);

  yday = month = 0;
  while (month < tmbuf->tm_mon)
  {
    yday += _ytab[LEAPYEAR(tm_year)][month];
    month++;
  }
  yday += (tmbuf->tm_mday - 1);
  if (day + yday < 0) overflow++;
  day += yday;

  tmbuf->tm_yday = yday;
  tmbuf->tm_wday = (day + 4) % 7;               // Day 0 was thursday (4)

  seconds = ((tmbuf->tm_hour * 60L) + tmbuf->tm_min) * 60L + tmbuf->tm_sec;

  if ((TIME_MAX - seconds) / SECS_DAY < day)
		overflow++;

  seconds += day * SECS_DAY;


  return (time_t) seconds;
}





///* free mktime function
//   Copyright 1988, 1989 by David MacKenzie <djm@ai.mit.edu>
//   and Michael Haertel <mike@ai.mit.edu>
//   Unlimited distribution permitted provided this copyright notice is
//   retained and any functional modifications are prominently identified.  */
//
///* Note: This version of mktime is ignorant of the tzfile; it does not
//   return correct results during the few hours around when daylight savings
//   time goes in to or out of effect.  It also does not allow or adjust
//   for invalid values in any of the fields, contrary to the ANSI C
//   specification. */
//
//#ifdef MKTIME_MISSING
//#include <sys/types.h>
//#include <time.h>
//
//time_t mkgmtime ();
//
///* Return the equivalent in seconds past 12:00:00 a.m. Jan 1, 1970 GMT
//   of the local time and date in the exploded time structure `tm',
//   and set `tm->tm_yday', `tm->tm_wday', and `tm->tm_isdst'.
//   Return -1 if any of the other fields in `tm' has an invalid value. */
//
//time_t
//mktime (tm)
//     struct tm *tm;
//{
//  struct tm save_tm;            /* Copy of contents of `*tm'. */
//  struct tm *ltm;               /* Local time. */
//  time_t then;                  /* The time to return. */
//
//  then = mkgmtime (tm);
//  if (then == -1)
//    return -1;
//
//  /* In case `tm' points to the static area used by localtime,
//     save its contents and restore them later. */
//  save_tm = *tm;
//  /* Correct for the timezone and any daylight savings time.
//     If a change to or from daylight savings time occurs between when
//     it is the time in `tm' locally and when it is that time in Greenwich,
//     the change to or from dst is ignored, but that is a rare case. */
//  then += then - mkgmtime (localtime (&then));
//
//  ltm = localtime (&then);
//  save_tm.tm_yday = ltm->tm_yday;
//  save_tm.tm_wday = ltm->tm_wday;
//  save_tm.tm_isdst = ltm->tm_isdst;
//  *tm = save_tm;
//
//  return then;
//}
//
///* Nonzero if `y' is a leap year, else zero. */
//#define leap(y) (((y) % 4 == 0 && (y) % 100 != 0) || (y) % 400 == 0)
//
///* Number of leap years from 1970 to `y' (not including `y' itself). */
//#define nleap(y) (((y) - 1969) / 4 - ((y) - 1901) / 100 + ((y) - 1601) / 400)
//
///* Number of days in each month of the year. */
//static char monlens[] =
//{
//  31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
//};
//
///* Return the equivalent in seconds past 12:00:00 a.m. Jan 1, 1970 GMT
//   of the Greenwich Mean time and date in the exploded time structure `tm',
//   and set `tm->tm_yday', `tm->tm_wday', and `tm->tm_isdst'.
//   Return -1 if any of the other fields in `tm' has an invalid value. */
//
//time_t
//mkgmtime (tm)
//     struct tm *tm;
//{
//  int years, months, days, hours, minutes, seconds;
//
//  years = tm->tm_year + 1900;   /* year - 1900 -> year */
//  months = tm->tm_mon;          /* 0..11 */
//  days = tm->tm_mday - 1;       /* 1..31 -> 0..30 */
//  hours = tm->tm_hour;          /* 0..23 */
//  minutes = tm->tm_min;         /* 0..59 */
//  seconds = tm->tm_sec;         /* 0..61 in ANSI C. */
//
//  if (years < 1970
//      || months < 0 || months > 11
//      || days < 0
//      || days > monlens[months] + (months == 1 && leap (years)) - 1
//      || hours < 0 || hours > 23
//      || minutes < 0 || minutes > 59
//      || seconds < 0 || seconds > 61)
//  return -1;
//
//  /* Set `days' to the number of days into the year. */
//  if (months > 1 && leap (years))
//    ++days;
//  while (months-- > 0)
//    days += monlens[months];
//  tm->tm_yday = days;
//
//  /* Now set `days' to the number of days since Jan 1, 1970. */
//  days += 365 * (years - 1970) + nleap (years);
//  tm->tm_wday = (days + 4) % 7; /* Jan 1, 1970 was Thursday. */
//  tm->tm_isdst = 0;
//
//  return 86400 * days + 3600 * hours + 60 * minutes + seconds;
//}
////#endif

#define DATE_STR        "??? ??? ?? ??:??:?? ????"

static char * two_digits(register char *pb, int i, int nospace)
{
        *pb = (i / 10) % 10 + '0';
        if (!nospace && *pb == '0') *pb = ' ';
        pb++;
        *pb++ = (i % 10) + '0';
        return ++pb;
}

static char * four_digits(register char *pb, int i)
{
        i %= 10000;
        *pb++ = (i / 1000) + '0';
        i %= 1000;
        *pb++ = (i / 100) + '0';
        i %= 100;
        *pb++ = (i / 10) + '0';
        *pb++ = (i % 10) + '0';
        return ++pb;
}

char *asctime(const struct tm *timeptr)
{
        static char buf[26];
        char *pb = buf;
        const char *ps;
        int n;

        strcpy(pb, DATE_STR);
        ps = _days[timeptr->tm_wday];
        n = ABB_LEN;
        while(--n >= 0) *pb++ = *ps++;
        pb++;
        ps = _months[timeptr->tm_mon];
        n = ABB_LEN;
        while(--n >= 0) *pb++ = *ps++;
        pb++;
        pb = two_digits(
                    two_digits(
                            two_digits(two_digits(pb, timeptr->tm_mday, 0)
                                        , timeptr->tm_hour, 1)
                            , timeptr->tm_min, 1)
                    , timeptr->tm_sec, 1);

        four_digits(pb, timeptr->tm_year + 1900);
        return buf;
}

//double difftime(time_t time1, time_t time0)
//{
//	if ((time_t)-1 > 0 && time0 > time1) {
//		return - (double) (time0 - time1);
//	} else {
//		return (double)(time1 - time0);
//	}
//}

struct tm *GetTime(struct tm *tmbuff)
{
	tmbuff->tm_sec = (int)RTC_GetTime(LPC_RTC, RTC_TIMETYPE_SECOND);
	tmbuff->tm_min = (int)RTC_GetTime(LPC_RTC, RTC_TIMETYPE_MINUTE);
	tmbuff->tm_hour = (int)RTC_GetTime(LPC_RTC, RTC_TIMETYPE_HOUR);
	tmbuff->tm_mon = (int)RTC_GetTime(LPC_RTC, RTC_TIMETYPE_MONTH) - 1;
	tmbuff->tm_year = (int)RTC_GetTime(LPC_RTC, RTC_TIMETYPE_YEAR);
	tmbuff->tm_mday = (int)RTC_GetTime(LPC_RTC, RTC_TIMETYPE_DAYOFMONTH);
	tmbuff->tm_wday = (int)RTC_GetTime(LPC_RTC, RTC_TIMETYPE_DAYOFWEEK);
	tmbuff->tm_yday = (int)RTC_GetTime(LPC_RTC, RTC_TIMETYPE_DAYOFYEAR) - 1;

	SEC_TIMER = mktime(tmbuff);
    fsm_wdog_evo( 99, 0 );

	return tmbuff;
}


