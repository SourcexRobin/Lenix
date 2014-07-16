

#include <type.h>
#include <lio.h>
/*
 *  短格式时间转换为长格式时间
 *  sd: short date
 *  st: short time
 */
#define TIME_PRINTF(time)        do{ char _t_str_[36] ; \
                                     _printf("%s\n", \
                                     Time_to_string(time,_t_str_));\
                                   }while(0)
#define TIME_PRINTK(time)        do{ char _t_str_[36] ; \
                                     _printk("%s\n", \
                                     Time_to_string(time,_t_str_));\
                                   }while(0)
#define LTIME_PRINTF(ltime)     do{ char _t_str_[36]; \
                                     _printf("%s\n", \
                                     Ltime_to_string(ltime,_t_str_));\
                                   }while(0)
#define LTIME_PRINTK(ltime)     do{ char _t_str_[36]; \
                                     _printk("%s\n", \
                                     Ltime_to_string(ltime,_t_str_));\
                                   }while(0)


stime_t     Time_to_short(time_t * time);
sdate_t     Date_to_short(date_t * date);
ltime_t     Time_to_long(date_t * date,time_t * time);
ltime_t     Time_get_time(date_t * date,time_t * time);
ltime_t     Time_get_ltime(void);
void        Ltime_to_date_time(ltime_t ltime,date_t * date,time_t * time);
time_t *    Stime_to_time(stime_t stime,time_t * time);
date_t *    Sdate_to_date(sdate_t sdate,date_t * date);

char *      Time_to_string(time_t * time,char * str);
char *      Date_to_string(date_t * date,char * str);
char *      Stime_to_string(stime_t stime,char * str);
char *      Sdate_to_string(sdate_t sdate,char * str);
char *      Ltime_to_string(ltime_t ltime,char * str);