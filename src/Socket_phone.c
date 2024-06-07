#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define BUFFER_SIZE 256

void *send_data_serv(void *arg) {
    // 通信ができてから音声を送信
    FILE *fp;
    char *cmdline = "rec -t raw -b 16 -c 1 -e s -r 44100 -";
    if((fp = popen(cmdline, "r")) == NULL){
        perror("popen");
        exit(1);
    }

    int s = *(int *)arg;
    // 出力データのバッファ
    // 標準入力から読み込み，データを送信
    char buf[1];
    while(1){
        int n = fread(buf, sizeof(char), sizeof(char), fp);
        if(n == -1){
            perror("read");
            exit(1);
        }else if(n == 0){
            break;
        }

        int m = send(s, buf, n, 0);
        if(m == -1){
            perror("send");
            exit(1);
        }
    }
    pclose(fp);
    return NULL;
}

void *send_data_client(void *arg) {
    int s = *(int *)arg;
    char data[BUFFER_SIZE];
    while (1) {
        int n = read(0, data, BUFFER_SIZE);
        if (n == -1) {
            perror("read");
            exit(1);
        } else if (n == 0) {
            break;
        }
        int m = send(s, data, n, 0);
        if (m == -1) {
            perror("send");
            exit(1);
        }
    }
    shutdown(s, SHUT_WR);
    return NULL;
}

void *recv_data(void *arg) {
    int s = *(int *)arg;
    char data[BUFFER_SIZE];
    while (1) {
        int n = recv(s, data, BUFFER_SIZE, 0);
        if (n == -1) {
            perror("recv");
            exit(1);
        } else if (n == 0) {
            break;
        }
        int m = write(1, data, n);
        if (m == -1) {
            perror("write");
            exit(1);
        }
    }
    return NULL;
}

int main(int argc, char *argv[]){
    // サーバー側
    if (argc == 2) {
        // ソケットの作成．返り値はファイルディスクリプタ
        int ss = socket(PF_INET, SOCK_STREAM, 0);
        if(ss == -1){
            perror("socket");
            exit(1);
        }
        struct sockaddr_in addr; //最終的にbindに渡すアドレス
        addr.sin_family = AF_INET; /* これはIPv4のアドレス*/
        addr.sin_port = htons(atoi(argv[1]));  //ポート番号
        addr.sin_addr.s_addr = INADDR_ANY; //どのIPアドレスも受付

        int ret = bind(ss, (struct sockaddr *)&addr, sizeof(addr));
        if(ret == -1){
            perror("bind");
            exit(1);
        }

        listen(ss, 10);

        struct sockaddr_in client_addr;
        socklen_t len = sizeof(struct sockaddr_in);
        int s = accept(ss, (struct sockaddr *)&client_addr, &len);
        if(s == -1){
            perror("accept");
            exit(1);
        }
        close(ss);

        // 並列処理
        pthread_t send_thread, recv_thread;

        if (pthread_create(&send_thread, NULL, send_data_serv, &s) != 0) {
            perror("pthread_create");
            exit(1);
        }

        if (pthread_create(&recv_thread, NULL, recv_data, &s) != 0) {
            perror("pthread_create");
            exit(1);
        }

        pthread_join(send_thread, NULL);
        pthread_join(recv_thread, NULL);

        close(s);
    }

    else if(argc == 3){
        // クライアント側
        int s = socket(PF_INET, SOCK_STREAM, 0);
        if (s == -1) {
            perror("socket");
            exit(1);
        }

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(argv[1]);
        addr.sin_port = htons(atoi(argv[2]));

        int ret = connect(s, (struct sockaddr *)&addr, sizeof(addr));
        if (ret == -1) {
            perror("connect");
            exit(1);
        }

        //並列処理
        pthread_t send_thread, recv_thread;

        if (pthread_create(&send_thread, NULL, send_data_client, &s) != 0) {
            perror("pthread_create");
            exit(1);
        }

        if (pthread_create(&recv_thread, NULL, recv_data, &s) != 0) {
            perror("pthread_create");
            exit(1);
        }

        pthread_join(send_thread, NULL);
        pthread_join(recv_thread, NULL);

        close(s);
    }

    else{
        fprintf(stderr, "Usage: %s <Port>\n", argv[0]);
        exit(1);
    }

    return 0;
}