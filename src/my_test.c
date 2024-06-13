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

int main(){
    for (int i = 0; i < 3; ++i){
        FILE *fp;
            char *cmdline = "play ../data/Ringtone/call.mp3 ";
            if((fp = popen(cmdline, "w")) == NULL){
                perror("popen");
                exit(1);
            }
        pclose(fp);
    }
    return 0;
}