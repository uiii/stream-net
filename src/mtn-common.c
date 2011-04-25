#include "mtn-common.h"

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>

#include "mtn-config.h"

static pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

static
inline
void
print_message(FILE* stream, const char* prefix, const char* format, va_list argv, bool print_errno)
{
    pthread_mutex_lock(&print_mutex);

    fprintf(stream, prefix);
    vfprintf(stream, format, argv);

    if(print_errno)
    {
        fprintf(stream, ": %s", strerror(errno));
    }

    fprintf(stream, "\n");

    pthread_mutex_unlock(&print_mutex);
}

void
print(const char* format, ...)
{
    va_list argv;
    va_start(argv, format);

    print_message(stdout, "", format, argv, false);

    va_end(argv);
}

void
info(const char* format, ...)
{
    va_list argv;
    va_start(argv, format);

    print_message(stdout, "Info: ", format, argv, false);

    va_end(argv);
}

void
warning(const char* format, ...)
{
    va_list argv;
    va_start(argv, format);

    print_message(stderr, "Warning: ", format, argv, false);

    va_end(argv);
}

void
warning_errno(const char* format, ...)
{
    va_list argv;
    va_start(argv, format);

    print_message(stderr, "Warning: ", format, argv, true);

    va_end(argv);
}

void
error(const char* format, ...)
{
    va_list argv;
    va_start(argv, format);

    print_message(stderr, "Error: ", format, argv, false);

    va_end(argv);
}

void
error_errno(const char* format, ...)
{
    va_list argv;
    va_start(argv, format);

    print_message(stderr, "Error: ", format, argv, true);

    va_end(argv);
}

void
fatal_error(int eval, const char* format, ...)
{
    va_list argv;
    va_start(argv, format);

    print_message(stderr, "Fatal error: ", format, argv, false);

    va_end(argv);

    exit(eval);
}

void
fatal_error_errno(int eval, const char* format, ...)
{
    va_list argv;
    va_start(argv, format);

    print_message(stderr, "Fatal error: ", format, argv, true);

    va_end(argv);

    exit(eval);
}

void
debug(unsigned int level, const char* format, ...)
{
    char* level_str = get_config_value("debug");
    if(level_str && level <= atoi(level_str))
    {
        va_list argv;
        va_start(argv, format);
        
        char prefix[15];
        sprintf(prefix, "Debug[%d]: ", level);

        print_message(stderr, prefix, format, argv, false);

        va_end(argv);
    }
}

time_t
current_time()
{
    time_t c_time;
    if((c_time = time(NULL)) == -1)
    {
        fatal_error(1, "cannot measure time");
    }

    return c_time;
}

int
strcopy(char* dest, size_t* dest_position, const char* src, int copy_len)
{
    size_t position = *dest_position;
    int i = 0;
    for(i = 0; (copy_len == -1 || i < copy_len) && src[i] != '\0'; ++i)
    {
        dest[position + i] = src[i];
    }

    if(i < copy_len || copy_len == -1)
    {
        dest[position + i] = '\0';
    }

    *dest_position = position + i;
    return i;
}

void
str_replace(const char* what, const char* with, const char* in, char* where)
{
    char* substr = strstr(in, what);

    if((substr != NULL) && (what != NULL))
    {
        size_t pos = 0;
        int prefix_len = substr - in;
        strcopy(where, &pos, in, prefix_len);

        if(with != NULL)
        {
            strcopy(where, &pos, with, -1);
        }

        char* sufix = substr + strlen(what);
        strcopy(where, &pos, sufix, -1);
    }
    else
    {
        strcpy(where, in);
    }
}

bool
file_exists(const char* file)
{
    char path[4096];
    str_replace("~", getenv("HOME"), file, path);

    return access(path, F_OK) == 0;
}

void
init_dir(char* path)
{
    if(access(path, F_OK) != 0) // directory does not exists
    {
        char* slash_ptr = strrchr(path, '/');
        if(slash_ptr != NULL)
        {
            slash_ptr[0] = '\0';

            init_dir(path);

            slash_ptr[0] = '/';
        }

        // ending slash is present, then directory was created
        // in recursive call
        if((slash_ptr == NULL) || streq(slash_ptr, "/"))
        {
            if(mkdir(path, 0755) == -1)
            {
                error_errno("cannot create %s directory", path);
            }

            debug(1, "creating %s direcotry", path);
        }
    }
}

