/*
Networks Lab

Rushil Venkateswar (20CS30045)
Jatin Gupta (20CS10087)
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/in_systm.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <sys/ioctl.h>

#define MAX_SIZE 1024
#define IP_MAXLEN 16
#define DWORD 4
#define IPv4 4
#define ALPHA 0.125
#define PORT 20000
#define POLL_TIMEOUT 1

uint16_t checksum(const void *buff, size_t nbytes);
char *dnsLookup(const char *h_name, struct sockaddr_in *addr);
char *networkInterfaceLookup();
void printIP(struct iphdr *ip);
void printICMP(struct icmphdr *icmp);
uint16_t create_packet(char *buffer, char *msg, char *src_ip, char *dest_ip, int ttl);
double calc_bandwidth(double t1, double t2, int d1, int d2, int cumm_prev_time);

int main(int argc, char *argv[])
{
    const int ON = 1;
    int sockfd, n, T;
    struct sockaddr_in destaddr;
    char *src_ip, *dest_ip;

    int ttl = 1;
    double cumm_prev_latency = 0;
    double cumm_prev_time = 0;
    struct pollfd fd[1];
    int trial = 5;

    if (getuid() != 0)
    {
        printf("This application requires root privileges\n");
        exit(EXIT_FAILURE);
    }
    if (argc != 4)
    {
        printf("Invalid command\n");
        printf("Usage: <executable> <site_to_probe> <n> <T>\n");
        exit(EXIT_FAILURE);
    }

    if ((src_ip = networkInterfaceLookup()) == NULL)
    {
        exit(EXIT_FAILURE);
    }
    if ((dest_ip = dnsLookup(argv[1], &destaddr)) == NULL)
    {
        exit(EXIT_FAILURE);
    }
    printf("%s\n", src_ip);
    printf("%s\n", dest_ip);

    n = atoi(argv[2]);
    T = atoi(argv[3]);

    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0)
    {
        perror("socket()");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, (const void *)&ON, sizeof(ON)) < 0)
    {
        perror("setsockopt with IP_HDRINCL option");
        exit(EXIT_FAILURE);
    }
    
    while (ttl<64)
    {
        int ending = 0;
        char *buffer = (char *)malloc(MAX_SIZE * sizeof(char));
        char *msg = NULL;
        int tot_len = create_packet(buffer, msg, src_ip, dest_ip, ttl);
        int repeat = 0, no_success = 0;
        uint32_t intermediate_ip = 0;

        for(int i = 0;i<5;i++){
            if (sendto(sockfd, buffer, tot_len, 0, (const struct sockaddr *)&destaddr, sizeof(destaddr)) < 0)
            {
                perror("sendto()");
                exit(EXIT_FAILURE);
            }
            struct sockaddr_in addr;
            socklen_t addrlen = sizeof(addr);
            char *recv_buffer = (char *)malloc(MAX_SIZE * sizeof(char));
            int recvlen;

            // adding polling
            fd[0].fd = sockfd;
            fd[0].events = POLLIN;
            int rc = poll(fd, 1, POLL_TIMEOUT * 1000);
            if(rc < 0){
                perror("poll failed\n");
                exit(EXIT_FAILURE);
            }
            else if(rc == 0){
                repeat = 1;
                continue; 
            }

            // polling successful
            if ((recvlen = recvfrom(sockfd, recv_buffer, MAX_SIZE, 0, (struct sockaddr *)&addr, &addrlen)) < 0){
                perror("recvfrom");
                exit(EXIT_FAILURE);
            }
            struct iphdr *ip_reply = (struct iphdr *)recv_buffer;            
            struct icmphdr *icmp_reply = (struct icmphdr *)(recv_buffer + sizeof(struct iphdr));
            char *data_reply = recv_buffer + sizeof(struct iphdr) + sizeof(struct icmphdr);

            if(i == 0){
                intermediate_ip = (ip_reply->saddr);
            }
            else if(intermediate_ip == (ip_reply->saddr));
            else{
                repeat = 1;
                break;
            }
        }
        if(!trial){
            printf("Route not found after 5 trials. Exiting the program . . .\n");
            break;
        }
        if(repeat){
            trial--;
            printf("Trial Left: %d\n", trial);
            continue;
        }

        printf("Intermediate ip: %s\n", inet_ntoa(*(struct in_addr *) &intermediate_ip));

        // we have the ip address of the intermediate router
        char *buffer1 = (char *)malloc(MAX_SIZE*sizeof(char));
        char *msg1 = NULL;
        uint16_t tot_len1 = create_packet(buffer1, msg1,src_ip, dest_ip, ttl);
        float rtt_avg1 = -1;

        char *buffer2 = (char *)malloc(MAX_SIZE*sizeof(char));
        char msg2[]="hello world";
        uint16_t tot_len2 = create_packet(buffer2, msg2, src_ip, dest_ip, ttl);
        float rtt_avg2 = -1;

        for (int i = 0; i < n; i++)
        {
            if (sendto(sockfd, buffer1, tot_len1, 0, (const struct sockaddr *)&destaddr, sizeof(destaddr)) < 0)
            {
                perror("sendto()");
                exit(EXIT_FAILURE);
            }
            struct timeval send_tv, recv_tv;
            if (gettimeofday(&send_tv, NULL) < 0)
            {
                perror("gettimeofday");
                exit(EXIT_FAILURE);
            }
            struct sockaddr_in addr;
            socklen_t addrlen = sizeof(addr);
            char *recv_buffer = (char *)malloc(MAX_SIZE * sizeof(char));
            int recvlen;

            // adding polling
            fd[0].fd = sockfd;
            fd[0].events = POLLIN;
            int rc = poll(fd, 1, T * 1000);
            if(rc < 0){
                perror("poll failed\n");
                exit(EXIT_FAILURE);
            }
            else if(rc == 0){
                continue;
            }

            if ((recvlen = recvfrom(sockfd, recv_buffer, MAX_SIZE, 0, (struct sockaddr *)&addr, &addrlen)) < 0)
            {
                perror("recvfrom");
                exit(EXIT_FAILURE);
            }
            if (gettimeofday(&recv_tv, NULL) < 0)
            {
                perror("gettimeofday");
                exit(EXIT_FAILURE);
            }
            int rtt_m1 = 1000000 * (recv_tv.tv_sec - send_tv.tv_sec) + (recv_tv.tv_usec - send_tv.tv_usec);

            struct iphdr *ip_reply = (struct iphdr *)recv_buffer;            
            struct icmphdr *icmp_reply = (struct icmphdr *)(recv_buffer + sizeof(struct iphdr));
            char *data_reply = recv_buffer + sizeof(struct iphdr) + sizeof(struct icmphdr);

            printIP(ip_reply);
            printICMP(icmp_reply);

            // second packet
            if (sendto(sockfd, buffer2, tot_len2, 0, (const struct sockaddr *)&destaddr, sizeof(destaddr)) < 0)
            {
                perror("sendto()");
                exit(EXIT_FAILURE);
            }

            if (gettimeofday(&send_tv, NULL) < 0)
            {
                perror("gettimeofday");
                exit(EXIT_FAILURE);
            }

            addrlen = sizeof(addr);
            recv_buffer = (char *)malloc(MAX_SIZE * sizeof(char));

            // adding polling
            fd[0].fd = sockfd;
            fd[0].events = POLLIN;
            rc = poll(fd, 1, T * 1000);
            if(rc < 0){
                perror("poll failed\n");
                exit(EXIT_FAILURE);
            }
            else if(rc == 0){
                continue;
            }

            if ((recvlen = recvfrom(sockfd, recv_buffer, MAX_SIZE, 0, (struct sockaddr *)&addr, &addrlen)) < 0)
            {
                perror("recvfrom");
                exit(EXIT_FAILURE);
            }
            if (gettimeofday(&recv_tv, NULL) < 0)
            {
                perror("gettimeofday");
                exit(EXIT_FAILURE);
            }
            int rtt_m2 = 1000000 * (recv_tv.tv_sec - send_tv.tv_sec) + (recv_tv.tv_usec - send_tv.tv_usec);

            rtt_avg1 = (rtt_avg1 < 0) ? rtt_m1 : (1 - ALPHA) * rtt_avg1 + ALPHA * rtt_m1;
            rtt_avg2 = (rtt_avg2 < 0) ? rtt_m2 : (1 - ALPHA) * rtt_avg2 + ALPHA * rtt_m2;

            ip_reply = (struct iphdr *)recv_buffer;            
            icmp_reply = (struct icmphdr *)(recv_buffer + sizeof(struct iphdr));
            data_reply = recv_buffer + sizeof(struct iphdr) + sizeof(struct icmphdr);
            
            printIP(ip_reply);
            printICMP(icmp_reply);
            // printf("%s\n", data_reply);

            if(icmp_reply->type == ICMP_ECHOREPLY)  ending = 1;
        }


        printf("HOP = %d: rtt_avg1 = %lf us rtt_avg2 = %lf us\n", ttl, rtt_avg1, rtt_avg2);
        double latency = rtt_avg1 - cumm_prev_latency;
        printf("Link Latency: %lf\n", latency);
        cumm_prev_latency += latency;

        double bandwidth = calc_bandwidth(rtt_avg1, rtt_avg2, 0, strlen(msg2), cumm_prev_time);
        printf("Link Bandwidth: %lf\n\n", bandwidth);
        cumm_prev_time += rtt_avg2 - rtt_avg1;
        if(ending) break;
        ttl++;
    }
    return 0;
}

uint16_t create_packet(char *buffer, char *msg, char *src_ip, char *dest_ip, int ttl){
    struct iphdr *ip = (struct iphdr *)buffer;
    struct icmphdr *icmp = (struct icmphdr *)(buffer + sizeof(struct iphdr));
    char *data = (char *)(buffer + sizeof(struct iphdr) + sizeof(struct icmphdr));
    ip->version = IPv4;
    ip->ihl = sizeof(struct iphdr) / DWORD;
    ip->tos = 0;
    ip->tot_len = htons(sizeof(struct iphdr) + sizeof(struct icmphdr) + sizeof(msg));
    ip->id = getpid();
    ip->frag_off = 0;
    ip->ttl = ttl;
    ip->protocol = IPPROTO_ICMP;
    ip->check = 0;
    ip->saddr = inet_addr(src_ip);
    ip->daddr = inet_addr(dest_ip);
    ip->check = checksum(ip, sizeof(struct iphdr));

    icmp->type = ICMP_ECHO;
    icmp->code = 0;
    icmp->checksum = 0;
    icmp->un.echo.id = rand();
    icmp->un.echo.sequence = 0;
    if(msg){
        for (int i = 0; i < sizeof(msg); i++)
            data[i] = msg[i];
    }
    icmp->checksum = checksum(icmp, sizeof(struct icmphdr) + sizeof(msg));   
    return ntohs(ip->tot_len);
}

uint16_t checksum(const void *buff, size_t nbytes)
{
    uint64_t sum = 0;
    uint16_t *words = (uint16_t *)buff;
    size_t _16bitword = nbytes / 2;
    while (_16bitword--)
    {
        sum += *(words++);
    }
    if (nbytes & 1)
    {
        sum += (uint16_t)(*(uint8_t *)words) << 0x0008;
    }
    sum = ((sum >> 16) + (sum & 0xFFFF));
    sum += (sum >> 16);
    return (uint16_t)(~sum);
}

char *dnsLookup(const char *h_name, struct sockaddr_in *addr)
{
    struct hostent *host;
    if ((host = gethostbyname(h_name)) == NULL || host->h_addr_list[0] == NULL)
    {
        printf("%s: can't resolve\n", h_name);
        switch (h_errno)
        {
        case HOST_NOT_FOUND:
            printf("Host not found\n");
            break;
        case TRY_AGAIN:
            printf("Non-authoritative. Host not found\n");
            break;
        case NO_DATA:
            printf("Valid name, no data record of requested type\n");
            break;
        case NO_RECOVERY:
            printf("Non-recoverable error\n");
            break;
        }
        return NULL;
    }
    // check AF_INET
    if (addr != NULL)
    {
        addr->sin_family = host->h_addrtype;
        addr->sin_port = htons(PORT);
        addr->sin_addr = *(struct in_addr *)host->h_addr_list[0];
    }
    return strdup(inet_ntoa(*(struct in_addr *)host->h_addr_list[0]));
}

char *networkInterfaceLookup(){
    int fd;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    /* I want to get an IPv4 IP address */
    ifr.ifr_addr.sa_family = AF_INET;

    /* I want IP address attached to "eth0" */
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);

    ioctl(fd, SIOCGIFADDR, &ifr);

    close(fd);

    /* display result */
    return strdup(inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
}

