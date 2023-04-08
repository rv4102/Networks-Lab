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
#define MAX_SIZE 1024
#define IP_MAXLEN 16
#define DWORD 4
#define IPv4 4
#define ALPHA 0.125
#define PORT 20000
uint16_t checksum(const void *buff, size_t nbytes);
char *dnsLookup(const char *h_name, struct sockaddr_in *addr);
char *niLookup(int ni_family, struct sockaddr_in *addr);
void printIP(struct ip *ip);
void printICMP(struct icmp *icmp);

int main(int argc, char *argv[])
{
    const int ON = 1;
    int sockfd, n, T;
    struct sockaddr_in srcaddr, destaddr;
    char *src_ip, *dest_ip;

    if (getuid() != 0)
    {
        printf("This application requires root priviledges\n");
        exit(EXIT_FAILURE);
    }
    if (argc != 4)
    {
        printf("Invalid command\n");
        printf("Usage: <executable> <site_to_probe> <n> <T>\n");
        exit(EXIT_FAILURE);
    }

    if ((src_ip = niLookup(AF_INET, &srcaddr)) == NULL)
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

    int ttl = 0;
    while (++ttl < 64)
    {   ttl = 64;
        int rtt_min = -1, rtt_max = -1;
        float rtt_avg = -1;
        for (int i = 0; i < n; i++)
        {
            char msg[] = "hello server!!!";
            char *buffer = (char *)malloc(MAX_SIZE * sizeof(char));
            struct ip *ip = (struct ip *)buffer;
            struct icmp *icmp = (struct icmp *)(buffer + sizeof(struct ip));
            char *data = (char *)(buffer + sizeof(struct ip) + sizeof(struct icmp));
            ip->ip_v = IPv4;
            ip->ip_hl = sizeof(struct ip) / DWORD;
            ip->ip_tos = 0;
            ip->ip_len = htons(sizeof(struct ip) + sizeof(struct icmp) + sizeof(msg));
            ip->ip_id = getpid();
            ip->ip_off = 0;
            ip->ip_ttl = ttl;
            ip->ip_p = IPPROTO_ICMP;
            ip->ip_sum = 0;
            ip->ip_src = srcaddr.sin_addr;
            ip->ip_dst = destaddr.sin_addr;
            ip->ip_sum = checksum(ip, sizeof(struct ip));

            icmp->type = ICMP_ECHO;
            icmp->code = 0;
            icmp->checksum = 0;
            icmp->un.echo.id = rand();
            icmp->un.echo.sequence = 0;

            for (int i = 0; i < sizeof(msg); i++)
                data[i] = msg[i];

            icmp->checksum = checksum(icmp, sizeof(struct icmp) + sizeof(msg));

            assert(checksum(ip, sizeof(struct ip)) == 0);
            assert(checksum(icmp, sizeof(struct icmp) + sizeof(msg)) == 0);
            printIP(ip);
            printICMP(icmp);
            printf("%s\n", data);

            if (sendto(sockfd, buffer, ntohs(ip->tot_len), 0, (const struct sockaddr *)&destaddr, sizeof(destaddr)) < 0)
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
            if ((recvlen = recvfrom(sockfd, recv_buffer, MAX_SIZE, 0, &addr, &addrlen)) < 0)
            {
                perror("recvfrom");
                exit(EXIT_FAILURE);
            }
            if (gettimeofday(&recv_tv, NULL) < 0)
            {
                perror("gettimeofday");
                exit(EXIT_FAILURE);
            }
            int rtt_m = 1000000 * (recv_tv.tv_sec - send_tv.tv_sec) + (recv_tv.tv_usec - send_tv.tv_usec);
            rtt_min = (rtt_min < 0 || rtt_m < rtt_min) ? rtt_m : rtt_min;
            rtt_max = (rtt_max < 0 || rtt_m > rtt_max) ? rtt_m : rtt_max;
            rtt_avg = (rtt_avg < 0) ? rtt_m : (1 - ALPHA) * rtt_avg + ALPHA * rtt_m;
            // printf("recieved %d bytes from %s: time = %d us\n", recvlen, inet_ntoa(addr.sin_addr), rtt_m);
            struct ip *ip_reply = (struct ip *)recv_buffer;            if ((recvlen = recvfrom(sockfd, recv_buffer, MAX_SIZE, 0, &addr, &addrlen)) < 0)

            struct icmp *icmp_reply = (struct icmp *)(recv_buffer + sizeof(struct ip));
            char *data_reply = recv_buffer + sizeof(struct ip) + sizeof(struct icmp);
            printIP(ip_reply);
            printICMP(icmp_reply);
            printf("%s\n", data_reply);
        }
        printf("HOP = %d: rtt_min = %d us, rtt_max = %d us, rtt_avg = %f us\n", ttl, rtt_min, rtt_max, rtt_avg);
    }
    return 0;
}

/**
 * @brief returns 16-bit checksum
 *
 * @param buff bitstream whose checksum is to be computed
 * @param nbytes number of bytes in buff
 */
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

/**
 * @brief resolves a hostname into an IP address in numbers-and-dots notation
 *
 * @param h_name hostname
 * @param addr sockaddr of hostname filled up, if NOT null
 */
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

/**
 * @brief returns an IP address in numbers-and-dots notation to the network interface of specified family in the local system
 *
 * @param ni_family family of network interface
 * @param addr sockaddr of network interface filled up, if NOT null
 */
char *niLookup(int ni_family, struct sockaddr_in *addr)
{
    struct ifaddrs *ifaddr, *it;
    if (getifaddrs(&ifaddr) < 0)
    {
        perror("getifaddrs");
        return NULL;
    }
    it = ifaddr;
    while (it != NULL)
    {
        if (it->ifa_addr != NULL && it->ifa_addr->sa_family == ni_family && !(it->ifa_flags & IFF_LOOPBACK) && (it->ifa_flags & IFF_RUNNING))
        {
            if (addr != NULL)
            {
                addr->sin_family = ((struct sockaddr_in *)it->ifa_addr)->sin_family;
                addr->sin_port = ((struct sockaddr_in *)it->ifa_addr)->sin_port;
                addr->sin_addr = ((struct sockaddr_in *)it->ifa_addr)->sin_addr;
            }
            break;
        }
        it = it->ifa_next;
    }
    freeifaddrs(ifaddr);

    if (it == NULL)
    {
        printf("No active network interface of family: %d found\n", ni_family);
        return NULL;
    }
    return strdup(inet_ntoa(((struct sockaddr_in *)it->ifa_addr)->sin_addr));
}

void printIP(struct ip *ip)
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

void printICMP(struct icmp *icmp)
{
    printf("-----------------------------------------------------------------\n");
    printf("|    type:%-2d    |    code:%-2d    |        checksum:%-6d        |\n", icmp->type, icmp->code, icmp->checksum);
    printf("-----------------------------------------------------------------\n");
    if (icmp->type == ICMP_ECHO || icmp->type == ICMP_ECHOREPLY)
        printf("|           id:%-6d           |        sequence:%-6d        |\n", icmp->un.echo.id, icmp->un.echo.sequence);
    printf("-----------------------------------------------------------------\n");
}