#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>

#define BLOCK_SIZE 512

long long timespec_diff_ns(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);
}

int request(uint8_t* buf, size_t buf_size, uint16_t opcode, const char* filename, const char* mode) {
    if(opcode != 1 && opcode != 2) {
        fprintf(stderr, "请求报文错误: 操作码必需是 1 或 2\n");
        exit(EXIT_FAILURE);
    }
    if(strlen(filename) == 0 || strlen(mode) == 0) {
        fprintf(stderr, "请求报文错误: 文件名或模式为空\n");
        exit(EXIT_FAILURE);
    }
    if(strlen(filename) + strlen(mode) + 4 > buf_size) {
        fprintf(stderr, "请求报文错误: 数据溢出\n");
        exit(EXIT_FAILURE);
    }

    *(uint16_t*)buf = htons(opcode);    // 把操作码写入数据包
    char *p = (char*)buf + 2;                      // 指向写入 filename 的起始位置
    strcpy(p, filename);                // 写入文件名
    p = p + strlen(filename) + 1;               // 指向写入 mode 的起始位置
    strcpy(p, mode);                    // 写入传输模式
    p = p + strlen(mode) + 1;                   // 指向数据最后一个 \0 位置
    
    return p - (char*)buf;                         // 返回数据大小
}

int main(int argc, char* argv[]) {
    if(argc < 4 || argc > 5) {
        fprintf(stderr, "参数错误s\n");
        exit(EXIT_FAILURE);
    }

    const char* method = argv[1];
    const char* server_ip = argv[2];
    const char* remote_file;
    const char* local_file;
    
    uint16_t opcode;
    uint8_t buffer[516];
    
    if (strcmp(method, "get") == 0) {
        if (argc == 4) {
            remote_file = argv[3];
            local_file = argv[3];
        } else {
            remote_file = argv[3];
            local_file = argv[4];
        }
        opcode = 1;
    }
    else if (strcmp(method, "put") == 0) {
        if (argc == 4) {
            local_file = argv[3];
            remote_file = argv[3];
        } else {
            local_file = argv[3];
            remote_file = argv[4];
        }
        opcode = 2;
    }
    else {
        fprintf(stderr, "错误参数: 必需输入 get 或 put\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr = {0};                  // 存储服务器的网络地址信息
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(69);                      // 初始端口为 69
    server_addr.sin_addr.s_addr = inet_addr(server_ip);    // 服务器 ip 地址
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);           // 创建UDP socket
    if (sockfd < 0) {
        fprintf(stderr, "创建 socket 错误\n");
        exit(EXIT_FAILURE);
    }

    int request_len = request(buffer, sizeof(buffer), opcode, remote_file, "octet");
    if (request < 0) {
        fprintf(stderr, "请求报文错误：返回长度不对\n");
        exit(EXIT_FAILURE);
    }
    int send = sendto(sockfd, buffer, request_len, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (send < 0) {
        fprintf(stderr, "发送请求报文失败\n");
        exit(EXIT_FAILURE);
    }
    
    return 0;
}
