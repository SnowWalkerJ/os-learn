#ifndef TIME_H
#define TIME_H

typedef struct {
    int year, month, day, hour, minute, second;
} time_t;

typedef struct {
    int dday, dhr, dmin, dsec;
} time_delta_t;

int to_timestamp(time_t);
time_t from_timestamp(int);
time_t time();
char* strftime(time_t, char*);
time_t strptime(char*, char*);

#endif