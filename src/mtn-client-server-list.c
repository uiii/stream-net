#include "mtn-server-list.h"

client_server_list
load_client_server_list(const char* server_list_file)
{
    int server_list_file_fd;
    server_list_file_fd = load_client_server_list(server_list_file);

    struct address_list_node* list = parse_client_server_list(server_list_file_fd);
    return list;
}

int
load_client_server_list_file(const char* server_list_file)
{
    int server_list_file_fd;
    if(server_list_file == NULL)
    {
        char path[4096];
        str_replace("~", getenv("HOME"), USER_CFG_PATH, path);
        init_dir(path);

        strcat(path, USER_CLIENT_SERVER_LIST_FILE);
        init_file(path, NULL);

        server_list_file_fd = open(path, O_RDONLY);
    }
    else
    {
        server_list_file_fd = open(server_list_file, O_RDONLY);
    }

    if(server_list_file_fd == -1)
    {
        err(1, "Cannot open server list file");
    }

    return server_list_file_fd;
}

client_server_list
parse_client_server_list(int server_list_file_fd);
{
    client_server_list list = NULL;

    int count = 0;

    char line[4096];
    while(get_file_line(line, server_list_file_fd, true) != -1)
    {
        char address_str[4096];
        get_token(address_str, line, " \t", true);

        struct sockaddr_in address;
        memset(&address, 0, sizeof(address));

        address.sin_family = AF_INET;

        char ip_adress_str[INET_ADDRSTRLEN];
        if(get_token(ip_adress_str, address_str, ":", true))
        {
            if(inet_pton(AF_INET, ip_address_str, &(address.sin_addr)) != 1)
            {
                fprintf(stderr, "Error: invalid IP adress '%s' ... IGNORING\n", ip_address_str);
            }
            else
            {
                char port_str[5];
                if(get_token(port, address_str, ":", false))
                {
                    address.sin_port = htons(atoi(port_str));
                }
                else
                {
                    printf("Warning: using default server port for IP adress '%s'", ip_address_str);
                    address.sin_port = htons(SERVER_DEFAULT_PORT);
                }

                struct server_list_node* node = (struct server_list_node*) malloc(sizeof(struct server_list_node));
                struct server_data* server_data = (struct server_data*) malloc(sizeof(struct server_data));
                if(node == NULL || server_data == NULL)
                {
                    fprintf(stderr, "Fatal Error: cannot allocate memory\n");
                    exit(1);
                }
                else
                {
                    memset(node, 0, sizeof(node));
                    memset(server_data, 0, sizeof(server_data));

                    server_data->address = address;

                    node->data = server_data;
                }

                // add address node to linked list
                DL_APPEND(list, node);

                count++;
            }
        }
    }

    if(count == 0)
    {
        fprintf(stderr, "Warning: no server is present in server list\n");
    }

    return list;
}
