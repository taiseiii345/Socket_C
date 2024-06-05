#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]){
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

    pid_t pid;
    pid = fork();
    if(pid == -1){
        perror("fork");
        exit(1);
    } else if(pid == 0){
        // 子プロセスの処理
        // データを受け取り
        int N = 256;
        char data[N];
        while(1){
            int n = recv(s, data, N, 0);
            if(n == -1){
                perror("recv");
                exit(1);
            }else if(n == 0){
                break;
            }
            write(1, data, n);
        }

    } else if (pid > 0){
        // 親プロセスの処理
        // データ送信
        FILE *fp;
        char *cmdline = "rec -t raw -b 16 -c 1 -e s -r 44100 -";
        if((fp = popen(cmdline, "r")) == NULL){
            perror("popen");
            exit(1);
        }

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
    }
}