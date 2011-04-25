#include "mtn-client-server-list.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <err.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "mtn-client.h"

server_map*
client_server_map()
{
    static server_map map = NULL;
    return &map;
}

void
load_client_server_list(const char* server_list_file)
{
    int server_list_file_fd;
    if(server_list_file)
    {
        server_list_file_fd = load_file(server_list_file, O_RDONLY, NULL);
    }
    else
    {
        server_list_file_fd = load_file(USER_CLIENT_SERVER_LIST_FILE, O_RDONLY, NULL);
    }

    if(server_list_file_fd == -1)
    {
        warning("client server list won't be loaded");
    }
    else
    {
        /*server_map* map = client_server_map();
        *map = parse_client_server_list(server_list_file_fd);*/
        *client_server_map() = parse_client_server_list(server_list_file_fd);
    }
}

server_map
parse_client_server_list(int server_list_file_fd)
{
    server_map map = NULL;

    char line[4096];
    while(get_file_line(line, server_list_file_fd, true) != -1)
    {
        char* line_ptr = line;

        char address_str[4096];
        get_token(address_str, &line_ptr, " \t");

        struct sockaddr_in address;
        memset(&address, 0, sizeof(address));

        address.sin_family = AF_INET;

        char* address_str_ptr = address_str;

        char ip_address_str[INET_ADDRSTRLEN];
        if(get_token(ip_address_str, &address_str_ptr, ":"))
        {
            if(inet_pton(AF_INET, ip_address_str, &(address.sin_addr)) != 1)
            {
                error("invalid IP adress '%s' ... IGNORING", ip_address_str);
            }
            else
            {
                int port = 0;
                char port_str[5];

                if(get_token(port_str, &address_str_ptr, ":"))
                {
                    port = atoi(port_str);
                }

                if(port == 0)
                {
                    warning("using default server port for IP adress '%s'", ip_address_str);
                    port = SERVER_DEFAULT_PORT;
                }

                address.sin_port = htons(port);

                struct server_map_node* node;
                HASH_FIND(hh, map, &address, sizeof(address), node);

                if(node)
                {
                    error("server %s:%d already loaded ... IGNORING", ip_address_str, port);
                }
                else
                {
                    info("loading %s:%d server from list", ip_address_str, port);

                    node = (struct server_map_node*) malloc(sizeof(struct server_map_node));
                    struct server_data* server_data = (struct server_data*) malloc(sizeof(struct server_data));
                    if(node == NULL || server_data == NULL)
                    {
                        fatal_error_errno(1, "cannot allocate memory");
                    }
                    else
                    {
                        memset(node, 0, sizeof(node));
                        memset(server_data, 0, sizeof(server_data));

                        server_data->address = address;
                        pthread_mutex_init(&server_data->mutex, NULL);

                        node->key = address;
                        node->data = server_data;
                    }

                    // add address node to server map
                    HASH_ADD(hh, map, key, sizeof(server_map_key), node);
                }
            }
        }
    }

    int count = HASH_COUNT(map);
    if(count == 0)
    {
        warning("no server is present in the server list");
    }
    else
    {
        info("%d server%s loaded", count, (count > 1) ? "s" : "");
    }

    return map;
}
