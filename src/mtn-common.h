#ifndef MTN_COMMON_H
#define MTN_COMMON_H

#include "mtn-protocol.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <time.h>
#include <semaphore.h>
#include <dirent.h>

#include <sys/stat.h>
#include <fcntl.h>

#include "uthash.h"

#define LINE_MAX_SIZE 4096
#define TOKEN_MAX_SIZE 4096

#define SERVER_DEFAULT_PORT 48888
#define SERVER_DEFAULT_PORT_TXT "48888"

#define SUBSCR_EXPIRE_TIME 5 

struct msg_id
{
    uint32_t message;
    uint32_t block;
};

struct mtn_request
{
    mtn_request_type type;
    union {
        struct msg_id msg;
    } info;
};

void print(const char* format, ...);
void info(const char* format, ...);
void warning(const char* format, ...);
void warning_errno(const char* format, ...);
void error(const char* format, ...);
void error_errno(const char* format, ...);
void fatal_error(int eval, const char* format, ...);
void fatal_error_errno(int eval, const char* format, ...);
void debug(unsigned int level, const char* format, ...);

time_t current_time();

int strcopy(char* dest, size_t* dest_position, const char* src, int copy_len);

/**
 * Replaces first occurance of substring in a string
 * with another string
 *
 * All input string have to end with '\0'
 *
 * @param[in] what what to replace
 * @param[in] with replace with what
 * @param[in] in source string
 * @param[out] where where the result come
 */
void str_replace(const char* what, const char* with, const char* in, char* where);

//void strjoin(char* dest, const char* delim, ...);

bool file_exists(const char* file);

void init_dir(char* path);
void init_file(char* file_path, const char* example_file);

int load_file(const char* file, int flags, const char* example_file);
DIR* load_dir(const char* dir);

bool rm_file(const char* file);

int get_file_line(char* line, int conf_file_fd, bool nonempty);
int get_token(char* token, char** str_ptr, const char* delim);

int get_sem_value(sem_t* sem);

#define print_hr() print("-------------------------------------------------------------")

#define streq(str1, str2) strcmp((str1), (str2)) == 0
#define strneq(str1, str2, length) strncmp((str1), (str2), (length)) == 0

#define strdcat(dest, src, delim) strcat(dest, delim); strcat(dest, src)

#define ALLOCATE(type, var) \
    type* var = (type*) malloc(sizeof(type)); \
    if(var == NULL) \
    { \
        fatal_error(1, "cannot allocate memory"); \
    } \
    memset(var, 0, sizeof(type))

#endif
