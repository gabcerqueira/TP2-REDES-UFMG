#pragma once

#include <stdlib.h>
#include <arpa/inet.h>

void logexit(const char *msg);

int parseInt(char *str);

void addrtostr(const struct sockaddr *addr, char *str, size_t strsize);

int server_sockaddr_init(const char *proto, const char *portstr,
                         struct sockaddr_storage *storage);

int getMsgSize(char *reader, char *receiver);

void getWholeMsg(int sfd, char *reader, char *buf, size_t count);

int addrparse(const char *addrstr, const char *portstr,
              struct sockaddr_storage *storage);





