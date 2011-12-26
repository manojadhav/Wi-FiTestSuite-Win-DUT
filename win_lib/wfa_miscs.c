/****************************************************************************
 *  (c) Copyright 2007 Wi-Fi Alliance.  All Rights Reserved
 *
 *
 *  LICENSE
 *
 *  License is granted only to Wi-Fi Alliance members and designated
 *  contractors ($B!H(BAuthorized Licensees$B!I(B)..AN  Authorized Licensees are granted
 *  the non-exclusive, worldwide, limited right to use, copy, import, export
 *  and distribute this software:
 *  (i) solely for noncommercial applications and solely for testing Wi-Fi
 *  equipment; and
 *  (ii) solely for the purpose of embedding the software into Authorized
 *  Licensee$B!G(Bs proprietary equipment and software products for distribution to
 *  its customers under a license with at least the same restrictions as
 *  contained in this License, including, without limitation, the disclaimer of
 *  warranty and limitation of liability, below..AN  The distribution rights
 *  granted in clause
 *  (ii), above, include distribution to third party companies who will
 *  redistribute the Authorized Licensee$B!G(Bs product to their customers with or
 *  without such third party$B!G(Bs private label. Other than expressly granted
 *  herein, this License is not transferable or sublicensable, and it does not
 *  extend to and may not be used with non-Wi-Fi applications..AN  Wi-Fi Alliance
 *  reserves all rights not expressly granted herein..AN 
 *.AN 
 *  Except as specifically set forth above, commercial derivative works of
 *  this software or applications that use the Wi-Fi scripts generated by this
 *  software are NOT AUTHORIZED without specific prior written permission from
 *  Wi-Fi Alliance.
 *.AN 
 *  Non-Commercial derivative works of this software for internal use are
 *  authorized and are limited by the same restrictions; provided, however,
 *  that the Authorized Licensee shall provide Wi-Fi Alliance with a copy of
 *  such derivative works under a perpetual, payment-free license to use,
 *  modify, and distribute such derivative works for purposes of testing Wi-Fi
 *  equipment.
 *.AN 
 *  Neither the name of the author nor "Wi-Fi Alliance" may be used to endorse
 *  or promote products that are derived from or that use this software without
 *  specific prior written permission from Wi-Fi Alliance.
 *
 *  THIS SOFTWARE IS PROVIDED BY WI-FI ALLIANCE "AS IS" AND ANY EXPRESS OR
 *  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 *  OF MERCHANTABILITY, NON-INFRINGEMENT AND FITNESS FOR A.AN PARTICULAR PURPOSE,
 *  ARE DISCLAIMED. IN NO EVENT SHALL WI-FI ALLIANCE BE LIABLE FOR ANY DIRECT,
 *  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, THE COST OF PROCUREMENT OF SUBSTITUTE
 *  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 *  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE) ARISING IN ANY WAY OUT OF
 *  THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. ******************************************************************************
 */

/*
 *   Revision History: WINv03.00.00 - Simga 3.0 Release, supports TGn Program including WMM and WPA2
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WINDOWS
#include <winsock.h>
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#ifdef _CYGWIN
#include <cygwin/if.h>
#else
#include <linux/if.h>
#endif
#include <sys/ioctl.h>
#endif
#include "wfa_debug.h"
#include "wfa_main.h"
#include "wfa_types.h"
#include "wfa_tg.h"
#ifdef _WINDOWS
#include < time.h >
#include <windows.h> //I've ommited this line.
#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif
extern int gettimeofday(struct timeval *tv, void *tz);
static double counterPerMicrosecond = -1.0;            /* GLOBAL */
static unsigned __int64 frequency = 0;                 /* GLOBAL */
static unsigned __int64 timeSecOffset = 0;             /* GLOBAL */
static unsigned __int64 startPerformanceCounter = 0;   /* GLOBAL */ 
struct timezone 
{
  int  tz_minuteswest; /* minutes W of Greenwich */
  int  tz_dsttime;     /* type of dst correction */
};

#endif
extern tgStream_t *theStreams;

