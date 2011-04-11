#include "common.h"

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <err.h>

#include <sys/stat.h>

int
strcopy(char* dest, size_t* dest_position, const char* src, int copy_len)
{
    int position = *dest_position;
    int i = 0;
    for(i = 0; src[i] != '\0'; ++i)
    {
        dest[position + i] = src[i];

        if(copy_len > -1 && (i == copy_len))
        {
            break;
        }
    }

    *dest_position = position + i;
    return i;
}

void str_replace(const char* what, const char* with, const char* in, char* where)
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

void
init_dir(char* path)
{
    printf("init_dir %s\n", path);
    if(access(path, F_OK) != 0) // directory does not exists
    {
        char* slash_ptr = strrchr(path, '/');
        if(slash_ptr != NULL)
        {
            slash_ptr[0] = '\0';

            init_dir(path);

            slash_ptr[0] = '/';
        }

        // ending slash is present, than directory was created
        // in recursive call
        if(strcmp(slash_ptr, "/") != 0)
        {
            if(mkdir(path, 0755) == -1)
            {
                err(1, "Cannot create %s directory", path);
            }
        }
    }
}

void
init_file(char* file_path, const char* example_file)
{
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
            err(1, "Cannot initialize the %s file\n", file_path);
        }
    }
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
                    fprintf(stderr, "Error: Line in file is too long");
                    exit(1);
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
get_token(char* token, const char* str, const char* delim, bool from_beginning)
{
    static const char* line_ptr = 0;

    if(from_beginning)
    {
        line_ptr = line;
    }

    int skip_len = strspn(line_ptr, delim);
    line_ptr += skip_len;

    int token_len = strcspn(line_ptr, delim);
    strncpy(token, line_ptr, token_len);
    token[token_len] = '\0';

    line_ptr += token_len;

    return token_len;
}
