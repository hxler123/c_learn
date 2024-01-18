#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include <sys/param.h>

#define MAX_REQUEST_LENGTH 1024

char host[MAXHOSTNAMELEN] = "http://learn.microsoft.com：9090/zh-cn/";
char request[MAX_REQUEST_LENGTH];
int port = 80;
int client = 0;

static int Socket(char *host, int port);
static void generate_request();
static int bench();
static void bench_core();

int main(int argc, char *argv[]) {


}


/* generate request header */
static void generate_request() {
    strcat(request, "GET ")

}

/* Create socket to connect remote socket， return socket file descriptor */
static int Socket(char *host, int port) {
    int sock;
    unsigned long addr;
    struct sockaddr_in ad;
    struct hostent *hp;

    memset(&ad, 0, sizeof(ad));
    ad.sin_family = AF_INET;
    ad.sin_port = htons(port);
    addr = inet_addr(host);
    if (addr != INADDR_NONE) {
        memcpy(&ad.sin_addr, &addr, sizeof(addr));
    } else {
        hp = gethostbyname(host);
        memcpy(&ad.sin_addr, hp->h_addr, sizeof(hp->h_length));
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Create socket failed!");
        return sock;
    }

    if (connect(sock, (struct sockaddr *)&ad, sizeof(ad)) < 0)
        return -1;
    return sock;
}