void
init_file(char* path, const char* example_file)
{
    char file_path[4096];
    strcpy(file_path, path);

    if(access(file_path, F_OK) != 0) // server config file does not exists
    {
        // init file base directory
        char* slash_ptr = strrchr(file_path, '/');
        if(slash_ptr != NULL)
        {
            slash_ptr[0] = '\0';

            init_dir(file_path);

            slash_ptr[0] = '/';
        }

        bool error = false;

        int inf, outf;
        if((outf = creat(file_path, 0655)) != -1)
        {
            debug(1, "creating %s file", file_path);

            if(example_file)
            {
                // copy example config file 
                if((inf = open(example_file, O_RDONLY)) != -1)
                {
                    char buffer[4096];
                    ssize_t read_len, write_len;
                    while((read_len = read(inf, buffer, 4096)) > 0)
                    {
                        write_len = write(outf, buffer, read_len);
                        if(write_len != read_len)
                        {
                            error = true;
                        }
                    }

                    close(inf);
                }
                else { error = true; }
            }

            close(outf);
        }
        else { error = true; }

        if(error)
        {
            error_errno("cannot initialize the %s file", file_path);
        }
    }
}

int
load_file(const char* file, int flags, const char* example_file)
{
    int file_fd;
    char path[4096];

    str_replace("~", getenv("HOME"), file, path);

    if(example_file)
    {
        char example_path[4096];
        str_replace("~", getenv("HOME"), example_file, example_path);
        init_file(path, example_path);
    }
    else
    {
        init_file(path, NULL);
    }


    file_fd = open(path, flags);

    if(file_fd == -1)
    {
        error_errno("cannot open %s file", path);
    }

    debug(1, "loading %s file", file);

    return file_fd;
}

DIR*
load_dir(const char* dir_path)
{
    DIR* dir;
    char path[4096];

    str_replace("~", getenv("HOME"), dir_path, path);
    init_dir(path);

    dir = opendir(path);

    if(dir == NULL)
    {
        error_errno("cannot open %s directory", path);
    }

    debug(1, "loading %s directory", dir_path);

    return dir;
}

bool
rm_file(const char* file)
{
    char path[4096];

    str_replace("~", getenv("HOME"), file, path);
    if(unlink(path) == -1)
    {
        error_errno("cannot remove file %s", path);
        return false;
    }

    return true;
}

int
get_file_line(char* line, int conf_file_fd, bool nonempty)
{
    static char buffer[LINE_MAX_SIZE + 1] = "";
    static char* buffer_ptr = buffer;
    static int read_len = 0;

    if(read_len == 0)
    {
        read_len = read(conf_file_fd, buffer, LINE_MAX_SIZE);
        buffer[read_len] = '\0';
        buffer_ptr = buffer;
    }

    if(read_len)
    {
        if(nonempty)
        {
            int skip_len = strspn(buffer_ptr, "\n");
            buffer_ptr += skip_len;
            read_len -= skip_len;
        }

        int line_len = strcspn(buffer_ptr, "\n");

        strncpy(line, buffer_ptr, line_len);
        line[line_len] = '\0';

        buffer_ptr += line_len + 1;
        read_len -= line_len + 1;

        if(read_len < 0)
        {
            read_len = 0;
            
            char next_line[LINE_MAX_SIZE + 1];
            int next_line_len = get_file_line(next_line, conf_file_fd, false);
            
            if(next_line_len)
            {
                line_len += next_line_len;
                if(line_len > LINE_MAX_SIZE)
                {
                    fatal_error(1, "line in file is too long");
                }
                else
                {
                    strcat(line, next_line);
                }
            }
        }

        return line_len;
    }
    else
    {
        strcpy(line, "");
        return -1;
    }
}

int
get_token(char* token, char** str_ptr, const char* delim)
{
    int skip_len = strspn(*str_ptr, delim);
    *str_ptr += skip_len;

    int token_len = strcspn(*str_ptr, delim);
    strncpy(token, *str_ptr, token_len);
    token[token_len] = '\0';

    *str_ptr += token_len;

    return token_len;
}

int
get_sem_value(sem_t* sem)
{
    int val;
    if(sem_getvalue(sem, &val) == -1)
    {
        fatal_error_errno(1, "cannot get value of a semaphore");
    }

    return val;
}
