#include "common.h"


#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>

#define BUFSZ 504 // 4 additional chars to identify msg size


int parseInt(char *str)
{
    int zeros = 0;
    char buf[2];

    if (str[strlen(str) - 1] == '\n')
    {
        str[strcspn(str, "\n")] = 0; // removes '\n' from string
    }

    for (int i = 0; i < strlen(str); i++)
    { // handles case where number starts with '0'
        sprintf(buf, "%c", str[i]);
        if (strcmp(buf, "0") != 0)
            break;
        zeros++;
    }

    int val = atoi(str);

    char validator[strlen(str)];
    sprintf(validator, "%d", val);

    if (strcmp(validator, str) == zeros)
        return val;

    return -1;
}



void logexit(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int getMsgSize(char *reader, char *receiver)
{
    char *strReader[BUFSZ];
    char c[2]; // char size

    for (int i = 0; i < 4; i++)
    { // messages can have max 4 digits representing its size
        sprintf(c, "%c", reader[i]);
        if (strcmp(c, "-") == 0)
        {
            *strReader = &reader[i + 1];
            reader[i] = '\0';
            strcpy(receiver, *strReader);
            break;
        }
        bzero(c, strlen(c));
    }

    return parseInt(reader);
}

void getWholeMsg(int sfd, char *reader, char *buf, size_t count)
{
    int nBytes = getMsgSize(reader, buf);
    unsigned total = strlen(buf);

    while (total < nBytes)
    { // case when the message received is partitioned
        bzero(reader, BUFSZ);
        count = recv(sfd, reader, BUFSZ - 1, 0);
        strcat(buf, reader);
        total += count;
    }
}

int server_sockaddr_init(const char *proto, const char *portstr,
                         struct sockaddr_storage *storage)
{
    uint16_t port = (uint16_t)atoi(portstr); // unsigned short
    if (port == 0)
    {
        return -1;
    }
    port = htons(port); // host to network short

    memset(storage, 0, sizeof(*storage));
    if (0 == strcmp(proto, "v4"))
    {
        struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
        addr4->sin_family = AF_INET;
        addr4->sin_addr.s_addr = INADDR_ANY;
        addr4->sin_port = port;
        return 0;
    }
    else if (0 == strcmp(proto, "v6"))
    {
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_addr = in6addr_any;
        addr6->sin6_port = port;
        return 0;
    }
    else
    {
        return -1;
    }
}

int addrparse(const char *addrstr, const char *portstr,
              struct sockaddr_storage *storage)
{
    if (addrstr == NULL || portstr == NULL)
    {
        return -1;
    }

    uint16_t port = (uint16_t)atoi(portstr); 
    if (port == 0)
    {
        return -1;
    }
    port = htons(port); 

    struct in_addr inaddr4; 
    if (inet_pton(AF_INET, addrstr, &inaddr4))
    {
        struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
        addr4->sin_family = AF_INET;
        addr4->sin_port = port;
        addr4->sin_addr = inaddr4;
        return 0;
    }

    struct in6_addr inaddr6; 
    if (inet_pton(AF_INET6, addrstr, &inaddr6))
    {
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = port;
        
        memcpy(&(addr6->sin6_addr), &inaddr6, sizeof(inaddr6));
        return 0;
    }

    return -1;
}

void addrtostr(const struct sockaddr *addr, char *str, size_t strsize)
{
    int version;
    char addrstr[INET6_ADDRSTRLEN + 1] = "";
    uint16_t port;

    if (addr->sa_family == AF_INET)
    {
        version = 4;
        struct sockaddr_in *addr4 = (struct sockaddr_in *)addr;
        if (!inet_ntop(AF_INET, &(addr4->sin_addr), addrstr,
                       INET6_ADDRSTRLEN + 1))
        {
            logexit("ntop");
        }
        port = ntohs(addr4->sin_port);
    }
    else if (addr->sa_family == AF_INET6)
    {
        version = 6;
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)addr;
        if (!inet_ntop(AF_INET6, &(addr6->sin6_addr), addrstr,
                       INET6_ADDRSTRLEN + 1))
        {
            logexit("ntop");
        }
        port = ntohs(addr6->sin6_port); 
    }
    else
    {
        logexit("unknown protocol family.");
    }
    if (str)
    {
        snprintf(str, strsize, "IPv%d %s %hu", version, addrstr, port);
    }
}




