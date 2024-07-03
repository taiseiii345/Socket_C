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
#include <termios.h>
#include <fcntl.h>

#define BUFFER_SIZE 256
#define max_call 3

// 通話の開始
int connected = 0;
// ミュートの開始
int mute = 0;
// スピーカーの開始
int speaker = 0;

void *send_data(void *arg) {
    char *cmdline = "rec -t raw -b 16 -c 1 -e s -r 44100 -";
    FILE *fp = popen(cmdline, "r");
    if (fp == NULL) {
        perror ("popen send failed");
        exit(EXIT_FAILURE);
    }

    int s = *(int *)arg;

    short data[1];
    while(1){
        int n = fread(data, sizeof(short), 1, fp);
        if(n == -1){
            perror("read");
            exit(1);
        }
        if(n == 0){
            break;
        }
        if(speaker == 1){
            data[0] *= 2;
        }
        if(mute == 1){
            short mute_sig[1];
            mute_sig[0] = (short)0;
            int m = send(s, mute_sig, sizeof(short), 0);
            if (m == -1) {
                perror("write");
                exit(1);
            }
        }
        else{
            int nn = send(s, data, sizeof(data), 0);
            if(nn < 0){
                perror("send");
                exit(1);
            }
        }
    }
    pclose(fp);
    return NULL;
}

void *recv_data(void *arg) {
    char *cmdline = "play -t raw -b 16 -c 1 -e s -r 44100 -";
    FILE *fp = popen(cmdline, "w");
    if (fp == NULL) {
        perror ("popen recv failed");
        exit(EXIT_FAILURE);
    }

    int s = *(int *)arg;
    short data[1];
    while (1) {
        int n = recv(s, data, sizeof(data), 0);
        if (n == -1) {
            perror("recv");
            exit(1);
        } else if (n == 0) {
            break;
        }
        
            int m = fwrite(data, sizeof(short), 1, fp);
            if (m == -1) {
                perror("write");
                exit(1);
            }

    }
    pclose(fp);
    return NULL;
}

void *ring(){
    int counter = 0;
    while(counter < max_call){
        if(connected == 1){
            break;
        }
        FILE *fp;
        char *cmdline = "play ../data/Ringtone/call.mp3";
        if((fp = popen(cmdline, "w")) == NULL){
            perror("popen");
            exit(1);
        }
        ++counter;
        pclose(fp);
    }

    pthread_exit(NULL);

}

// getcharはEOF(Enter)押さないと発動しない
// cでserver側が通話開始、mでお互いに相手をミュート
void *getchar_self(void *arg){
    int s = *(int *)arg;
    char data[1];
    while(1){
        data[0] = getchar();
        // printf("%c\n",data[0]);
        switch(data[0]){
            case 'c':
                connected = 1;
                // 送る処理
                int send_char_num = send(s, data, sizeof(char), 0);
                if(send_char_num < 0){
                    perror("send");
                    exit(1);
                }
                // break;
                pthread_exit(NULL);
            case 'm':
                mute = (mute + 1) % 2;
            case 's':
                speaker = (speaker + 1) % 2;
        }
    }
}

// cでclient側が通話開始
void *getchar_opponent(void *arg){
    int s = *(int *)arg;
    char data[1];
    while (1) {
        int n = recv(s, data, sizeof(data), 0);
        if (n == -1) {
            perror("recv");
            exit(1);
        }
        switch(data[0]){
            case 'c':
                connected = 1;
                // break;
                pthread_exit(NULL);
        }
    }
}

int main(int argc, char *argv[]){
    // サーバー側
    if (argc == 2) {
        // ソケットの作成
        int ss = socket(PF_INET, SOCK_STREAM, 0);
        if(ss == -1){
            perror("socket");
            exit(1);
        }
        struct sockaddr_in addr; //最終的にbindに渡すアドレス
        addr.sin_family = AF_INET; /* これはIPv4のアドレス*/
        addr.sin_port = htons(atoi(argv[1]));  //ポート番号
        addr.sin_addr.s_addr = INADDR_ANY; //どのIPアドレスも受付

        // どのポートで待ち受けるか
        int ret = bind(ss, (struct sockaddr *)&addr, sizeof(addr));
        if(ret == -1){
            perror("bind");
            exit(1);
        }
        // 待ち受け可能宣言
        listen(ss, 10);

        // クライアントがconnectするまで待つ
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(struct sockaddr_in);
        int s = accept(ss, (struct sockaddr *)&client_addr, &len);
        if(s == -1){
            perror("accept");
            exit(1);
        }
        close(ss);

        // 着信音

        pthread_t getchar_self_thread, ring_thread;

        if (pthread_create(&getchar_self_thread, NULL, getchar_self, &s) != 0){
            perror("pthread_create");
            exit(1);
        }

        if (pthread_create(&ring_thread, NULL, ring, &s) != 0){
            perror("pthread_create");
            exit(1);
        }

        pthread_join(getchar_self_thread, NULL);
        pthread_join(ring_thread, NULL);

        if (connected == 0){
            return 0;
        }

        // 並列処理
        pthread_t send_thread, recv_thread;

        if (pthread_create(&send_thread, NULL, send_data, &s) != 0) {
            perror("pthread_create");
            exit(1);
        }

        if (pthread_create(&recv_thread, NULL, recv_data, &s) != 0) {
            perror("pthread_create");
            exit(1);
        }

        if (pthread_create(&getchar_self_thread, NULL, getchar_self, &s) != 0){
            perror("pthread_create");
            exit(1);
        }

        pthread_join(send_thread, NULL);
        pthread_join(recv_thread, NULL);
        pthread_join(getchar_self_thread, NULL);

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

        // 並列処理
        pthread_t ring_thread, getchar_opponent_thread;

        // タイミング調整
        sleep(1);

        if (pthread_create(&ring_thread, NULL, ring, &s) != 0) {
            perror("pthread_create");
            exit(1);
        }

        if (pthread_create(&getchar_opponent_thread, NULL, getchar_opponent, &s) != 0) {
            perror("pthread_create");
            exit(1);
        }

        pthread_join(ring_thread, NULL);
        pthread_join(getchar_opponent_thread, NULL);

        if(connected == 0){
            return 0;
        } 

        // 並列処理
         pthread_t send_thread, recv_thread;

        if (pthread_create(&send_thread, NULL, send_data, &s) != 0) {
            perror("pthread_create");
            exit(1);
        }

        if (pthread_create(&recv_thread, NULL, recv_data, &s) != 0) {
            perror("pthread_create");
            exit(1);
        }

        if (pthread_create(&getchar_opponent_thread, NULL, getchar_opponent, &s) != 0){
            perror("pthread_create");
            exit(1);
        }

        pthread_join(send_thread, NULL);
        pthread_join(recv_thread, NULL);
        pthread_join(getchar_opponent_thread, NULL);

        close(s);
    }

    else{
        fprintf(stderr, "Usage (server): %s <Port>\n", argv[0]);
        fprintf(stderr, "Usage (client): %s <IP> <Port>\n", argv[0]);
        exit(1);
    }

    return 0;
}