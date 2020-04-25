/* 
 *
 * Citing Sources: 
 *
 * https://www.linuxtopia.org/online_books/programming_books/gnu_c_programming_tutorial/argp-example.html 
 * 
 */

#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <stdint.h>



typedef struct file_id_ymd{
    unsigned int year : 8;
    unsigned int month: 4;
    unsigned int day  : 5;
} file_id_ymd_t;

typedef struct file_id_sec{
    unsigned int sec : 6;
} file_id_sec_t;


int packet_id (void)
{

    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char s[64];

    assert(strftime(s, sizeof(s), "%c", tm));
    printf("%s\n", s);
    printf("%d_%d_%d %d:%d:%d \n", (*tm).tm_year-100, (*tm).tm_mon, (*tm).tm_mday, (*tm).tm_hour, (*tm).tm_min, (*tm).tm_sec );
 

    //YMD:
    file_id_ymd_t my_time;   
    my_time.year  = (*tm).tm_year-100;
    my_time.month = (*tm).tm_mon;
    my_time.day   = (*tm).tm_mday;

    int file_id_ymd_var = my_time.year + my_time.month + my_time.day;

#ifdef DEBUG
    printf("my_time.year=%d\nmy_time.month=%d\nmy_time.day=%d\n\n", \
            my_time.year,  \
            my_time.month, \
            my_time.day    );
    printf("my_time.year=%02x\nmy_time.month=%01x\nmy_time.day=%d\n\n", \
            my_time.year,  \
            my_time.month, \
            my_time.day    );
    printf("file_id_ymd=%d\n", file_id_ymd_var);
    printf("file_id_ymd=0x%04x\n\n", file_id_ymd_var);
#endif

    //SEC:
    file_id_sec_t file_id_sec_var;
    file_id_sec_var.sec = (*tm).tm_sec;

#ifdef DEBUG
    printf("file_id_sec=%d\n", file_id_sec_var.sec);
    printf("file_id_sec=0x%04x\n\n", file_id_sec_var.sec);
    printf("file_id_ymd_var=0x%04x\n\n", file_id_ymd_var);
#endif
 
//    uint16_t file_id_var = ( (my_time.year + my_time.month + my_time.day) << 6 ) | file_id_sec_var.sec ;
//    uint16_t file_id_var = ( (my_time.year + my_time.month + my_time.day) << 6+5+6 ) | (*tm).tm_hour << 5+6 | (*tm).tm_min << 6 | file_id_sec_var.sec ;
    uint16_t file_id_var = ( (my_time.year + my_time.month + my_time.day) << 6+5+6 ) | (*tm).tm_hour << 5+6 | (*tm).tm_min << 6 | 0 ;

    printf("file_id_var=0x%d\n", file_id_var);
    printf("file_id_var=0x%04x\n\n", file_id_var);
    
    return file_id_var;
}
