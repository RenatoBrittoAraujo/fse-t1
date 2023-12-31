#ifndef SHARED_UTIL_H
#define SHARED_UTIL_H 1

#define LEVEL_DEBUG 0
#define LEVEL_INFO 1
#define LEVEL_WARNING 2
#define LEVEL_ERROR 3

#include <shared/inc/errors.h>

void set_level(int level);
void log_print(char *str, int level);

char *itoa(int x);

// if index is not present, may set index to a negative number
int read_env_int_index(char *name, int index);

// if index is not present, may set index to a negative number
char *read_env_str_index(char *name, int index);

// if index is not present, may set index to a negative number
double read_env_double_index(char *name, int index);

void print_bits(char *str, size_t size);


#endif