#ifndef ASSERT_H
#define ASSERT_H
#include <kernel/console.h>

#define assert(condition, msg) if(!(condition)) panic(msg)

#endif