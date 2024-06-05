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
    int s = socket(PF_INET, SOCK_STREAM, 0);
    if(s == -1){
        perror("socket");
        exit(1);
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET; /* これはIPv4のアドレス*/
    addr.sin_addr.s_addr = inet_addr(argv[1]); 
    addr.sin_port = htons(atoi(argv[2]));
     /* ポート番号は12345 */
    int ret = connect(s, (struct sockaddr *)&addr, sizeof(addr));
    if(ret == -1){
        perror("connect");
        exit(1);
    }

    pid_t pid;
    pid = fork();
    if(pid == -1){
        perror("fork");
        exit(1);
    } else if(pid == 0){
        // 子プロセスの処理
        int N = 256;
        char data[N];
        while(1){
            int n = read(0, data, N);
            if(n == -1){
                perror("read");
                exit(1);
            }else if(n == 0){
                break;
            }
            int m = send(s, data, n, 0);
            if(m == -1){
                perror("send");
                exit(1);
            }
        }
    } else if (pid > 0){
        // 親プロセスの処理
        // その後，サーバからのデータを読み込んで，標準出力に出力
        int N2 = 256;
        char data2[N2];
        while(1){
            int n2 = recv(s, data2, N2, 0);
            if(n2 == -1){
                perror("recv");
                exit(1);
            }else if(n2 == 0){
                break;
            }
            int m2 = write(1, data2, n2);
            if(m2 == -1){
                perror("write");
                exit(1);
            }
        }

    }
}