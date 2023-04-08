#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/in.h>
#include <sys/time.h>

#define PORT 8080
int min(int a, int b)
{
    return a < b ? a : b;
}
int max(int a, int b)
{
    return a > b ? a : b;
}
uint16_t checksum(const void *buff, size_t nbytes)
{
    uint64_t sum = 0;
    uint16_t *words = (uint16_t *)buff;
    size_t _16bitword = nbytes / 2;
    // Sum all the 16-bit words
    while (_16bitword--)
    {
        sum += *(words++);
    }
    // Add left-over byte, if any
    if (nbytes & 1)
    {
        sum += (uint16_t)(*(uint8_t *)words);
    }
    sum = ((sum >> 16) + (sum & 0xFFFF)); // Fold to 16 bits
    sum += (sum >> 16);                   // Add carry
    return (uint16_t)(~sum);              // Return one's complement
}
void printIP(struct iphdr *ip)
{
    printf("-----------------------------------------------------------------\n");
    printf("|   version:%-2d  |   hlen:%-4d   |     tos:%-2d    |  totlen:%-4d  |\n", ip->version, ip->ihl, ip->tos, ip->tot_len);
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
int create_ip_packet(char *ip_packet, int bytes_to_send, int ttl, int id, int seq, struct sockaddr_in dest_addr)
{
    struct iphdr *ip = (struct iphdr *)ip_packet;
    ip->version = 4;
    ip->ihl = 5;
    ip->tos = 0;
    ip->tot_len = sizeof(struct iphdr) + sizeof(struct icmphdr) + bytes_to_send;
    ip->id = htons(0);
    ip->frag_off = 0;
    ip->ttl = ttl;
    ip->protocol = IPPROTO_ICMP;
    ip->check = 0;
    ip->saddr = INADDR_ANY;
    // ip->daddr = addr.sin_addr.s_addr;
    ip->daddr = dest_addr.sin_addr.s_addr;
    ip->check = checksum((uint16_t *)ip, sizeof(struct iphdr));
    struct icmphdr *icmp = (struct icmphdr *)(ip_packet + sizeof(struct iphdr));
    icmp->type = ICMP_ECHO;
    icmp->code = 0;
    icmp->checksum = 0;
    icmp->un.echo.id = 0;
    icmp->un.echo.sequence = 0;
    icmp->checksum = 0;
    char *data = (char *)malloc(bytes_to_send);
    for (int i = 0; i < bytes_to_send; i++)
        data[i] = 'a';
    memcpy(ip_packet + sizeof(struct iphdr) + sizeof(struct icmphdr), data, bytes_to_send);
    icmp->checksum = checksum((uint16_t *)icmp, sizeof(struct icmphdr) + bytes_to_send);
    return ip->tot_len;
}
void linear_regression(double x[], double y[], int n, double* slope, double* intercept) {
    double sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_xx = 0.0;
    for (int i = 0; i < n; i++) {
        sum_x += x[i];
        sum_y += y[i];
        sum_xy += x[i] * y[i];
        sum_xx += x[i] * x[i];
    }
    double x_mean = sum_x / n;
    double y_mean = sum_y / n;
    double denominator = sum_xx - sum_x * x_mean;
    if (denominator == 0) {
        *slope = 0;
        *intercept = y_mean;
    } else {
        *slope = (sum_xy - sum_x * y_mean) / denominator;
        *intercept = y_mean - *slope * x_mean;
    }
}
int main()
{
    printf("Enter hostname/ip address: ");
    char host[1000];
    scanf("%s", host);
    printf("Enter number of times to ping(per data size): ");
    int n;
    scanf("%d", &n);
    printf("Enter time diff b/w pings: ");
    int time_diff;
    scanf("%d", &time_diff);
    struct hostent *h = gethostbyname(host);
    if (h == NULL)
    {
        printf("gethostbyname failed\n");
        exit(1);
    }
    char *ip_address = malloc(1000);
    ip_address = inet_ntoa(*((struct in_addr *)h->h_addr));
    printf("IP address: %s\n", ip_address);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(ip_address);

    // create raw socket
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock < 0)
    {
        printf("socket failed\n");
        exit(1);
    }

    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &(int){1}, sizeof(int)) < 0)
    {
        printf("setsockopt failed\n");
        exit(1);
    }
    int max_ttl = 32;
    int bytes_to_send = 60;

    printf("traceroute to iitkgp.ac.in (%s), %d hops max, %d byte packets\n", ip_address, max_ttl, bytes_to_send);
    printf("%-3s %-15s %-6s\n", "HOP", "IP", "RTT");
    int num_hops = 0;
    struct sockaddr_in nodes[max_ttl];
    for (int ttl = 1; ttl <= max_ttl; ttl++)
    {
        char *ip_packet = (char *)malloc(sizeof(struct iphdr) + sizeof(struct icmphdr) + bytes_to_send);
        int ip_packet_len = create_ip_packet(ip_packet, bytes_to_send, ttl, 0, 0, addr);
        struct timeval start, end;
        gettimeofday(&start, NULL);
        int sent = sendto(sock, ip_packet, ip_packet_len, 0, (struct sockaddr *)&addr, sizeof(addr));
        if (sent < 0)
        {
            printf("sendto failed\n");
            exit(1);
        }
        char *buffer = (char *)malloc(65536);
        struct sockaddr_in intermediate_node;
        int len = sizeof(intermediate_node);
        int recvd = recvfrom(sock, buffer, 65536, 0, (struct sockaddr *)&intermediate_node, &len);
        gettimeofday(&end, NULL);
        double rtt = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
        rtt /= 1000;
        struct iphdr *ip_reply = (struct iphdr *)buffer;
        printf("protocol: %d\n", ip_reply->protocol);
        struct icmphdr *icmp_reply = (struct icmphdr *)(buffer + sizeof(struct iphdr));
        printf("%-3d %-15s %-.4fms\n", ttl, inet_ntoa(intermediate_node.sin_addr), rtt);
        nodes[ttl - 1] = intermediate_node;
        if (icmp_reply->type == ICMP_ECHOREPLY)
        {
            num_hops = ttl;
            break;  
        }
    }

    // Get latency and bandwidth
    int data_sizes[] = {1, 20, 80, 200};
    int num_data_sizes = 4;
    // Hop latency bandwidth
    printf("%-3s %-10s %-10s\n", "HOP", "LATENCY", "BANDWIDTH");
    // for(int hop=1; hop<=num_hops; hop++){
        double rtts[num_data_sizes];
        for(int i=0; i<num_data_sizes; i++){
            printf("i = %d\n", i);
            double rtt_avg = 0.0;
            int bytes_to_send = data_sizes[i];
            for(int num_packet=1; num_packet<=n; num_packet++){
                printf("num_packet: %d\n", num_packet);
                printf("data sizes[i] = %d\n", bytes_to_send);
                char* ip_packet = (char*)malloc(sizeof(struct iphdr) + sizeof(struct icmphdr) + bytes_to_send);
                int ip_packet_len = create_ip_packet(ip_packet, bytes_to_send, num_hops, num_packet, num_packet, nodes[0]);
                // int ip_packet_len = create_ip_packet(ip_packet, bytes_to_send, num_hops, 0, num_packet, addr);
                struct timeval start, end;
                gettimeofday(&start, NULL);
                int sent = sendto(sock, ip_packet, ip_packet_len, 0, (struct sockaddr*)&nodes[0], sizeof(nodes[0]));
                // int sent = sendto(sock, ip_packet, ip_packet_len, 0, (struct sockaddr*)&addr, sizeof(addr));
                // printf("Sent %d bytes\n", bytes_to_send);
                // if(sent < 0){
                //     printf("sendto failed\n");
                //     exit(1);
                // }
                char* buffer = (char*)malloc(65536);
                struct sockaddr_in intermediate_node;
                int len = sizeof(intermediate_node);
                int recvd = recvfrom(sock, buffer, 65536, 0, (struct sockaddr*)&intermediate_node, &len);
                // printf("recvd\n");
                gettimeofday(&end, NULL);
                struct iphdr* ip_reply = (struct iphdr*)buffer;
                struct icmphdr* icmp_reply = (struct icmphdr*)(buffer + sizeof(struct iphdr));
                // printf("protocol: %d\n", ip_reply->protocol);
                // printf("icmp_reply->type: %d\n", icmp_reply->type);
                char* data = (char*)(buffer + sizeof(struct iphdr) + sizeof(struct icmphdr));
                printf("data: %s\n", data);
                if(strcmp(ip_packet+sizeof(struct iphdr)+sizeof(struct icmphdr), data) != 0){
                    printf("Data mismatch\n");
                    exit(1);
                }
                double rtt = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
                rtt /= 1000;
                rtt_avg += rtt;
                sleep(time_diff);
            }
            rtt_avg /= n;
            rtts[i] = rtt_avg;
            // sleep(time_diff);
        }
        double latency=0.0, bandwidth=0.0;
        for(int i=0; i<num_data_sizes; i++){
            printf("%.5f ", rtts[i]);
        }
        printf("\n");
        // double data_sizes_mbs[num_data_sizes];
        // for(int i=0; i<num_data_sizes; i++){
        //     data_sizes_mbs[i] = data_sizes[i]/1024.0;
        // }
        // linear_regression(rtts, data_sizes_mbs, num_data_sizes, &bandwidth, &latency);
        // printf("%-3d %-10.4fms %-10.4fMbps\n", hop, latency, bandwidth);
    // }
    return 0;

}