tgStream_t *findStreamProfile(int id);
#if 1
int gettimeofday1(struct timeval *tv, struct timezone *tz)
{
  FILETIME ft;
  unsigned __int64 tmpres = 0;
  static int tzflag;
 
  if (NULL != tv)
  {
    GetSystemTimeAsFileTime(&ft);
 
    tmpres |= ft.dwHighDateTime;
    tmpres <<= 32;
    tmpres |= ft.dwLowDateTime;
 
    /*converting file time to unix epoch*/
    tmpres -= DELTA_EPOCH_IN_MICROSECS; 
    tmpres /= 10;  /*convert into microseconds*/
    tv->tv_sec = (long)(tmpres / 1000000UL);
    tv->tv_usec = (long)(tmpres % 1000000UL);
  }
 
  if (NULL != tz)
  {
    if (!tzflag)
    {
      _tzset();
      tzflag++;
    }
    tz->tz_minuteswest = _timezone / 60;
    tz->tz_dsttime = _daylight;
  }
 
  return 0;
}
#endif
/*
 * printProfile(): a debugging function to display a profile info based on
 *                 a streamId
 */

void printProfile(tgProfile_t *pf)
{
    printf("profile type %i direction %i Dest ipAddr %s Dest port %i So ipAddr %s So port %i rate %i duration %i pksize %i\n", pf->profile, pf->direction, pf->dipaddr, pf->dport, pf->sipaddr, pf->sport, pf->rate, pf->duration, pf->pksize);
}

int isString(char *str)
{
    if(*str == '\0')
       return FALSE;

    if((str[0] >= 'a' && str[0] <= 'z') 
          || (str[0] > 'A' && str[0] < 'Z'))
       return TRUE;
    else
       return FALSE;

}

int isNumber(char *str)
{
    if(*str == '\0')
       return FALSE;

    if (str[0] >= '0' && str[0] <= '9')
       return TRUE;
    else
       return FALSE;
}

int isIpV4Addr(char *str)
{
    int dots = 0;
    char *tmpstr = str;

    if(*str == '\0')
       return FALSE;

    while(*tmpstr != '\0')
    {
      if(*tmpstr == '.')
      {
         dots++;
      }

      tmpstr++;
    }

    if(dots <3)
      return FALSE;
    else
      return TRUE;
}

 double wfa_timeval2double(struct timeval *tval)
{
    return ((double) tval->tv_sec + (double) tval->tv_usec*1e-6);
}

 void wfa_double2timeval(struct timeval *tval, double dval)
{
    tval->tv_sec = (long int) dval;
    tval->tv_usec = (long int) ((dval - tval->tv_sec) * 1000000);
}

 double wfa_ftime_diff(struct timeval *t1, struct timeval *t2)
{
   double dtime;

   dtime = wfa_timeval2double(t2) - wfa_timeval2double(t1);
   return dtime ;
}

int wfa_itime_diff(struct timeval *t1, struct timeval *t2)
{
   int dtime;
   int sec = t2->tv_sec - t1->tv_sec;
   int usec = t2->tv_usec - t1->tv_usec;

   if(usec < 0)
   {
       sec -=1;
       usec += 1000000;
   }

   dtime = sec*1000000 + usec;
   return dtime;
}

/*
 * THe following two functions are converting Little Endian to Big Endian. 
 * If your machine is already a Big Endian, you may flag it out.
 */
 void int2BuffBigEndian(int val, char *buf)
{
   char *littleEn = (char *)&val;

   buf[0] = littleEn[3];
   buf[1] = littleEn[2];
   buf[2] = littleEn[1];
   buf[3] = littleEn[0];
}

int bigEndianBuff2Int(char *buff)
{
   int val;
   char *strval = (char *)&val;

   strval[0] = buff[3];
   strval[1] = buff[2];
   strval[2] = buff[1];
   strval[3] = buff[0];

   return val;
}

int wfa_estimate_timer_latency()
{
    struct timeval t1, t2, tp2;
    int sleep=20; /* two miniseconds */
    int latency =0;

    gettimeofday(&t1, NULL);

#ifdef _WINDOWS
    Sleep(sleep);
#else

    usleep(sleep*1000);
#endif

    gettimeofday(&t2, NULL); 

    tp2.tv_usec = t1.tv_usec + 20000;
    if( tp2.tv_usec >= 1000000)
    {
        tp2.tv_sec = t1.tv_sec +1;
	tp2.tv_usec -= 1000000;
    }
    else
        tp2.tv_sec = t1.tv_sec;
printf("before sec %i, usec %i sleep %i after sec %i usec %i\n",t1.tv_sec,t1.tv_usec,t2.tv_sec,t2.tv_usec);
    return latency = (t2.tv_sec - tp2.tv_sec) * 1000000 + (t2.tv_usec - tp2.tv_usec); 
}
#ifdef _WINDOWS


