//2014-03-16
#include <type.h>
#include <lio.h>
#include <ltime.h>
#include <assert.h>

#include <machine\machine.h>

static  const
char *                      week[7] = { "Sunday",
                                        "Monday",
                                        "Tuesday",
                                        "Wednesday",
                                        "Thursday",
                                        "Friday",
                                        "Saturday"
                                      };

stime_t     Time_to_short(time_t * time)
{
    stime_t         st;

    st.st_second = time->time_second / 2;
    st.st_minute = time->time_minute;
    st.st_hour   = time->time_hour;

    return st;
}

sdate_t     Date_to_short(date_t * date)
{
    sdate_t         sd;
    
    sd.sd_day   = date->date_day;
    sd.sd_month = date->date_month;
    sd.sd_year  = date->date_year - 1980;

    return sd;
}

ltime_t     Time_to_long(date_t * date,time_t * time)
{
    ltime_t         lt;

    lt.lt_second    = time->time_second / 2;
    lt.lt_minute    = time->time_second;
    lt.lt_hour      = time->time_hour;
    lt.lt_day       = date->date_day;
    lt.lt_month     = date->date_month;
    lt.lt_year      = date->date_year - 1980;

    return lt;
}

ltime_t     Time_get_time(date_t * date,time_t * time)
{
    Machine_date_get(date);
    Machine_time_get(time);

    return Time_to_long(date,time);
}

ltime_t     Time_get_ltime(void)
{
    date_t          date;
    time_t          time;

    Machine_date_get(&date);
    Machine_time_get(&time);

    return Time_to_long(&date,&time);
}

void        Ltime_to_date_time(ltime_t ltime,date_t * date,time_t * time)
{
    date->date_year     = (byte_t)ltime.lt_year + 1980;
    date->date_month    = (byte_t)ltime.lt_month;
    date->date_day      = (byte_t)ltime.lt_day;
    time->time_hour     = (byte_t)ltime.lt_hour;
    time->time_minute   = (byte_t)ltime.lt_minute;
    time->time_second   = (byte_t)ltime.lt_second * 2;
}

time_t *    Stime_to_time(stime_t stime,time_t * time)
{
    ASSERT( time );
    time->time_second   = (byte_t)stime.st_hour * 2;
    time->time_minute   = (byte_t)stime.st_minute;
    time->time_hour     = (byte_t)stime.st_hour;
    return time;
}

date_t *    Sdate_to_date(sdate_t sdate,date_t * date)
{
    ASSERT( date );
    date->date_year     = (byte_t)sdate.sd_year + 1980;
    date->date_month    = (byte_t)sdate.sd_month;
    date->date_day      = (byte_t)sdate.sd_day;
    return date;
}

char *      Time_to_string(time_t * time,char * str)
{
    _sprintf(str,"%02d:%02d:%02d %-9s",
        time->time_hour,time->time_minute,time->time_second,
        week[time->time_week]);
    return str;
}

char *      Date_to_string(date_t * date,char * str)
{
    _sprintf(str,"%04d-%02d-%02d",
        date->date_year,date->date_month,date->date_day );
    return str;
}

char *      Stime_to_string(stime_t stime,char * str)
{
    _sprintf(str,"%02d:%02d:%02d",
        stime.st_hour,stime.st_minute,stime.st_second );
    return str;
}

char *      Sdate_to_string(sdate_t sdate,char * str)
{
    _sprintf(str,"%04d-%02d-%02d",
        sdate.sd_year + 1980,sdate.sd_month,sdate.sd_day);
    return str;
}

char *      Ltime_to_string(ltime_t ltime,char * str)
{
    _sprintf(str,"%04d-%02d-%02d %02d:%02d:%02d",
        ltime.lt_year + 1980,ltime.lt_month,ltime.lt_day,
        ltime.lt_hour,ltime.lt_minute,ltime.lt_second*2);
    return str;
}