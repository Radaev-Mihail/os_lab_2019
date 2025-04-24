#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "multmodulo.h"
struct Server {
    char ip[64];
    int port;
};



int read_servers(const char *path, struct Server **servers) {
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Cannot open file %s\n", path);
        return -1;
    }

    size_t count = 0;
    size_t capacity = 4;
    *servers = malloc(capacity * sizeof(struct Server));

    while (fscanf(file, "%63s %d", (*servers)[count].ip, &(*servers)[count].port) == 2) {
        count++;
        if (count >= capacity) {
            capacity *= 2;
            *servers = realloc(*servers, capacity * sizeof(struct Server));
        }
    }

    fclose(file);
    return count;
}

int send_task(const struct Server *server, uint64_t begin, uint64_t end, uint64_t mod, uint64_t *result) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        fprintf(stderr, "Error: Cannot create socket\n");
        return -1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server->port);
    inet_pton(AF_INET, server->ip, &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "Error: Cannot connect to server %s:%d\n", server->ip, server->port);
        close(sock);
        return -1;
    }

    uint64_t task[3] = {begin, end, mod};
    if (send(sock, task, sizeof(task), 0) < 0) {
        fprintf(stderr, "Error: Cannot send data to server\n");
        close(sock);
        return -1;
    }

    if (recv(sock, result, sizeof(*result), 0) < 0) {
        fprintf(stderr, "Error: Cannot receive data from server\n");
        close(sock);
        return -1;
    }

    close(sock);
    return 0;
}

int main(int argc, char **argv) {
    uint64_t k = 0;
    uint64_t mod = 0;
    char *servers_path = NULL;

    while (true) {
        int current_optind = optind ? optind : 1;

        static struct option options[] = {
            {"k", required_argument, 0, 'k'},
            {"mod", required_argument, 0, 'm'},
            {"servers", required_argument, 0, 's'},
            {0, 0, 0, 0}};

        int option_index = 0;
        int c = getopt_long(argc, argv, "", options, &option_index);

        if (c == -1)
            break;

        switch (c) {
        case 'k':
            k = strtoull(optarg, NULL, 10);
            break;
        case 'm':
            mod = strtoull(optarg, NULL, 10);
            break;
        case 's':
            servers_path = optarg;
            break;
        default:
            fprintf(stderr, "Usage: %s --k <value> --mod <value> --servers <path>\n", argv[0]);
            return 1;
        }
    }

    if (k == 0 || mod == 0 || servers_path == NULL) {
        fprintf(stderr, "Usage: %s --k <value> --mod <value> --servers <path>\n", argv[0]);
        return 1;
    }

    struct Server *servers = NULL;
    int servers_count = read_servers(servers_path, &servers);
    if (servers_count <= 0) {
        fprintf(stderr, "Error: No servers available\n");
        return 1;
    }

    uint64_t chunk_size = k / servers_count;
    uint64_t remaining = k % servers_count;

    uint64_t total = 1;
    for (int i = 0; i < servers_count; i++) {
        uint64_t begin = i * chunk_size + 1;
        uint64_t end = (i + 1) * chunk_size;

        if (i == servers_count - 1) // Add remaining part to the last chunk
            end += remaining;

        uint64_t result = 0;
        if (send_task(&servers[i], begin, end, mod, &result) < 0) {
            fprintf(stderr, "Error: Task failed on server %s:%d\n", servers[i].ip, servers[i].port);
            free(servers);
            return 1;
        }

        total = MultModulo(total, result, mod);
    }

    printf("Final answer: %lu\n", total);

    free(servers);
    return 0;
}