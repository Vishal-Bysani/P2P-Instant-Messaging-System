#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <pthread.h>
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

int main(int argc, char *argv[]){
    char my_ip[INET_ADDRSTRLEN];
    get_my_ip(my_ip);
    std::cout << "My IP: " << my_ip << std::endl;
    return 0;
}