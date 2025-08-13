// config.hpp
#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <iostream>
#include <cstring>
#include <vector>
#include <cstdlib>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <fstream>

#define MAX_NAME_LEN 50
#define MAX_IP_LEN 16
#define MAX_MSG_LEN 1024
#define MAX_USERS 10
#define MAX_CONTACTS 10
#define MAX_QUEUED_REQUESTS 5
#define TRACKER_IP "127.0.0.1"
#define MAX_CLIENT_PORTS 6
#define MAX_PORT_LENGTH 5

std::vector<int> available_ports = {8080, 8081}, client_available_ports = {8082, 8083, 8084, 8086, 8087, 8088};

struct Peer {
    char name[MAX_NAME_LEN];
    char ip[MAX_IP_LEN];
    int port;
    char* contacts[MAX_CONTACTS];
    char* contacts_ip[MAX_CONTACTS];
    int contacts_port[MAX_CONTACTS];
};

struct Tracker {
    char* names[MAX_USERS];
    char* ips[MAX_USERS];
    int ports[MAX_USERS];
};

void get_my_ip(char ip[]) {
    struct ifaddrs *ifAddrStruct = nullptr;
    getifaddrs(&ifAddrStruct);
    for (struct ifaddrs *ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) continue;
        if (ifa->ifa_addr->sa_family == AF_INET && strcmp(ifa->ifa_name, "lo") != 0) {
            void* tmp = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            inet_ntop(AF_INET, tmp, ip, INET_ADDRSTRLEN);
            break;
        }
    }
    if (ifAddrStruct != nullptr) freeifaddrs(ifAddrStruct);
}

#endif