/*
 * gettimeofday for windows
 *
 * CounterPerMicrosecond is the number of counts per microsecond.
 * Double is required if we have less than 1 counter per microsecond.  
 * On a PIII 700, I get about 3.579545.  This is guaranteed not to change while the processor is running.
 * We really don't need to check for loop detection.  On my machine it would take about 59645564 days to loop.
 * (2^64) / frequency / 60 / 60 / 24.
 *
 */
int
gettimeofday(struct timeval *tv, void *tz)
{
  unsigned __int64 counter;
  //HANDLE L_GETTIMEOFDAY;
  QueryPerformanceCounter((LARGE_INTEGER *) &counter);

  if (counter < startPerformanceCounter || counterPerMicrosecond == -1.0)
    {
      time_t t;

      QueryPerformanceFrequency((LARGE_INTEGER *) &frequency);

      counterPerMicrosecond = (double) ((__int64) frequency) / 1000000.0f;

      time(&t);
      QueryPerformanceCounter((LARGE_INTEGER *) &counter);
      startPerformanceCounter = counter;

      counter /= frequency;

      timeSecOffset = t - counter;

    //  ReleaseMutex (L_GETTIMEOFDAY);
      QueryPerformanceCounter((LARGE_INTEGER *) &counter);
    }

  tv->tv_sec = ((long) counter / (long) frequency) + (long) timeSecOffset;
  tv->tv_usec = ((__int64) (((__int64) counter) / counterPerMicrosecond) % 1000000);

  return 0;
}
int settimeofday(struct timeval *tv,void *tz)
     {
       SYSTEMTIME st;
       struct tm *gmtm;
       long x = tv->tv_sec;
       long y = tv->tv_usec;
     
       gmtm = gmtime((const time_t *) &x);
       st.wSecond		= (WORD) gmtm->tm_sec;
       st.wMinute		= (WORD) gmtm->tm_min;
       st.wHour			= (WORD) gmtm->tm_hour;
       st.wDay			= (WORD) gmtm->tm_mday;
       st.wMonth			= (WORD) (gmtm->tm_mon  + 1);
       st.wYear			= (WORD) (gmtm->tm_year + 1900);
       st.wDayOfWeek		= (WORD) gmtm->tm_wday;
       st.wMilliseconds	= (WORD) (y / 1000);
	   if (!SetSystemTime(&st))
	   {
			printf("SetSystemTime failed:\n");
			return -1;
       }
     
       
       return 0;
    }
/* strtok_r version of Windows */
char * strtok_r(char *string, const char *sepset, char **lasts)
{
        char    *q, *r;

        /* first or subsequent call */
        if (string == NULL)
                string = *lasts;

        if (string == NULL)             /* return if no tokens remaining */
                return (NULL);

        q = string + strspn(string, sepset);    /* skip leading separators */

        if (*q == '\0')         /* return if no tokens remaining */
                return (NULL);

        if ((r = strpbrk(q, sepset)) == NULL)   /* move past token */
                *lasts = NULL;  /* indicate this is last token */
        else {
                *r = '\0';
                *lasts = r + 1;
        }
        return (q);
}
/* Windows version of strncasecmp */
int strncasecmp(const char *s1, const char *s2, size_t n)
{
    while(n > 0
          && toupper((unsigned char)*s1) == toupper((unsigned char)*s2))
    {
        if(*s1 == '\0')
            return 0;
        s1++;
        s2++;
        n--;
    }
    if(n == 0)
        return 0;
    return toupper((unsigned char)*s1) - toupper((unsigned char)*s2);
}
/* Windows version of strcasecmp */
int strcasecmp(const char *s1, const char *s2)
{
    while(toupper((unsigned char)*s1) == toupper((unsigned char)*s2)) {
        if(*s1 == '\0')
            return 0;
        s1++;
        s2++;
    }
    return toupper((unsigned char)*s1) - toupper((unsigned char)*s2);
}


#endif
