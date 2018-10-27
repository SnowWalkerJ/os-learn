#include "time.h"
#include <stddef.h>
#include <drivers/rtc.h>
#include <kernel/memory.h>
#include <libs/string.h>
#include <libs/stdlib.h>
#define days_in_year(year) (is_leap_year(year) ? 366 : 365)

int days_in_month(int, int);
int is_leap_year(int);
int add_with_carry(int, int, int, int*);
time_t _get_time_from_rtc();

time_t time() {
    time_t last_time = _get_time_from_rtc(), this_time;
    while (1) {
        this_time = _get_time_from_rtc();
        if (this_time.second == last_time.second &&
            this_time.minute == last_time.minute &&
            this_time.hour   == last_time.hour &&
            this_time.day    == last_time.day) {
                break;
            }
        last_time = this_time;
    }
    return this_time;
}

char* strftime(time_t tm, char* format) {
    size_t len0 = strlen(format);
    char* result = (char*)malloc(len0+20);
    int state = 0;
    int j = 0;
    for (int i = 0; i < (int)len0; i++) {
        if (state == 0) {
            if (format[i] != '%') {
                result[j++] = format[i];
            } else {
                state = 1;
            }
        } else {
            int n;
            switch (format[i]) {
            case 'Y':
                n = tm.year;
                break;
            case 'm':
                n = tm.month;
                break;
            case 'd':
                n = tm.day;
                break;
            case 'H':
                n = tm.hour;
                break;
            case 'M':
                n = tm.minute;
                break;
            case 'S':
                n = tm.second;
                break;
            default:
                result[j++] = '%';
                state = 0;
                i--;
            }
            char* tmp = int_to_oct(n, 2);
            size_t len = strlen(tmp);
            memcpy(tmp, result+j, len);
            free(tmp);
            j += len;
            state = 0;
        }
    }
    result[j] = 0;
    return result;
}

time_t strptime(char* content, char* format) {
    size_t len0 = strlen(format);
    time_t tm;
    int state = 0;
    size_t ci, fi;
    for (fi = ci = 0; fi < len0; fi++) {
        char fch = format[fi], cch = content[ci];
        if (state == 0) {
            if (fch == '%') {
                state = 1;
                continue;
            } else {
                ci++;
            }
        } else {
            switch (fch) {
            case 'Y':
                tm.year = (content[ci] - '0') * 1000 +
                          (content[ci+1] - '0') * 100 +
                          (content[ci+2] - '0') * 10 +
                          (content[ci+3] - '0')
                ci += 4;
                break;
            case 'm':
                tm.month = (content[ci] - '0') * 10 +
                           (content[ci+1] - '0')
                ci += 2;
                break;
            case 'd':
                tm.day = (content[ci] - '0') * 10 +
                         (content[ci+1] - '0')
                ci += 2;
                break;
            case 'H':
                tm.hour = (content[ci] - '0') * 10 +
                          (content[ci+1] - '0')
                ci += 2;
                break;
            case 'M':
                tm.minute = (content[ci] - '0') * 10 +
                            (content[ci+1] - '0')
                ci += 2;
                break;
            case 'S':
                tm.second = (content[ci] - '0') * 10 +
                            (content[ci+1] - '0')
                ci += 2;
                break;
            }
        }
    }
    return tm;
}

time_t change_time(time_t base, time_delta_t delta) {
    int carry;
    base.second = add_with_carry(base.second, delta.dsec        , 60, &carry);
    base.minute = add_with_carry(base.minute, delta.dmin + carry, 60, &carry);
    base.hour   = add_with_carry(base.hour  , delta.dhr  + carry, 24, &carry);
    delta.dday += carry;
    int leap = is_leap_year(base.year);

    int month;
    delta.dday += base.day - 1;
    base.day = 1;
    for (month = base.month - 1; month > 0; month--) {
        delta.dday += days_in_month(leap, month);
    }
    base.month = 1;
    
    for (; ;) {
        int d_in_y = days_in_year(base.year);
        if (delta.dday < d_in_y) break;
        delta.dday -= d_in_y;
        base.year++;
    }
    leap = is_leap_year(base.year);
    for (; ;) {
        int d_in_m = days_in_month(leap, base.month);
        if (delta.dday < d_in_m) break;
        delta.dday -= d_in_m;
        month++;
    }
    base.day += delta.dday;
    return base;
}

int to_timestamp(time_t tm) {
    time_t base = {1970, 1, 1, 0, 0, 0};
    int days = 0, year, month;
    for (year = base.year; year < tm.year; year++) {
        days += days_in_year(year);
    }
    int leap = is_leap_year(year);
    for (month = 1; month < tm.month; month++) {
        days += days_in_month(leap, month);
    }
    days += tm.day - 1;
    int timestamp = ((days * 24 + tm.hour) * 60 + tm.minute) * 60 + tm.second;
    return timestamp;
}

time_t from_timestamp(int timestamp) {
    time_t base = {1970, 1, 1, 0, 0, 0};
    time_delta_t delta = {0, 0, 0, timestamp};
    return change_time(base, delta);
}

int days_in_month(int leap, int month) {
    int days = 0;
    switch (month) {
    case 1:
        days = 31;
        break;
    case 2:
        days = leap ? 29 : 28;
        break;
    case 3:
        days = 31;
        break;
    case 4:
        days = 30;
        break;
    case 5:
        days = 31;
        break;
    case 6:
        days = 30;
        break;
    case 7:
        days = 31;
        break;
    case 8:
        days = 31;
        break;
    case 9:
        days = 30;
        break;
    case 10:
        days = 31;
        break;
    case 11:
        days = 30;
        break;
    case 12:
        days = 31;
        break;
    }
    return days;
}

inline int is_leap_year(int year) {
    return (year % 100 == 0) ? (year % 400 == 0) : (year % 4 == 0);
}

int add_with_carry(int val1, int val2, int base, int* carry) {
    int result = val1 + val2;
    *carry = result / base;
    return result % base;
}

time_t _get_time_from_rtc() {
    time_t tm = {
        .second = BCD_TO_BIN(read_rtc_register(0)),
        .minute = BCD_TO_BIN(read_rtc_register(2)),
        .hour   = BCD_TO_BIN(read_rtc_register(4)),
        .day    = BCD_TO_BIN(read_rtc_register(7)),
        .month  = BCD_TO_BIN(read_rtc_register(8)),
        .year   = BCD_TO_BIN(read_rtc_register(9)) + BCD_TO_BIN(read_rtc_register(0x32))*100,
    };
    return tm;
}