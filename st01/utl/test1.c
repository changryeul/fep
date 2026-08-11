#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#ifndef GET_IP_H
#define GET_IP_H

int get_ip_address(char *Get_Ip);

#endif

int get_ip_address(char *Get_Ip) {
    struct ifaddrs *ifaddr, *ifa;
    char addr[INET_ADDRSTRLEN];

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return -1;  // 오류 발생 시 -1 반환
    }

    int found = 0;

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;

        if (ifa->ifa_addr->sa_family == AF_INET) {
            // lo (loopback) 제외
            if (strcmp(ifa->ifa_name, "lo") == 0)
                continue;

            struct sockaddr_in *sa = (struct sockaddr_in *)ifa->ifa_addr;
            inet_ntop(AF_INET, &(sa->sin_addr), addr, INET_ADDRSTRLEN);

            strncpy(Get_Ip, addr, 30);  // 최대 30바이트로 복사
            Get_Ip[29] = '\0';  // 안전하게 널종료
            found = 1;
            break;  // 첫번째로 찾은 IP만 사용
        }
    }

    freeifaddrs(ifaddr);

    if (found)
        return 0;  // 성공
    else
        return -2;  // IP 못 찾음
}