void printIP(struct iphdr *ip)
{
    printf("-----------------------------------------------------------------\n");
    printf("|   version:%-2d  |   hlen:%-4d   |     tos:%-2d    |  totlen:%-4d  |\n", ip->version, ip->ihl, ip->tos, ntohs(ip->tot_len));
    printf("-----------------------------------------------------------------\n");
    printf("|           id:%-6d           |%d|%d|%d|      frag_off:%-4d      |\n", ntohs(ip->id), ip->frag_off && (1 << 15), ip->frag_off && (1 << 14), ip->frag_off && (1 << 14), ip->frag_off);
    printf("-----------------------------------------------------------------\n");
    printf("|    ttl:%-4d   |  protocol:%-2d  |         checksum:%-6d       |\n", ip->ttl, ip->protocol, ip->check);
    printf("-----------------------------------------------------------------\n");
    printf("|                    source:%-16s                    |\n", inet_ntoa(*(struct in_addr *)&ip->saddr));
    printf("-----------------------------------------------------------------\n");
    printf("|                 destination:%-16s                  |\n", inet_ntoa(*(struct in_addr *)&ip->daddr));
    printf("-----------------------------------------------------------------\n");
}

void printICMP(struct icmphdr *icmp)
{
    printf("-----------------------------------------------------------------\n");
    printf("|    type:%-2d    |    code:%-2d    |        checksum:%-6d        |\n", icmp->type, icmp->code, icmp->checksum);
    printf("-----------------------------------------------------------------\n");
    if (icmp->type == ICMP_ECHO || icmp->type == ICMP_ECHOREPLY)
        printf("|           id:%-6d           |        sequence:%-6d        |\n", icmp->un.echo.id, icmp->un.echo.sequence);
    printf("-----------------------------------------------------------------\n");
}

double calc_bandwidth(double t1, double t2, int d1, int d2, int cumm_prev_time){
    double temp = (d2 - d1)/(double)(t2 - t1 - cumm_prev_time);
    return temp;
}